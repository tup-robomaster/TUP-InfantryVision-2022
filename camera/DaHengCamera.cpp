
/*********************************************************************************
    FileName: 
    Author: 周俊平;刘上;赵梓合;顾昊
    Version: 1.0.0
    Date: 2021.04.10
    *Description: 大恒相机MER_139_210U3C SDK
    *Function List:
        1.DaHengCamera 初始化相机库
        2.StartDevice    打开设备
        3.SetStreamOn 开始采集
        4.SetResolution  设置分辨率
        5.SetExposureTime   设置曝光值
        6.SetGAIN   设置增益
        7.Set_BALANCE_AUTO  设置白平衡模式
        8.GetMat   得到Mat图像
**********************************************************************************/
#include "./DaHengCamera.h"
// #include "../Debug.h"

/**
 * @brief 相机构建函数  完成库的初始化
 */
DaHengCamera::DaHengCamera()
{
    //初始化库
    status = GXInitLib();
    //检测初始化是否成功
    if (status != GX_STATUS_SUCCESS)
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 相机库初始化失败!\n");
    }
}

/**
 * @brief 打开相机
 * @param serial_number为要打开设备的序列号
 * @return 返回检测到的连接相机个数
 */
int DaHengCamera::StartDevice(int serial_number)
{
    uint32_t nDeviceNum = 0;
    //枚 举 设 备 列 表
    status = GXUpdateDeviceList(&nDeviceNum, 1000);
    if (serial_number > nDeviceNum)
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 设备号错误，超过所枚举数量\n");
        return -1;
    }
    //打 开 设 备
    status = GXOpenDeviceByIndex(serial_number, &hDevice);
    if (status == GX_STATUS_SUCCESS)
    {
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] 设备打开成功!\n");
        return nDeviceNum;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 设备打开失败!\n");
        return -1;
    }
}

/**
 * @brief DaHengCamera::SetStreamOn 设置设备开始采集，设置分辨率应在采集图像之前
 * @return bool 返回是否设置成功
 */
bool DaHengCamera::SetStreamOn()
{
    //    status = GXSetAcqusitionBufferNumber(hDevice,1);
    //    if(status == GX_STATUS_SUCCESS){
    //        cout<<"buffer设置成功!"<<endl;
    //    }else{
    //        cout<<"buffer设置失败!"<<endl;
    //    }
    //    int64_t nAcqMode = GX_ACQ_MODE_CONTINUOUS;
    //    status = GXSetEnum(hDevice, GX_ENUM_ACQUISITION_MODE, nAcqMode);
    //    if(status == GX_STATUS_SUCCESS){
    //        cout<<"设置单帧成功!"<<endl;
    //        return true;
    //    }else{
    //        if(status == GX_STATUS_INVALID_ACCESS){
    //            cout<<"单帧采集功能当前不可访问!"<<endl;
    //        }
    //        if(status == GX_STATUS_INVALID_PARAMETER){
    //            cout<<"单帧采集功能无效参数!"<<endl;
    //        }
    //        if(status == GX_STATUS_INVALID_CALL){
    //            cout<<"单帧采集功能无效的接口调用!"<<endl;
    //        }
    //        cout<<"设置单帧失败!"<<endl;
    //        return false;
    //    }
    //发 送 开 始 采 集 命 令
    //    status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_START);
    //    if(status == GX_STATUS_SUCCESS){
    //        cout<<"开始采集图像!"<<endl;
    //        return true;
    //    }else{
    //        cout<<"采集失败!"<<endl;
    //        return false;
    //    }

    /**********************/
    //设置buffer数量
    //设 置 采 集 buffer 个 数
    status = GXSetAcqusitionBufferNumber(hDevice, 2);
    if (status == GX_STATUS_SUCCESS)
    {
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] buffer设置成功!\n");
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] buffer设置失败!\n");
    }

    status = GXSetBool(hDevice, GX_BOOL_CHUNKMODE_ACTIVE, true);
    if (status == GX_STATUS_SUCCESS)
    {
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] 帧信息模式已设置为使能!\n");
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 帧信息模式设置失败!\n");
    }

    status = status = GXSetEnum(hDevice, GX_ENUM_CHUNK_SELECTOR, GX_CHUNK_SELECTOR_CHUNK_TIME_STAMP);
    if (status == GX_STATUS_SUCCESS)
    {
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] 时间戳帧信息已启用!\n");
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 时间戳帧信息启用失败!\n");
    }
    //开 采
    status = GXStreamOn(hDevice);
    if (status == GX_STATUS_SUCCESS)
    {
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] 开始采集图像!\n");
        return true;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 采集失败!\n");
        return false;
    }
}
/**
 * @brief DaHengCamera::UpdateTimestamp 进行一次采图，更新时间戳
 * @param Src 引入方式传递
 * @return bool 返回是否成功
 */
bool DaHengCamera::UpdateTimestampOffset(std::chrono::_V2::steady_clock::time_point time_start)
{
    //清空缓冲队列
    int64_t nPayLoadSize = 0;
    //获 取 图 像 buffer 大 小 , 下 面 动 态 申 请 内 存
    status = GXGetInt(hDevice, GX_INT_PAYLOAD_SIZE, &nPayLoadSize);
    if (status == GX_STATUS_SUCCESS && nPayLoadSize > 0)
    {
        //定 义 GXGetImage 的 传 入 参 数
        GX_FRAME_DATA stFrameData;
        //根 据 获 取 的 图 像 buffer 大 小 m_nPayLoadSize 申 请 buffer
        stFrameData.pImgBuf = malloc((size_t)nPayLoadSize);

        //发 送 开 始 采 集 命 令
        int64_t nAcqMode = GX_ACQ_MODE_SINGLE_FRAME;
        status = GXSetEnum(hDevice, GX_ENUM_ACQUISITION_MODE, nAcqMode);
        status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_START);
        if (status == GX_STATUS_SUCCESS)
        {
            //调 用 GXGetImage 取 一 帧 图 像
            status = GXGetImage(hDevice, &stFrameData, 100);
            auto time_cam_end = std::chrono::steady_clock::now();
            lastImgTimestamp = stFrameData.nTimestamp;
            int program_timestamp = (int)std::chrono::duration<double,std::milli>(time_cam_end - time_start).count();
            int cam_timestamp = Get_TIMESTAMP();
            timestamp_offset = cam_timestamp - program_timestamp;
            return true;
        }
    }
    return false;
}

/**
 * @brief DaHengCamera::GetMat 读取图像
 * @param Src 引入方式传递
 * @return bool 返回是否成功
 */
bool DaHengCamera::GetMat(Mat &Src)
{
    // ------------------------------------------- For Soft Trigger------------------------------------------------------------
    // int64_t nPayLoadSize = 0;
    // //获 取 图 像 buffer 大 小 , 下 面 动 态 申 请 内 存
    // status = GXGetInt(hDevice, GX_INT_PAYLOAD_SIZE, &nPayLoadSize);
    // if (status == GX_STATUS_SUCCESS && nPayLoadSize > 0)
    // {
    //     //定 义 GXGetImage 的 传 入 参 数
    //     GX_FRAME_DATA stFrameData;
    //     //根 据 获 取 的 图 像 buffer 大 小 m_nPayLoadSize 申 请 buffer
    //     stFrameData.pImgBuf = malloc((size_t)nPayLoadSize);

    //     //发 送 开 始 采 集 命 令
    //     // int64_t nAcqMode = GX_ACQ_MODE_SINGLE_FRAME;
    //     // status = GXSetEnum(hDevice, GX_ENUM_ACQUISITION_MODE, nAcqMode);
    //     status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_START);
    //     if (status == GX_STATUS_SUCCESS)
    //     {
    //         //调 用 GXGetImage 取 一 帧 图 像
    //         status = GXGetImage(hDevice, &stFrameData, 100);
    //         lastImgTimestamp = stFrameData.nTimestamp;
    //     }
    //     else
    //     {
    //         return false;
    //     }

    //     if (status == GX_STATUS_SUCCESS && stFrameData.nStatus == GX_FRAME_STATUS_SUCCESS)
    //     {
    //         lastImgTimestamp = stFrameData.nTimestamp;
    //         char *pRGB24Buf = new char[stFrameData.nWidth * stFrameData.nHeight * 3]; //输 出 图 像 RGB 数 据
    //         if (pRGB24Buf == NULL)
    //         {
    //             return false;
    //         }
    //         else
    //         {
    //             memset(pRGB24Buf, 0, stFrameData.nWidth * stFrameData.nHeight * 3 * sizeof(char));
    //             //缓 冲 区 初 始 化
    //         }
    //         DX_BAYER_CONVERT_TYPE cvtype = RAW2RGB_NEIGHBOUR3; //选 择 插 值 算 法
    //         DX_PIXEL_COLOR_FILTER nBayerType = DX_PIXEL_COLOR_FILTER(BAYERBG);
    //         //选 择 图 像 Bayer 格 式
    //         bool bFlip = false;

    //         VxInt32 DxStatus = DxRaw8toRGB24(stFrameData.pImgBuf, pRGB24Buf, stFrameData.nWidth, stFrameData.nHeight, cvtype, nBayerType, bFlip);
    //         if (DxStatus != DX_OK)
    //         {
    //             fmt::print(fmt::fg(fmt::color::red), "[CAMERA] Raw8 to RGB24 failed!\n");
    //             if (pRGB24Buf != NULL)
    //             {
    //                 delete[] pRGB24Buf;
    //                 pRGB24Buf = NULL;
    //             }
    //             return false;
    //         }

    //         // if (set_contrast)
    //         // {
    //         //     DxStatus = DxContrast(pRGB24Buf, pRGB24Buf,stFrameData.nWidth * stFrameData.nHeight * 3, contrast_factor);
    //         //     if (DxStatus != DX_OK)
    //         //         cout << "Contrast Set Failed" <<endl;
    //         // }
    //         // if (set_color)
    //         // {
    //         //     DxStatus = DxImageImprovment(pRGB24Buf, pRGB24Buf,stFrameData.nWidth, stFrameData.nHeight, nColorCorrectionParam,NULL,pGammaLut);
    //         //     if (DxStatus != DX_OK)
    //         //         fmt::print(fmt::fg(fmt::color::red), "[CAMERA] Color Set Failed!\n");
    //         // }
    //         // if (set_saturation)
    //         // {
    //         //     DxStatus = DxSaturation(pRGB24Buf, pRGB24Buf,stFrameData.nWidth * stFrameData.nHeight * 3, saturation_factor);
    //         //     if (DxStatus != DX_OK)
    //         //         cout << "Saturation Set Failed" <<endl;
    //         // }

    //         Mat src = Mat(stFrameData.nHeight, stFrameData.nWidth, CV_8UC3);
    //         memcpy(src.data, pRGB24Buf, stFrameData.nWidth * stFrameData.nHeight * 3);
    //         src.copyTo(Src);

    //         delete[] pRGB24Buf;
    //         pRGB24Buf = NULL;
    //         return true;
    //     }
    //     else
    //     {
    //         // cout << "读取图片缓冲失败" << endl;
    //         fmt::print(fmt::fg(fmt::color::red), "[CAMERA] GetMat:采图失败\n");
    //         return false;
    //     }
    // }
    // else
    // {
    //     return false;
    // }
    // ------------------------------------------- For Stream------------------------------------------------------------
    //调 用 GXDQBuf 取 一 帧 图 像
    status = GXDQBuf(hDevice, &pFrameBuffer, 1000);
    if (status == GX_STATUS_SUCCESS && pFrameBuffer->nStatus == GX_FRAME_STATUS_SUCCESS)
    {
        lastImgTimestamp = pFrameBuffer->nTimestamp;
        char *pRGB24Buf = new char[pFrameBuffer->nWidth * pFrameBuffer->nHeight * 3]; //输 出 图 像 RGB 数 据
        if (pRGB24Buf == NULL)
        {
            return false;
        }
        else
        {
            memset(pRGB24Buf, 0, pFrameBuffer->nWidth * pFrameBuffer->nHeight * 3 * sizeof(char));
            //缓 冲 区 初 始 化
        }
        DX_BAYER_CONVERT_TYPE cvtype = RAW2RGB_NEIGHBOUR3; //选 择 插 值 算 法
        DX_PIXEL_COLOR_FILTER nBayerType = DX_PIXEL_COLOR_FILTER(BAYERBG);
        //选 择 图 像 Bayer 格 式
        bool bFlip = false;

        VxInt32 DxStatus = DxRaw8toRGB24(pFrameBuffer->pImgBuf, pRGB24Buf, pFrameBuffer->nWidth, pFrameBuffer->nHeight, cvtype, nBayerType, bFlip);
        if (DxStatus != DX_OK)
        {
            fmt::print(fmt::fg(fmt::color::red), "[CAMERA] Raw8 to RGB24 failed!\n");
            if (pRGB24Buf != NULL)
            {
                delete[] pRGB24Buf;
                pRGB24Buf = NULL;
            }
            return false;
        }

        // if (set_contrast)
        // {
        //     DxStatus = DxContrast(pRGB24Buf, pRGB24Buf,pFrameBuffer->nWidth * pFrameBuffer->nHeight * 3, contrast_factor);
        //     if (DxStatus != DX_OK)
        //         cout << "Contrast Set Failed" <<endl;
        // }
        if (set_color)
        {
            DxStatus = DxImageImprovment(pRGB24Buf, pRGB24Buf,pFrameBuffer->nWidth, pFrameBuffer->nHeight, nColorCorrectionParam,NULL,pGammaLut);
            if (DxStatus != DX_OK)
                fmt::print(fmt::fg(fmt::color::red), "[CAMERA] Color Set Failed!\n");
        }
        // if (set_saturation)
        // {
        //     DxStatus = DxSaturation(pRGB24Buf, pRGB24Buf,pFrameBuffer->nWidth * pFrameBuffer->nHeight * 3, saturation_factor);
        //     if (DxStatus != DX_OK)
        //         cout << "Saturation Set Failed" <<endl;
        // }

        Mat src = Mat(pFrameBuffer->nHeight, pFrameBuffer->nWidth, CV_8UC3);
        memcpy(src.data, pRGB24Buf, pFrameBuffer->nWidth * pFrameBuffer->nHeight * 3);
        src.copyTo(Src);

        delete[] pRGB24Buf;
        pRGB24Buf = NULL;
        // //调 用 GXQBuf 将 图 像 buf 放 回 库 中 继 续 采 图
        status = GXQBuf(hDevice, pFrameBuffer);
        return true;
    }
    else
    {
        // cout << "读取图片缓冲失败" << endl;
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] GetMat:读取图片缓冲失败\n");
        status = GXQBuf(hDevice, pFrameBuffer);
        return false;
    }
}

/**
 * @brief DaHengCamera::SetResolution   设置分辨率
 * @param width_scale   宽比例
 * @param height_scale  高比例
 * @return bool 返回是否成功
 */
bool DaHengCamera::SetResolution(int width_scale, int height_scale)
{
    //配 置 一 个 2x2 的 Binning 和 2x2 的 Decimation
    GX_STATUS status = GX_STATUS_SUCCESS;
    int64_t nBinningH = width_scale;
    int64_t nBinningV = height_scale;
    int64_t nDecimationH = width_scale;
    int64_t nDecimationV = height_scale;

    //设 置 水 平 和 垂 直 Binning 模 式 为 Sum 模 式
    status = GXSetEnum(hDevice, GX_ENUM_BINNING_HORIZONTAL_MODE,
                       GX_BINNING_HORIZONTAL_MODE_AVERAGE);
    status = GXSetEnum(hDevice, GX_ENUM_BINNING_VERTICAL_MODE,
                       GX_BINNING_VERTICAL_MODE_AVERAGE);
    status = GXSetInt(hDevice, GX_INT_BINNING_HORIZONTAL, nBinningH);
    status = GXSetInt(hDevice, GX_INT_BINNING_VERTICAL, nBinningV);
    status = GXSetInt(hDevice, GX_INT_DECIMATION_HORIZONTAL, nDecimationH);
    status = GXSetInt(hDevice, GX_INT_DECIMATION_VERTICAL, nDecimationV);
    if (status == GX_STATUS_SUCCESS)
    {
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] 分辨率设置成功\n");
        return true;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 分辨率设置失败\n");
        return false;
    }
}

/**
 * @brief DaHengCamera::SetExposureTime 设置曝光值
 * @param ExposureTime  具体曝光值
 * @return bool 返回是否设置成功
 */
bool DaHengCamera::SetExposureTime(int ExposureTime)
{
    //设 置  曝 光 值
    status = GXSetFloat(hDevice, GX_FLOAT_EXPOSURE_TIME, ExposureTime);
    if (status == GX_STATUS_SUCCESS)
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

/**
 * @brief DaHengCamera::SetGAIN 手动设置曝光增益
 * @param value 选择曝光增益通道 0-B,1-G,2-R,3-All
 * @param ExpGain   具体增益值 范围0-16
 * @return
 */
bool DaHengCamera::SetGAIN(int value, int ExpGain)
{
    if (value == 0)
    {
        //选 择 增 益 通 道 类 型
        status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR, GX_GAIN_SELECTOR_BLUE);
    }
    else if (value == 1)
    {
        status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR, GX_GAIN_SELECTOR_GREEN);
    }
    else if (value == 2)
    {
        status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR, GX_GAIN_SELECTOR_RED);
    }
    else
    {
        status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR, GX_GAIN_SELECTOR_ALL);
    }
    //设 置  曝 光 值
    status = GXSetFloat(hDevice, GX_FLOAT_GAIN, ExpGain);
    if (status == GX_STATUS_SUCCESS)
        return true;
    else
        return false;
}

/**
 * @brief DaHengCamera::Set_BALANCE_AUTO 枚举变量为0是表示关闭，1为开启，具体请查询SDK手册,具有记忆功能
 * @return bool 返回是否设置成功
 */
bool DaHengCamera::Set_BALANCE_AUTO(int value)
{
    //设 置 连 续 自 动 白 平 衡
    status = GXSetEnum(hDevice, GX_ENUM_BALANCE_WHITE_AUTO, value);
    if (status == GX_STATUS_SUCCESS)
    {
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] 自动白平衡设置成功\n");
        return true;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 自动白平衡设置失败\n");
        return false;
    }
}

/**
 * @brief DaHengCamera::Color_Correct 0是表示关闭，1为开启，具体请查询SDK手册,bu具有记忆功能
 * @return bool 返回是否设置成功
 */
bool DaHengCamera::Color_Correct(bool value)
{
    if(value)
    {
        //为 图 像 色 彩 调 节 数 组 申 请 空 间
        VxInt16*parrCC = new VxInt16[(sizeof(VxInt16))*9];
        if (parrCC== NULL)
        {
            // cout<<"Color Failed"<<endl;
            return false;
        }
        //获 取 颜 色 校 正 系 数
        status = GXGetInt (hDevice, GX_INT_COLOR_CORRECTION_PARAM,&nColorCorrectionParam);
        //计 算 图 像 彩 色 调 节 数 组
        status = DxCalcCCParam(nColorCorrectionParam, nSaturation, parrCC,(sizeof(VxInt16))*9);
        if (status != DX_OK)
        {
            if (parrCC!= NULL)
            {
                delete []parrCC;
                parrCC= NULL;
            };
            // cout<<"Color Failed"<<endl;
            return false;
        }
        set_color = value;
        return true;
    }
    // cout<<"Color Failed"<<endl;
    return false;
}

/**
 * @brief DaHengCamera::Set_BALANCE 手动白平衡,设置之前必须先关闭自动白平衡,具有记忆功能
 * @param value 选择平衡通道 0-B,1-G,2-R
 * @param value_number 平衡系数
 * @return
 */
bool DaHengCamera::Set_BALANCE(int value, float value_number)
{
    status = GXSetEnum(hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF);
    if (value == 0)
    {
        //选 择 白 平 衡 通 道
        status = GXSetEnum(hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_BLUE);
    }
    else if (value == 1)
    {
        status = GXSetEnum(hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN);
    }
    else
    {
        status = GXSetEnum(hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_RED);
    }
    status = GXSetFloat(hDevice, GX_FLOAT_BALANCE_RATIO, (float)value_number);
    if (status == GX_STATUS_SUCCESS){
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] 白平衡 {} 设置成功\n",value);
        return true;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 白平衡 {} 设置失败\n",value);
        return false;
    }
}

/**
 * @brief DaHengCamera::Set_GAMMA 手动adjust gamma,具有记忆功能
 * @param value 选择平衡通道 0-B,1-G,2-R
 * @param value_number 平衡系数
 * @return
 */
bool DaHengCamera::Set_Gamma(bool set_status,double dGammaParam)
{
    set_contrast = set_status;
    if(set_status)
    {
        // //获 取 对 比 度 调 节 参 数
        // GX_STATUS GxStatus = GXGetInt (hDevice, GX_INT_CONTRAST_PARAM, &nContrastParam);
        // if (GxStatus != GX_STATUS_SUCCESS)
        // {
        //     return false;
        // };

        // //获 取 Gamma 调 节 参 数
        // GxStatus = GXGetFloat (hDevice,GX_FLOAT_GAMMA_PARAM, &dGammaParam);
        // if (GxStatus != GX_STATUS_SUCCESS)
        // {
        //     return false;
        // }
    
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] Gamma {} 设置成功\n",dGammaParam);

        do
        {
            //获 取 Gamma 查 找 表 的 长 度
            VxInt32 DxStatus= DxGetGammatLut(dGammaParam, NULL, &nLutLength);
            if (DxStatus != DX_OK)
            {
                break;
            }
            //为 Gamma 查 找 表 申 请 空 间
            pGammaLut = new int[nLutLength];
            if (pGammaLut== NULL)
            {
                DxStatus= DX_NOT_ENOUGH_SYSTEM_MEMORY;
                break;
            }
            //计 算 Gamma 查 找 表
            DxStatus = DxGetGammatLut(dGammaParam, pGammaLut, &nLutLength);
            if (DxStatus != DX_OK)
            {
                break;
            }
        }while (0);
        return true;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] NOT Seting Gamma Value!\n");
        return -1;
    }
}

/**
 * @brief DaHengCamera::Set_Contrast 手动adjust Contrast,具有记忆功能
 * @param value 选择平衡通道 0-B,1-G,2-R
 * @param value_number 平衡系数
 * @return
 */
bool DaHengCamera::Set_Contrast(bool set_status,int dContrastParam)
{
    set_contrast = set_status;
    if(set_status)
    {
        contrast_factor = dContrastParam;
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] Contrast {} 设置成功\n",dContrastParam);
        return true;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] Using Default Contrast Value!\n");
        return -1;
    }
}

/**
 * @brief DaHengCamera::Set_Contrast 手动adjust Contrast,具有记忆功能
 * @param value 选择平衡通道 0-B,1-G,2-R
 * @param value_number 平衡系数
 * @return
 */
bool DaHengCamera::Set_Saturation(bool set_status,int dSaturationParam)
{
    set_saturation = set_status;
    if(set_status)
    {
        saturation_factor = dSaturationParam;
        fmt::print(fmt::fg(fmt::color::green), "[CAMERA] Saturation {} 设置成功\n",saturation_factor);
        return true;
    }
    else
    {
        fmt::print(fmt::fg(fmt::color::red), "[CAMERA] Using Default Saturation Value!\n");
        return -1;
    }
}

/**
 * @brief DaHengCamera::Get_TIMESTAMP   得到时间戳锁存值
 *                  还有问题没解决，可能是不支持此功能
 * @return _time 单位ms
 */
int DaHengCamera::Get_TIMESTAMP()
{
    //更新频率为125000000Hz
    int _time = ((double)lastImgTimestamp / (1.25 * 1e6)) - timestamp_offset;
    return _time;
}

/**
 * @brief DaHengCamera::~DaHengCamera 析构函数关闭设备
 */
DaHengCamera::~DaHengCamera()
{
    //停 采
    status = GXStreamOff(hDevice);
    //关闭设备链接
    status = GXCloseDevice(hDevice);
    //释放库
    status = GXCloseLib();
    fmt::print(fmt::fg(fmt::color::red), "[CAMERA] 析构!\n");
}
