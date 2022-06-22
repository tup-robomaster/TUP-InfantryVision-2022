#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fmt/format.h>
#include <fmt/color.h>
#include <opencv2/opencv.hpp>
#include "MvCameraControl.h"

using namespace std;
using namespace cv;

typedef enum _GAIN_MODE_
{
    R_CHANNEL,
    G_CHANNEL,
    B_CHANNEL
} GAIN_MODE;

class HaiKangCamera
{
    int nRet = MV_OK;
    void* handle = NULL;
    MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    MV_FRAME_OUT pFrame = {0};
    bool g_bExit = false;
    unsigned int g_nPayloadSize = 0;

    int timestamp_offset = 0;

public:
    //相机库初始化
    HaiKangCamera();

    //打开设备
    int StartDevice(int serial_number);

    //开始采集
    bool SetStreamOn();

    //设置图像分辨率
    bool SetResolution(int width = 1280, int height = 1024);
    
    //设置曝光时间
    bool SetExposureTime(float ExposureTime);

    //设置曝光增益
    bool SetGAIN(int value, int ExpGain);

    //自动白平衡
    bool Set_Auto_BALANCE();

    //手动白平衡
    bool Set_BALANCE(int value, unsigned int value_number);

    //Gamma校正
    bool Set_Gamma(bool set_status,double dGammaParam);

    //色彩校正
    bool Color_Correct(bool value);

    //对比度调整
    bool Set_Contrast(bool set_status,int dContrastParam);

    //采集一张图像更新一次时间戳
    bool UpdateTimestampOffset(std::chrono::_V2::steady_clock::time_point time_start);

    //采集图像（原图像转rgb）
    bool GetMat(cv::Mat &Src);

    //读取相机时间戳
    int Get_TIMESTAMP();

    ~HaiKangCamera();

};