from libs.PipeLine import PipeLine, ScopedTiming
from media.sensor import *
from media.sensor import Sensor
from libs.AIBase import AIBase
from libs.AI2D import Ai2d
import os
import ujson
from media.media import *
from time import *
import nncase_runtime as nn
import ulab.numpy as np
import time
import image
import aidemo
import random
import gc
import sys
from machine import UART
from machine import FPIOA
import math

# 配置引脚
fpioa = FPIOA()
fpioa.set_function(11, FPIOA.UART2_TXD)
fpioa.set_function(12, FPIOA.UART2_RXD)
uart = UART(UART.UART2, baudrate=115200, bits=UART.EIGHTBITS, parity=UART.PARITY_NONE, stop=UART.STOPBITS_ONE)

# 自定义人脸检测任务类
class FaceDetApp(AIBase):
    def __init__(self,kmodel_path,model_input_size,anchors,confidence_threshold=0.25,nms_threshold=0.3,rgb888p_size=[1280,720],display_size=[1920,1080],debug_mode=0):
        super().__init__(kmodel_path,model_input_size,rgb888p_size,debug_mode)
        # kmodel路径
        self.kmodel_path=kmodel_path
        # 检测模型输入分辨率
        self.model_input_size=model_input_size
        # 置信度阈值
        self.confidence_threshold=confidence_threshold
        # nms阈值
        self.nms_threshold=nms_threshold
        self.anchors=anchors
        # sensor给到AI的图像分辨率，宽16字节对齐
        self.rgb888p_size=[ALIGN_UP(rgb888p_size[0],16),rgb888p_size[1]]
        # 视频输出VO分辨率，宽16字节对齐
        self.display_size=[ALIGN_UP(display_size[0],16),display_size[1]]
        # debug模式
        self.debug_mode=debug_mode
        # 长度阈值配置
        self.length_threshold = 100  # 默认50像素长度阈值
        # Ai2d实例，用于实现模型预处理
        self.ai2d=Ai2d(debug_mode)
        # 设置Ai2d的输入输出格式和类型
        self.ai2d.set_ai2d_dtype(nn.ai2d_format.NCHW_FMT,nn.ai2d_format.NCHW_FMT,np.uint8, np.uint8)

    # 配置预处理操作，这里使用了padding和resize，Ai2d支持crop/shift/pad/resize/affine
    def config_preprocess(self,input_image_size=None):
        with ScopedTiming("set preprocess config",self.debug_mode > 0):
            # 初始化ai2d预处理配置，默认为sensor给到AI的尺寸，可以通过设置input_image_size自行修改输入尺寸
            ai2d_input_size=input_image_size if input_image_size else self.rgb888p_size
            # 设置padding预处理
            self.ai2d.pad(self.get_pad_param(), 0, [104,117,123])
            # 设置resize预处理
            self.ai2d.resize(nn.interp_method.tf_bilinear, nn.interp_mode.half_pixel)
            # 构建预处理流程,参数为预处理输入tensor的shape和预处理输出的tensor的shape
            self.ai2d.build([1,3,ai2d_input_size[1],ai2d_input_size[0]],[1,3,self.model_input_size[1],self.model_input_size[0]])

    # 自定义任务后处理,这里使用了aidemo库的face_det_post_process接口
    def postprocess(self,results):
        with ScopedTiming("postprocess",self.debug_mode > 0):
            res = aidemo.face_det_post_process(self.confidence_threshold,self.nms_threshold,self.model_input_size[0],self.anchors,self.rgb888p_size,results)
            if len(res)==0:
                return res
            else:
                return res[0]

    # 计算padding参数
    def get_pad_param(self):
        dst_w = self.model_input_size[0]
        dst_h = self.model_input_size[1]
        # 计算最小的缩放比例，等比例缩放
        ratio_w = dst_w / self.rgb888p_size[0]
        ratio_h = dst_h / self.rgb888p_size[1]
        if ratio_w < ratio_h:
            ratio = ratio_w
        else:
            ratio = ratio_h
        new_w = (int)(ratio * self.rgb888p_size[0])
        new_h = (int)(ratio * self.rgb888p_size[1])
        dw = (dst_w - new_w) / 2
        dh = (dst_h - new_h) / 2
        top = (int)(round(0))
        bottom = (int)(round(dh * 2 + 0.1))
        left = (int)(round(0))
        right = (int)(round(dw * 2 - 0.1))
        return [0,0,0,0,top, bottom, left, right]

# 自定义人脸关键点任务类
class FaceLandMarkApp(AIBase):
    def __init__(self,kmodel_path,model_input_size,rgb888p_size=[1920,1080],display_size=[1920,1080],debug_mode=0):
        super().__init__(kmodel_path,model_input_size,rgb888p_size,debug_mode)
        # kmodel路径
        self.kmodel_path=kmodel_path
        # 关键点模型输入分辨率
        self.model_input_size=model_input_size
        # sensor给到AI的图像分辨率，宽16字节对齐
        self.rgb888p_size=[ALIGN_UP(rgb888p_size[0],16),rgb888p_size[1]]
        # 视频输出VO分辨率，宽16字节对齐
        self.display_size=[ALIGN_UP(display_size[0],16),display_size[1]]
        # debug模式
        self.debug_mode=debug_mode
        # 目标矩阵
        self.matrix_dst=None
        self.ai2d=Ai2d(debug_mode)
        self.ai2d.set_ai2d_dtype(nn.ai2d_format.NCHW_FMT,nn.ai2d_format.NCHW_FMT,np.uint8, np.uint8)

    # 配置预处理操作，这里使用了affine，Ai2d支持crop/shift/pad/resize/affine
    def config_preprocess(self,det,input_image_size=None):
        with ScopedTiming("set preprocess config",self.debug_mode > 0):
            # 初始化ai2d预处理配置，默认为sensor给到AI的尺寸，可以通过设置input_image_size自行修改输入尺寸
            ai2d_input_size=input_image_size if input_image_size else self.rgb888p_size
            # 计算目标矩阵，并获取仿射变换矩阵
            self.matrix_dst = self.get_affine_matrix(det)
            affine_matrix = [self.matrix_dst[0][0],self.matrix_dst[0][1],self.matrix_dst[0][2],
                             self.matrix_dst[1][0],self.matrix_dst[1][1],self.matrix_dst[1][2]]
            # 设置仿射变换预处理
            self.ai2d.affine(nn.interp_method.cv2_bilinear,0, 0, 127, 1,affine_matrix)
            # 构建预处理流程,参数为预处理输入tensor的shape和预处理输出的tensor的shape
            self.ai2d.build([1,3,ai2d_input_size[1],ai2d_input_size[0]],[1,3,self.model_input_size[1],self.model_input_size[0]])

    # 自定义后处理，results是模型输出的array列表，这里使用了aidemo库的invert_affine_transform接口
    def postprocess(self,results):
        with ScopedTiming("postprocess",self.debug_mode > 0):
            pred=results[0]
            # （1）将人脸关键点输出变换模型输入
            half_input_len = self.model_input_size[0] // 2
            pred = pred.flatten()
            for i in range(len(pred)):
                pred[i] += (pred[i] + 1) * half_input_len
            # （2）获取仿射矩阵的逆矩阵
            matrix_dst_inv = aidemo.invert_affine_transform(self.matrix_dst)
            matrix_dst_inv = matrix_dst_inv.flatten()
            # （3）对每个关键点进行逆变换
            half_out_len = len(pred) // 2
            for kp_id in range(half_out_len):
                old_x = pred[kp_id * 2]
                old_y = pred[kp_id * 2 + 1]
                # 逆变换公式
                new_x = old_x * matrix_dst_inv[0] + old_y * matrix_dst_inv[1] + matrix_dst_inv[2]
                new_y = old_x * matrix_dst_inv[3] + old_y * matrix_dst_inv[4] + matrix_dst_inv[5]
                pred[kp_id * 2] = new_x
                pred[kp_id * 2 + 1] = new_y
            return pred

    def get_affine_matrix(self,bbox):
        # 获取仿射矩阵，用于将边界框映射到模型输入空间
        with ScopedTiming("get_affine_matrix", self.debug_mode > 1):
            # 从边界框提取坐标和尺寸
            x1, y1, w, h = map(lambda x: int(round(x, 0)), bbox[:4])
            # 计算缩放比例，使得边界框映射到模型输入空间的一部分
            scale_ratio = (self.model_input_size[0]) / (max(w, h) * 1.5)
            # 计算边界框中心点在模型输入空间的坐标
            cx = (x1 + w / 2) * scale_ratio
            cy = (y1 + h / 2) * scale_ratio
            # 计算模型输入空间的一半长度
            half_input_len = self.model_input_size[0] / 2
            # 创建仿射矩阵并进行设置
            matrix_dst = np.zeros((2, 3), dtype=np.float)
            matrix_dst[0, 0] = scale_ratio
            matrix_dst[0, 1] = 0
            matrix_dst[0, 2] = half_input_len - cx
            matrix_dst[1, 0] = 0
            matrix_dst[1, 1] = scale_ratio
            matrix_dst[1, 2] = half_input_len - cy
            return matrix_dst
# 自定义注视估计任务类
class EyeGazeApp(AIBase):
    def __init__(self,kmodel_path,model_input_size,rgb888p_size=[1920,1080],display_size=[1920,1080],debug_mode=0):
        super().__init__(kmodel_path,model_input_size,rgb888p_size,debug_mode)
        # kmodel路径
        self.kmodel_path=kmodel_path
        # 注视估计模型输入分辨率
        self.model_input_size=model_input_size
        # sensor给到AI的图像分辨率，宽16字节对齐
        self.rgb888p_size=[ALIGN_UP(rgb888p_size[0],16),rgb888p_size[1]]
        # 视频输出VO分辨率，宽16字节对齐
        self.display_size=[ALIGN_UP(display_size[0],16),display_size[1]]
        # debug模式
        self.debug_mode=debug_mode
        # 长度阈值配置
        self.length_threshold = 100  # 默认50像素长度阈值
        # Ai2d实例，用于实现模型预处理
        self.ai2d=Ai2d(debug_mode)
        # 设置Ai2d的输入输出格式和类型
        self.ai2d.set_ai2d_dtype(nn.ai2d_format.NCHW_FMT,nn.ai2d_format.NCHW_FMT,np.uint8, np.uint8)

    # 配置预处理操作，这里使用了crop和resize，Ai2d支持crop/shift/pad/resize/affine
    def config_preprocess(self,det,input_image_size=None):
        with ScopedTiming("set preprocess config",self.debug_mode > 0):
            # 初始化ai2d预处理配置
            ai2d_input_size=input_image_size if input_image_size else self.rgb888p_size
            # 计算crop预处理参数
            x, y, w, h = map(lambda x: int(round(x, 0)), det[:4])
            # 设置crop预处理
            self.ai2d.crop(x,y,w,h)
            # 设置resize预处理
            self.ai2d.resize(nn.interp_method.tf_bilinear, nn.interp_mode.half_pixel)
            # 构建预处理流程,参数为预处理输入tensor的shape和预处理输出的tensor的shape
            self.ai2d.build([1,3,ai2d_input_size[1],ai2d_input_size[0]],[1,3,self.model_input_size[1],self.model_input_size[0]])

    # 自定义后处理，results是模型输出的array列表，这里调用了aidemo库的eye_gaze_post_process接口
    def postprocess(self,results):
        with ScopedTiming("postprocess",self.debug_mode > 0):
            post_ret = aidemo.eye_gaze_post_process(results)
            return post_ret[0],post_ret[1]


# 人脸标志解析
class FaceLandMark:
    def __init__(self,face_det_kmodel,face_landmark_kmodel,det_input_size,landmark_input_size,anchors,confidence_threshold=0.25,nms_threshold=0.3,rgb888p_size=[1920,1080],display_size=[1920,1080],debug_mode=0):
        self.face_det_kmodel=face_det_kmodel
        self.face_landmark_kmodel=face_landmark_kmodel
        self.det_input_size=det_input_size
        self.landmark_input_size=landmark_input_size
        self.anchors=anchors
        self.confidence_threshold=confidence_threshold
        self.nms_threshold=nms_threshold
        self.rgb888p_size=[ALIGN_UP(rgb888p_size[0],16),rgb888p_size[1]]
        self.display_size=[ALIGN_UP(display_size[0],16),display_size[1]]
        self.debug_mode=debug_mode
        self.dict_kp_seq = [
            [43, 44, 45, 47, 46, 50, 51, 49, 48],
            [97, 98, 99, 100, 101, 105, 104, 103, 102],
            [35, 36, 33, 37, 39, 42, 40, 41],
            [89, 90, 87, 91, 93, 96, 94, 95],
            [34, 88],
            [72, 73, 74, 86],
            [77, 78, 79, 80, 85, 84, 83],
            [52, 55, 56, 53, 59, 58, 61, 68, 67, 71, 63, 64],
            [65, 54, 60, 57, 69, 70, 62, 66],
            [1, 9, 10, 11, 12, 13, 14, 15, 16, 2, 3, 4, 5, 6, 7, 8, 0, 24, 23, 22, 21, 20, 19, 18, 32, 31, 30, 29, 28, 27, 26, 25, 17]
        ]
        self.color_list_for_osd_kp = [
            (255, 0, 255, 0),
            (255, 0, 255, 0),
            (255, 255, 0, 255),
            (255, 255, 0, 255),
            (255, 255, 0, 0),
            (255, 255, 170, 0),
            (255, 255, 255, 0),
            (255, 0, 255, 255),
            (255, 255, 220, 50),
            (255, 30, 30, 255)
        ]
        self.face_det=FaceDetApp(self.face_det_kmodel,model_input_size=self.det_input_size,anchors=self.anchors,confidence_threshold=self.confidence_threshold,nms_threshold=self.nms_threshold,rgb888p_size=self.rgb888p_size,display_size=self.display_size,debug_mode=0)
        self.face_landmark=FaceLandMarkApp(self.face_landmark_kmodel,model_input_size=self.landmark_input_size,rgb888p_size=self.rgb888p_size,display_size=self.display_size)
        self.face_det.config_preprocess()
    def run(self,input_np):
        det_boxes=self.face_det.run(input_np)
        landmark_res=[]
        for det_box in det_boxes:
            self.face_landmark.config_preprocess(det_box)
            res=self.face_landmark.run(input_np)
            landmark_res.append(res)
        return det_boxes,landmark_res
    def draw_result(self,pl,dets,landmark_res):
        # 只绘制嘴部区域和嘴巴张合度,通过串口打印指令和嘴巴坐标
        if not hasattr(self, 'mouth_start_time'):
            self.mouth_start_time = None
            self.mouth_signal_sent = False
        if dets:
            draw_img_np = np.zeros((self.display_size[1],self.display_size[0],4),dtype=np.uint8)
            draw_img = image.Image(self.display_size[0], self.display_size[1], image.ARGB8888, alloc=image.ALLOC_REF,data = draw_img_np)
            for pred in landmark_res:
                for sub_part_index in [7]:
                    sub_part = self.dict_kp_seq[sub_part_index]
                    face_sub_part_point_set = []
                    for kp_index in range(len(sub_part)):
                        real_kp_index = sub_part[kp_index]
                        x, y = pred[real_kp_index * 2], pred[real_kp_index * 2 + 1]
                        x = int(x * self.display_size[0] // self.rgb888p_size[0])
                        y = int(y * self.display_size[1] // self.rgb888p_size[1])
                        face_sub_part_point_set.append((x, y))
                    color = np.array(self.color_list_for_osd_kp[sub_part_index],dtype = np.uint8)
                    face_sub_part_point_set = np.array(face_sub_part_point_set)
                    aidemo.contours(draw_img_np, face_sub_part_point_set,-1,color,2,8)
                    if sub_part_index == 7:
                        y_coords = [kp[1] for kp in face_sub_part_point_set]
                        max_y = max(y_coords)
                        min_y = min(y_coords)
                        mouth_open = int((max_y - min_y) * self.rgb888p_size[1] // self.display_size[1])
                        sum_x = sum(kp[0] for kp in face_sub_part_point_set)
                        sum_y = sum(kp[1] for kp in face_sub_part_point_set)
                        center_x = sum_x // len(face_sub_part_point_set)
                        center_y = sum_y // len(face_sub_part_point_set)
                        print(f'Mouth Center: ({center_x}, {center_y})')
                        print(f'嘴巴状态：{"张开" if mouth_open > 100 else "闭合"}，开合度：{mouth_open}px')
                        text_x = int(face_sub_part_point_set[0][0])
                        text_y = int(face_sub_part_point_set[0][1]) + 30
                        # 显示open/close
                        if mouth_open > 100:
                            draw_img.draw_string(text_x, text_y, f'Open: {mouth_open}px', scale=4.0, color=(0, 255, 0))
                        else:
                            draw_img.draw_string(text_x, text_y, f'close: {mouth_open}px', scale=4.0, color=(255, 0, 0))
                        now = time.time()
                        if mouth_open > 100:
                            if self.mouth_start_time is None:
                                self.mouth_start_time = now
                                self.mouth_signal_sent = False
                            elif not self.mouth_signal_sent and (now - self.mouth_start_time) >= 5:
                                uart.write(bytes([5]))
                                self.mouth_signal_sent = True
                        else:
                            self.mouth_start_time = None
                            self.mouth_signal_sent = False
                        aidemo.contours(draw_img_np, face_sub_part_point_set,-1,color,2,8)
            pl.osd_img.copy_from(draw_img)

# 自定义注视估计类
class EyeGaze:
    def __init__(self,face_det_kmodel,eye_gaze_kmodel,det_input_size,eye_gaze_input_size,anchors,confidence_threshold=0.25,nms_threshold=0.3,rgb888p_size=[1920,1080],display_size=[1920,1080],debug_mode=0,length_threshold=200):
        self.face_det_kmodel=face_det_kmodel
        self.eye_gaze_kmodel=eye_gaze_kmodel
        self.det_input_size=det_input_size
        self.eye_gaze_input_size=eye_gaze_input_size
        self.anchors=anchors
        self.confidence_threshold=confidence_threshold
        self.nms_threshold=nms_threshold
        self.rgb888p_size=[ALIGN_UP(rgb888p_size[0],16),rgb888p_size[1]]
        self.display_size=[ALIGN_UP(display_size[0],16),display_size[1]]
        self.debug_mode=debug_mode
        self.length_threshold = 100
        self.face_det=FaceDetApp(self.face_det_kmodel,model_input_size=self.det_input_size,anchors=self.anchors,confidence_threshold=self.confidence_threshold,nms_threshold=self.nms_threshold,rgb888p_size=self.rgb888p_size,display_size=self.display_size,debug_mode=0)
        self.eye_gaze=EyeGazeApp(self.eye_gaze_kmodel,model_input_size=self.eye_gaze_input_size,rgb888p_size=self.rgb888p_size,display_size=self.display_size)
        self.face_det.config_preprocess()
    def run(self,input_np):
        det_boxes=self.face_det.run(input_np)
        eye_gaze_res=[]
        for det_box in det_boxes:
            self.eye_gaze.config_preprocess(det_box)
            pitch,yaw=self.eye_gaze.run(input_np)
            eye_gaze_res.append((pitch,yaw))
        return det_boxes,eye_gaze_res
    def draw_result(self,pl,dets,eye_gaze_res):
        # 只绘制主方向且主方向分量需大于副方向分量一定比例才判定为该方向
        DIRECTION_RATIO = 1.2  # 主方向分量需大于副方向分量1.2倍才判定
        if not hasattr(self, 'direction_start_time'):
            self.direction_start_time = None
            self.last_direction = None
            self.signal_sent = False
        if dets:
            for det,gaze_ret in zip(dets,eye_gaze_res):
                pitch , yaw = gaze_ret
                length = self.display_size[0]/ 2
                x, y, w, h = map(lambda x: int(round(x, 0)), det[:4])
                x = self.display_size[0] - (x * self.display_size[0] // self.rgb888p_size[0]) - w * self.display_size[0] // self.rgb888p_size[0]
                y = y * self.display_size[1] // self.rgb888p_size[1]
                w = w * self.display_size[0] // self.rgb888p_size[0]
                h = h * self.display_size[1] // self.rgb888p_size[1]
                center_x = (x + w / 2.0)
                center_y = (y + h / 2.0)
                dx = -length * math.sin(pitch) * math.cos(yaw)
                dy = -length * math.sin(yaw)
                # 判断主方向，需主方向分量大于副方向分量*比例
                direction = None
                if abs(dx) > abs(dy) * DIRECTION_RATIO:
                    if dx < 0:
                        direction = "left"
                    elif dx > 0:
                        direction = "right"
                elif abs(dy) > abs(dx) * DIRECTION_RATIO:
                    if dy < 0:
                        direction = "up"
                    elif dy > 0:
                        direction = "down"
                actual_length = math.sqrt(dx**2 + dy**2)
                now = time.time()
                if actual_length > self.length_threshold and direction:
                    if self.last_direction != direction:
                        self.direction_start_time = now
                        self.signal_sent = False
                        self.last_direction = direction
                    elif not self.signal_sent and self.direction_start_time and (now - self.direction_start_time) >= 5:
                        if direction == "up":
                            uart.write(bytes([1]))
                        elif direction == "down":
                            uart.write(bytes([2]))
                        elif direction == "left":
                            uart.write(bytes([3]))
                        elif direction == "right":
                            uart.write(bytes([4]))
                        self.signal_sent = True
                    # 绘制箭头和文字
                    if direction == "up":
                        pl.osd_img.draw_arrow(int(center_x), int(center_y), int(center_x), int(center_y - abs(dy)), color = (255,255,0,0), size = 80, thickness = 2)
                        pl.osd_img.draw_string(int(center_x), int(center_y - abs(dy)) - 40, "up", scale=6.0, color=(255,255,0))
                    elif direction == "down":
                        pl.osd_img.draw_arrow(int(center_x), int(center_y), int(center_x), int(center_y + abs(dy)), color = (255,255,0,0), size = 80, thickness = 2)
                        pl.osd_img.draw_string(int(center_x), int(center_y + abs(dy)) + 10, "down", scale=6.0, color=(255,255,0))
                    elif direction == "left":
                        pl.osd_img.draw_arrow(int(center_x), int(center_y), int(center_x - abs(dx)), int(center_y), color = (255,255,0,0), size = 80, thickness = 2)
                        pl.osd_img.draw_string(int(center_x - abs(dx)) - 40, int(center_y), "left", scale=6.0, color=(255,255,0))
                    elif direction == "right":
                        pl.osd_img.draw_arrow(int(center_x), int(center_y), int(center_x + abs(dx)), int(center_y), color = (255,255,0,0), size = 80, thickness = 2)
                        pl.osd_img.draw_string(int(center_x + abs(dx)) + 10, int(center_y), "right", scale=6.0, color=(255,255,0))
                else:
                    self.direction_start_time = None
                    self.signal_sent = False
                    self.last_direction = None
                # 打印方向信息始终保留
                if direction:
                    message = "注视方向: {}\n矢量分量 dx: {:.2f}, dy: {:.2f}\n".format(direction, dx, dy)
                    print(message)
                # 否则不显示任何内容

if __name__=="__main__":
    # 显示模式，默认"hdmi",可以选择"hdmi"和"lcd"，k230d受限于内存不支持
    display_mode="lcd"
    rgb888p_size = [1920, 1080]
    if display_mode=="hdmi":
        display_size=[1920,1080]
    else:
        display_size=[800,480]
    face_det_kmodel_path="/sdcard/examples/kmodel/face_detection_320.kmodel"
    face_landmark_kmodel_path="/sdcard/examples/kmodel/face_landmark.kmodel"
    eye_gaze_kmodel_path="/sdcard/examples/kmodel/eye_gaze.kmodel"
    anchors_path="/sdcard/examples/utils/prior_data_320.bin"
    face_det_input_size=[320,320]
    face_landmark_input_size=[192,192]
    eye_gaze_input_size=[448,448]
    confidence_threshold=0.5
    nms_threshold=0.2
    anchor_len=4200
    det_dim=4
    anchors = np.fromfile(anchors_path, dtype=np.float)
    anchors = anchors.reshape((anchor_len,det_dim))

    # 初始化PipeLine，只关注传给AI的图像分辨率，显示的分辨率
    pl=PipeLine(rgb888p_size=rgb888p_size,display_size=display_size,display_mode=display_mode)
    pl.create(hmirror=True)
    flm=FaceLandMark(face_det_kmodel_path,face_landmark_kmodel_path,det_input_size=face_det_input_size,landmark_input_size=face_landmark_input_size,anchors=anchors,confidence_threshold=confidence_threshold,nms_threshold=nms_threshold,rgb888p_size=rgb888p_size,display_size=display_size)
    eg=EyeGaze(face_det_kmodel_path,eye_gaze_kmodel_path,det_input_size=face_det_input_size,eye_gaze_input_size=eye_gaze_input_size,anchors=anchors,confidence_threshold=confidence_threshold,nms_threshold=nms_threshold,rgb888p_size=rgb888p_size,display_size=display_size)
    try:
        while True:
            with ScopedTiming("total",1):
                img=pl.get_frame()
                # 嘴巴张合与坐标检测
                det_boxes_lm,landmark_res=flm.run(img)
                flm.draw_result(pl,det_boxes_lm,landmark_res)
                # 注视方向检测（可选：可只对第一个人脸做注视方向）
                det_boxes_gaze,eye_gaze_res=eg.run(img)
                eg.draw_result(pl,det_boxes_gaze,eye_gaze_res)
                pl.show_image()
                gc.collect()
    except Exception as e:
        sys.print_exception(e)
    finally:
        flm.face_det.deinit()
        flm.face_landmark.deinit()
        eg.face_det.deinit()
        eg.eye_gaze.deinit()
        pl.destroy()
