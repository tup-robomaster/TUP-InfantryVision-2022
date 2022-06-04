#include "./include/HaiKangCamera.h"

HaiKangCamera::HaiKangCamera()
{
}

HaiKangCamera::~HaiKangCamera()
{
    nRet = MV_CC_FreeImageBuffer(handle, (&pFrame));
    if(nRet == MV_OK)
    {
        fmt::print(fmt::fg(fmt::color::blue), "[Camera] free image buffer succeed!");
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[Camera] free image buffer failed!");
    }
}

int HaiKangCamera::StartDevice(int serial_number)
{   //打开设备
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    //枚举设备
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if(MV_OK != nRet)
    {
        fmt::print(fmt::fg(fmt::color::red), "EnumDevices failed! nRet [%x]\n", nRet);
        return -1;
    }

    if(stDeviceList.nDeviceNum > 0) //设备数量不为0
    {
        // for(int i = 0; i < stDeviceList.nDeviceNum; i++)
        // {
        //     printf("[device %d]:\n", i);
        //     MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        //     if(NULL == pDeviceInfo)
        //     {
        //         printf("pDeviceInfo is NULL!\n");
        //         return -1;
        //     }
        //     HaiKangCamera::PrintDeviceInfo(pDeviceInfo);
        // }

        //打开设备
        //选择设备并创建句柄
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[serial_number]);
        if(MV_OK != nRet)
        {
            fmt::print(fmt::fg(fmt::color::red), "CreateHandle failed! nRet [%x]\n", nRet);
            return -1;
        }

        //打开设备
        nRet = MV_CC_OpenDevice(handle);
        if(MV_OK == nRet)
        {
            printf("OpenDevice succeed!\n");
            return serial_number;
        }
        if(MV_OK != nRet)
        {
            printf("OpenDevice failed! nRet [%x]\n", nRet);
            return -1;
        }
    }
    else
    {
        printf("Find No Devices!\n");
        return -1;
    }

    return serial_number;
}

bool HaiKangCamera::SetStreamOn()
{   //开始采集
    //设置触发模式为off
    this->nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
    if(MV_OK != nRet)
    {
        printf("SetEnumValue failed! nRet [%x]\n", nRet);
        return false;
    }

    //设置采集模式为连续采集
    this->nRet = MV_CC_SetEnumValue(handle, "AcquisitionMode", 2);
    if(MV_OK != nRet)
    {
        printf("SetEnumValue failed! nRet [%x]\n", nRet);
        return false;
    }

    // //获取数据包大小
    // MVCC_INTVALUE stParam;
    // memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    // nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
    // if (MV_OK != nRet)
    // {
    //     printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
    //     return false;
    // }
    // g_nPayloadSize = stParam.nCurValue;

    //开始取流
    nRet = MV_CC_StartGrabbing(handle);
    if(MV_OK == nRet)
    {
        printf("StartGrabbing succeed!\n");
        return true;
    }
    else
    {
        printf("StartGrabbing failed! nRet [%x]\n", nRet);
        return false;
    }
    
    //创建相机图像采集线程
    // pthread_t tid;
    // nRet = pthread_create(&tid, NULL, HaiKangCamera::WorkThread, handle);
    // if(MV_OK == nRet)
    // {
    //     printf("pthread_create succeed!\n");
    //     return true;
    // }
    // else
    // {
    //     printf("pthread_create failed! nRet [%d]\n", nRet);
    //     return false;
    // }
}

bool HaiKangCamera::SetResolution(int width, int height)
{   //TODO:分辨率根据相机采集上限设置，目前设置为1280*1024
    nRet = MV_OK;

    //设置像素格式
    nRet = MV_CC_SetPixelFormat(handle, PixelType_Gvsp_BGR8_Packed);
    if(nRet != MV_OK)
    {
        printf("setPixelFormat failed!\n");
        return false;
    }

    nRet = MV_CC_SetIntValue(this->handle, "Width", width);
    if(nRet != MV_OK)
    {
        printf("setResolution failed!\n");
        return false;
    }
    nRet = MV_CC_SetIntValue(this->handle, "Height", height);

    if(nRet == MV_OK)
    {
        printf("setResolution succeed!\n");
        return true;
    }
    else
    {
        printf("setResolution failed!\n");
        return false;
    }
}

bool HaiKangCamera::SetExposureTime(float ExposureTime)
{   //设置曝光时间
    nRet = MV_CC_SetFloatValue(this->handle, "ExposureTime", ExposureTime);
    if(nRet == MV_OK)
    {
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] 曝光值设置成功\n");
        return true;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 曝光值设置失败\n");
        return false;
    }
}

bool HaiKangCamera::SetGAIN(int value, int ExpGain)
{   //曝光增益
    if(value == 0)
    {
        nRet = MV_CC_SetEnumValue(handle, "GainMode", R_CHANNEL);
    }
    else if(value == 1)
    {
        nRet = MV_CC_SetEnumValue(handle, "GainMode", G_CHANNEL);
    }
    else if(value == 2)
    {
        nRet = MV_CC_SetEnumValue(handle, "GainMode", B_CHANNEL);
    }
    else
    {
        nRet = MV_CC_SetFloatValue(handle, "Gain", ExpGain);
        if(nRet == MV_OK)
        {
            printf("设置曝光增益成功！\n");
            return true;
        }
        else
        {
            printf("设置曝光增益失败！\n");
            return false;
        }
    }

    nRet = MV_CC_SetFloatValue(handle, "Gain", ExpGain);
    if(nRet == MV_OK)
    {
        printf("设置曝光增益成功！\n");
        return true;
    }
    else
    {
        printf("设置曝光增益失败！\n");
        return false;
    }
}

bool HaiKangCamera::Set_Auto_BALANCE()
{   //自动白平衡（具有记忆功能）
    this->nRet = MV_CC_SetEnumValue(this->handle, "BalanceWhiteAuto", 1);
    if(nRet != MV_OK)
    {
        printf("自动白平衡设置失败！\n");
        return false;
    }
    else
    {   
        printf("自动白平衡设置成功！\n");
        return true;
    }
}

bool HaiKangCamera::Set_BALANCE(int value, unsigned int value_number)
{   //手动白平衡（具有记忆功能））
    //关闭自动白平衡
    this->nRet = MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_OFF);
    if(nRet != MV_OK)
    {
        printf("关闭自动白平衡失败！\n");
        return false;
    }

    //设置RGB三通道白平衡值
    if(value == 0)
    {
        this->nRet = MV_CC_SetBalanceRatioRed(handle, value_number);
        
        if(nRet == MV_OK)
        {
            printf("set R_Balance succeed！\n");
        }
        else
        {
            printf("set R_Balance failed！\n");
            return false;
        }
    }
    else if(value == 1)
    {
        this->nRet = MV_CC_SetBalanceRatioGreen(handle, value_number);

        if(nRet == MV_OK)
        {
            printf("set G_Balance succeed！\n");
        }
        else
        {
            printf("set G_Balance failed！\n");
            return false;
        }
    }
    else if(value == 2)
    {
        this->nRet = MV_CC_SetBalanceRatioBlue(handle, value_number);
        
        if(nRet == MV_OK)
        {
            printf("set B_Balance succeed！\n");
        }
        else
        {
            printf("set B_Balance failed！\n");
            return false;
        }
    }
    return true;
}

bool HaiKangCamera::Set_Gamma(bool set_status,double dGammaParam)
{   //设置Gamma值
    if(set_status == true)
    {
        nRet = MV_CC_SetEnumValue(handle, "Gamma", 1);
        if(nRet == MV_OK)
        {
            printf("设置Gamma值成功！\n");
            return true;
        }
        else
        {
            printf("设置Gamma值失败！\n");
            return false;
        }
    }
    else
    {
        nRet = MV_CC_SetEnumValue(handle, "Gamma", 0);
        if(nRet == MV_OK)
        {
            printf("关闭Gamma值成功！\n");
            return true;
        }
        else
        {
            printf("关闭Gamma值失败！\n");
            return false;
        }
    }
}

bool HaiKangCamera::Color_Correct(bool value)
{   //设置色彩校正
    if(value == true)
    {
        nRet = MV_CC_SetEnumValue(handle, "ColorCorrection", 1);
        if(nRet == MV_OK)
        {
            printf("设置色彩校正成功！\n");
            return true;
        }
        else
        {
            printf("设置色彩校正失败！\n");
            return false;
        }
    }
    else
    {
        nRet = MV_CC_SetEnumValue(handle, "ColorCorrection", 0);
        if(nRet == MV_OK)
        {
            printf("关闭色彩校正成功！\n");
            return true;
        }
        else
        {
            printf("关闭色彩校正失败！\n");
            return false;
        }
    }
}

bool HaiKangCamera::Set_Contrast(bool set_status,int dContrastParam)
{   //设置对比度
    if(set_status == true)
    {
        nRet = MV_CC_SetEnumValue(handle, "Contrast", 1);
        if(nRet == MV_OK)
        {
            printf("设置对比度成功！\n");
            return true;
        }
        else
        {
            printf("设置对比度失败！\n");
            return false;
        }
    }
    else
    {
        nRet = MV_CC_SetEnumValue(handle, "Contrast", 0);
        if(nRet == MV_OK)
        {
            printf("关闭对比度成功！\n");
            return true;
        }
        else
        {
            printf("关闭对比度失败！\n");
            return false;
        }
    }
}

bool HaiKangCamera::UpdateTimestampOffset(std::chrono::_V2::steady_clock::time_point time_start)
{   //计算时间戳偏移
    std::chrono::_V2::steady_clock::time_point time_end = std::chrono::_V2::steady_clock::now();
    std::chrono::duration<double> time_span = time_end - time_start;
    timestamp_offset = time_span.count() * 1000;
    return true;
}

int HaiKangCamera::Get_TIMESTAMP()
{   //获取时间戳
    std::chrono::_V2::steady_clock::time_point time_start = std::chrono::_V2::steady_clock::now();
    return ((int)time_start.time_since_epoch().count() - timestamp_offset);
}

bool HaiKangCamera::GetMat(cv::Mat &Src)
{
    // ch:获取数据包大小 | en:Get payload size
    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
    if (MV_OK != nRet)
    {
        printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
        return NULL;
    }

    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    unsigned char * pData = (unsigned char *)malloc(sizeof(unsigned char) * stParam.nCurValue);
    if (NULL == pData)
    {
        return NULL;
    }
    unsigned int nDataSize = stParam.nCurValue;
    
    //从缓存区读取图像
    nRet = MV_CC_GetOneFrameTimeout(handle, pData, nDataSize, &stImageInfo, 1000);
    if(nRet != MV_OK)
    {
        printf("No data[%x]\n", nRet);
        return false;
    }

    cv::Mat src = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3);
    memcpy(src.data, pData, stImageInfo.nWidth * stImageInfo.nHeight * 3);
    src.copyTo(Src);
    // cv::Mat src(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, pData);
    // src.copyTo(Src);

    if(pData)
    {
        delete[] pData;
        pData = NULL;
    }
    return true;
}

bool HaiKangCamera::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }
    else
    {
        printf("Not support.\n");
    }
    return true;
}

void* HaiKangCamera::WorkThread(void* pUser)
{
    int nRet = MV_OK;

    //获取数据包大小
    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    nRet = MV_CC_GetIntValue(pUser, "DataPackageSize", &stParam);
    if (MV_OK != nRet)
    {
        printf("MV_CC_GetIntValue DataPackageSize failed! nRet [%x]\n", nRet);
        return NULL;
    }

    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    unsigned char *pData = (unsigned char *)malloc(sizeof(unsigned char) * (stParam.nCurValue));
    if (NULL == pData)
    {
        printf("malloc failed!\n");
        return NULL;
    }
    unsigned int nDataSize = stParam.nCurValue;

    while(true)
    {
        // if(this->g_bExit)
        // {
        //     break;
        // }
        nRet = MV_CC_GetOneFrameTimeout(pUser, pData, nDataSize, &stImageInfo, 1000);

        if(MV_OK != nRet)
        {
            printf("MV_CC_GetOneFrameTimeout failed! nRet [%x]\n", nRet);
            continue;
        }
        else
        {
            printf("GetOneFrame, Width[%d], Height[%d], nFrameNum[%d]\n", 
                stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);
        }
    }

    free(pData);
    return 0;
}

void HaiKangCamera::open_HaiKang_camera(int exposure_time, float exposure_gain, float b_balance, float g_balance, float r_balance)
{
    int nRet = MV_OK;
    void* handle = NULL;

    do
    {
        MV_CC_DEVICE_INFO_LIST stDeviceList;
        memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

        //枚举设备
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
        if(MV_OK != nRet)
        {
            printf("EnumDevices failed! nRet [%x]\n", nRet);
            break;
        }

        if(stDeviceList.nDeviceNum > 0) //设备数量不为0
        {
            for(int i = 0; i < stDeviceList.nDeviceNum; i++)
            {
                printf("[device %d]:\n", i);
                MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
                if(NULL == pDeviceInfo)
                {
                    printf("pDeviceInfo is NULL!\n");
                    break;
                }
                HaiKangCamera::PrintDeviceInfo(pDeviceInfo);
            }
        }
        else
        {
            printf("Find No Devices!\n");
            break;
        }

        // printf("Please input the index of the device which you want to use(0~%d):", stDeviceList.nDeviceNum - 1);
        // unsigned int nIndex = 0;
        // //手动输入设备索引
        // scanf("%d", &nIndex);
        // if(nIndex > stDeviceList.nDeviceNum)
        // {
        //     printf("Input Error!\n");
        //     break;
        // }

        //选择设备并创建句柄
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[0]);
        if(MV_OK != nRet)
        {
            printf("CreateHandle failed! nRet [%x]\n", nRet);
            break;
        }

        //打开设备
        nRet = MV_CC_OpenDevice(handle);
        if(MV_OK != nRet)
        {
            printf("OpenDevice failed! nRet [%x]\n", nRet);
            break;
        }

        //设置触发模式为off
        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
        if(MV_OK != nRet)
        {
            printf("SetEnumValue failed! nRet [%x]\n", nRet);
            break;
        }

        //获取数据包大小
        MVCC_INTVALUE stParam;
        memset(&stParam, 0, sizeof(MVCC_INTVALUE));
        nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
        if (MV_OK != nRet)
        {
            printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
            break;
        }
        g_nPayloadSize = stParam.nCurValue;

        //开始取流
        nRet = MV_CC_StartGrabbing(handle);
        if(MV_OK != nRet)
        {
            printf("StartGrabbing failed! nRet [%x]\n", nRet);
            break;
        }

        //创建线程
        pthread_t tid;
        nRet = pthread_create(&tid, NULL, HaiKangCamera::WorkThread, handle);
        if(0 != nRet)
        {
            printf("pthread_create failed! nRet [%d]\n", nRet);
            break;
        }
        
        //停止取流
        nRet = MV_CC_StopGrabbing(handle);
        if(MV_OK != nRet)
        {
            printf("StopGrabbing failed! nRet [%x]\n", nRet);
            break;
        }

        //关闭设备
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            printf("ClosDevice fail! nRet [0x%x]\n", nRet);
            break;
        }

        //销毁句柄
        nRet = MV_CC_DestroyHandle(handle);
        if(MV_OK != nRet)
        {
            printf("DestroyHandle failed! nRet [%x]\n", nRet);
            break;
        }
    } while (0);

    if(nRet != MV_OK)
    {
        if(handle)
        {
            MV_CC_DestroyHandle(handle);
            handle = NULL;
        }
    }

    printf("exit!\n");
}