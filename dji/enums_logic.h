/* SPDX-License-Identifier: MIT */
/*
 * Copyright (C) 2025 SZ DJI Technology Co., Ltd.
 *
 * All information contained herein is, and remains, the property of DJI.
 * The intellectual and technical concepts contained herein are proprietary
 * to DJI and may be covered by U.S. and foreign patents, patents in process,
 * and protected by trade secret or copyright law.  Dissemination of this
 * information, including but not limited to data and other proprietary
 * material(s) incorporated within the information, in any form, is strictly
 * prohibited without the express written consent of DJI.
 *
 * If you receive this source code without DJI’s authorization, you may not
 * further disseminate the information, and you must immediately remove the
 * source code and notify DJI of its removal. DJI reserves the right to pursue
 * legal actions against you for any loss(es) or damage(s) caused by your
 * failure to do so.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CMD_NO_RESPONSE = 0x00,     // Command frame - No response required after sending data
                                // 命令帧 - 发送数据后不需要应答
    CMD_RESPONSE_OR_NOT = 0x01, // Command frame - Response required, no error if not received
                                // 命令帧 - 发送数据后需要应答，没收到结果不报错
    CMD_WAIT_RESULT = 0x02,     // Command frame - Response required, error if not received
                                // 命令帧 - 发送数据后需要应答，没收到结果会报错

    ACK_NO_RESPONSE = 0x20,     // Response frame - No response required (00100000)
                                // 应答帧 - 不需要应答 (00100000)
    ACK_RESPONSE_OR_NOT = 0x21, // Response frame - Response required, no error if not received (00100001)
                                // 应答帧 - 需要应答，没收到结果不报错 (00100001)
    ACK_WAIT_RESULT = 0x22      // Response frame - Response required, error if not received (00100010)
                                // 应答帧 - 需要应答，没收到结果会报错 (00100010)
} cmd_type_t;

typedef enum {
    CAMERA_MODE_SLOW_MOTION = 0x00,      // Slow Motion Mode
                                         // 慢动作模式
    CAMERA_MODE_NORMAL = 0x01,           // Normal Mode
                                         // 普通模式
    CAMERA_MODE_TIMELAPSE_STATIC = 0x02, // Timelapse Mode 1 - Static Timelapse
                                         // 延时摄影模式 1 静止延时
    CAMERA_MODE_PHOTO = 0x05,            // Normal Photo Mode
                                         // 普通拍照模式
    CAMERA_MODE_TIMELAPSE_MOTION = 0x0A, // Timelapse Mode 2 - Motion Timelapse
                                         // 延时摄影模式 2 运动延时
    CAMERA_MODE_LIVE_STREAMING = 0x1A,   // Live Streaming Mode
                                         // 直播模式
    CAMERA_MODE_UVC_STREAMING = 0x23,    // UVC Streaming Mode
                                         // UVC 直播模式
    CAMERA_MODE_LOW_LIGHT_VIDEO = 0x28,  // Low Light Video (Super Night Scene)
                                         // 低光视频（超级夜景）
    CAMERA_MODE_SMART_TRACKING = 0x34    // Smart Tracking
                                         // 智能跟随
} camera_mode_t;
const char *camera_mode_to_string(camera_mode_t mode);

typedef enum {
    CAMERA_STATUS_SCREEN_OFF = 0x00,         // Screen Off
                                             // 屏幕关闭
    CAMERA_STATUS_LIVE_STREAMING = 0x01,     // Live Streaming
                                             // 直播
    CAMERA_STATUS_PLAYBACK = 0x02,           // Playback
                                             // 回放
    CAMERA_STATUS_PHOTO_OR_RECORDING = 0x03, // Taking Photo or Recording
                                             // 拍照或录像中
    CAMERA_STATUS_PRE_RECORDING = 0x05       // Pre-recording
                                             // 预录制中
} camera_status_t;
const char *camera_status_to_string(camera_status_t status);

typedef enum {
    VIDEO_RESOLUTION_1080P = 10,   // 1920x1080P
    VIDEO_RESOLUTION_2K_16_9 = 45, // 2720x1530P 2.7K 16:9
    VIDEO_RESOLUTION_2K_4_3 = 95,  // 2720x2040P 2.7K 4:3
    VIDEO_RESOLUTION_4K_16_9 = 16, // 4096x2160P 4K 16:9
    VIDEO_RESOLUTION_4K_4_3 = 103  // 4096x3072P 4K 4:3
} video_resolution_t;
const char *video_resolution_to_string(video_resolution_t res);

typedef enum {
    FPS_24 = 1,   // 24fps
    FPS_25 = 2,   // 25fps
    FPS_30 = 3,   // 30fps
    FPS_48 = 4,   // 48fps
    FPS_50 = 5,   // 50fps
    FPS_60 = 6,   // 60fps
    FPS_100 = 10, // 100fps
    FPS_120 = 7,  // 120fps
    FPS_200 = 19, // 200fps
    FPS_240 = 8   // 240fps
} fps_idx_t;
const char *fps_idx_to_string(fps_idx_t fps);

typedef enum {
    EIS_MODE_OFF = 0,     // Off
                          // 关闭
    EIS_MODE_RS = 1,      // RS
    EIS_MODE_RS_PLUS = 2, // RS+
    EIS_MODE_HB = 3,      // -HB
    EIS_MODE_HS = 4       // -HS
} eis_mode_t;
const char *eis_mode_to_string(eis_mode_t mode);

typedef enum {
    PUSH_MODE_OFF = 0,                   // Off
                                         // 关闭
    PUSH_MODE_SINGLE,                    // Single
                                         // 单次
    PUSH_MODE_PERIODIC,                  // Periodic
                                         // 周期
    PUSH_MODE_PERIODIC_WITH_STATE_CHANGE // Periodic + State Change Push
                                         // 周期 + 状态变化推送
} push_mode_t;

typedef enum {
    PUSH_FREQ_10HZ = 1 // 10Hz
} push_freq_t;

#ifdef __cplusplus
}
#endif