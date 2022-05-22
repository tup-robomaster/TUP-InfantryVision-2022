#include "thread.h"

/**
 * @brief 生产者线程
 * @param factory 工厂类
**/
bool producer(Factory<TaskData> &factory, MessageFilter<MCUData> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start)
{
#ifdef USING_DAHENG
    start_get_img:
    DaHengCamera DaHeng;
    DaHeng.StartDevice(1);
    // 设置分辨率
    DaHeng.SetResolution(1,1);
    //更新时间戳，设置时间戳偏移量
    DaHeng.UpdateTimestampOffset(time_start);
    // 开始采集帧
    DaHeng.SetStreamOn();
    // 设置曝光事件
    DaHeng.SetExposureTime(8000);
    // 设置
    DaHeng.SetGAIN(3, 14);
    // 是否启用自动白平衡7
    // DaHeng.Set_BALANCE_AUTO(0);
    // manual白平衡 BGR->012
    DaHeng.Set_BALANCE(0,1.56);
    DaHeng.Set_BALANCE(1,1.0);
    DaHeng.Set_BALANCE(2,1.548);
    // // Gamma
    // DaHeng.Set_Gamma(1,1.0);
    // //Color
    // DaHeng.Color_Correct(1);
    // //Contrast
    // DaHeng.Set_Contrast(1,10);
    // //Saturation
    // DaHeng.Set_Saturation(0,0);
#endif //USING_DAHENG

#ifdef USING_USB_CAMERA
    VideoCapture cap(0);
    // VideoCapture cap("/home/tup/Desktop/TUP-InfantryVision-2022-buff/RH.avi");
    fmt::print(fmt::fg(fmt::color::green), "[CAMERA] Open USB Camera success\n");
    // auto time_start = std::chrono::steady_clock::now();
#endif //USING_USB_CAMERA

    fmt::print(fmt::fg(fmt::color::green), "[CAMERA] Set param finished\n");
#ifdef SAVE_VIDEO
    /*============ video_writer ===========*/
    const std::string &storage_location = "../data/";
    char now[64];
    std::time_t tt;
    struct tm *ttime;
    tt = time(nullptr);
    ttime = localtime(&tt);
    strftime(now, 64, "%Y-%m-%d_%H_%M_%S", ttime);  // 以时间为名字
    std::string now_string(now);
    std::string path(std::string(storage_location + now_string).append(".avi"));
    auto writer = cv::VideoWriter(path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 60.0, cv::Size(1280, 1024));    // Avi form
    LOG(INFO) << "Save video to " << path;
#endif //SAVE_VIDEO
    while(1)
    {
        TaskData src;
        auto time_cap = std::chrono::steady_clock::now();

#ifdef USING_DAHENG
        auto DaHeng_stauts = DaHeng.GetMat(src.img);
        if (!DaHeng_stauts)
        {
            goto start_get_img;
            fmt::print(fmt::fg(fmt::color::red), "[CAMERA] GetMat false return\n");
            LOG(ERROR) << "[CAMERA] GetMat false return";
        }
        src.timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        // src.timestamp = DaHeng.Get_TIMESTAMP();
#endif //USING_DAHENG

#ifdef USING_USB_CAMERA
        cap >> src.img;
        src.timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        // waitKey(33.3);
#endif //USING_USB_CAMERA

        if (src.img.empty())
        {
#ifdef SAVE_LOG_ALL
            LOG(ERROR) << "[CAMERA] Get empty image";
#endif //SAVE_LOG_ALL
            continue;
        }

#ifdef SAVE_VIDEO
        //TODO:异步读写加速
        // auto write_video = std::async(std::launch::async, [&](){writer.write(src.img);});
        writer.write(src.img);
#endif //SAVE_VIDEO

#ifdef USING_IMU
        //获取下位机数据
        MCUData mcu_status;
        if (!receive_factory.consume(mcu_status, src.timestamp - 4))
            continue;
        src.quat = mcu_status.quat;
        src.mode = mcu_status.mode;
        // cout<<delta_t<<endl;
#endif //USING_IMU

        //用于辅助标注
        // DaHeng.SetExposureTime(1000 + src.timestamp % 100 * 30);
        factory.produce(src);

    }
    return true;
}

/**
 * @brief 消费者线程
 * @param factory 工厂类
**/
bool consumer(Factory<TaskData> &task_factory,Factory<VisionData> &transmit_factory)
{
    Autoaim autoaim;
    Buff buff;
    auto mode = -1;
    auto last_mode = -1;
    while(1)
    {
        TaskData dst;
        VisionData data;
        task_factory.consume(dst);
        mode = dst.mode;
#ifdef SAVE_TRANSMIT_LOG
    // cout<<mode<<"..."<<last_mode<<endl;
    if (mode != last_mode)
    {
        LOG(INFO)<<"[TASK] Mode switched to "<< mode;
        last_mode = mode;
    }
#endif //SAVE_TRANSMIT_LOG

        if (mode == 0x01)
        {
            if (autoaim.run(dst, data))
            {
                transmit_factory.produce(data);
            }
        }
        else if (mode == 0x03 || mode == 0x04)
        {
            if (buff.run(dst, data))
            {
                transmit_factory.produce(data);
            }   
        }

    }
    return true;
}

/**
 * @brief 数据发送线程
 * 
 * @param serial SerialPort类
 * @param transmit_factory Factory类
 * @return true 
 * @return false 
 */
bool dataTransmitter(SerialPort &serial,Factory<VisionData> &transmit_factory)
{
    while(1)
    {
        VisionData transmit;
        transmit_factory.consume(transmit);
        //若串口离线则跳过数据发送
        //TODO:使用无串口的模式时会导致此线程死循环，浪费CPU性能
        if (serial.need_init == true)
        {
            // cout<<"offline..."<<endl;
            sleep(5e-3);
            continue;
        }
        serial.TransformData(transmit);
        serial.send();
        // cout<<"transmitting..."<<endl;
    }
    return true;
}

#ifdef USING_IMU_C_BOARD
/**
 * @brief 数据接收线程
 * 
 * @param serial 
 * @param receive_factory 
 * @param time_start 
 * @return true 
 * @return false 
 */
bool dataReceiver(SerialPort &serial, MessageFilter<MCUData> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start)
{ 
    while(1)
    {
        //若串口离线则跳过数据发送
        if (serial.need_init == true)
        {
            // cout<<"offline..."<<endl;
            sleep(5e-3);
            continue;
        }
        //数据读取不成功进行循环
#ifndef DEBUG_WITHOUT_COM
        while (!serial.get_Mode())
            ;
#endif //DEBUG_WITHOUT_COM
        auto time_cap = std::chrono::steady_clock::now();
        auto timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        // cout<<"Quat: "<<serial.quat[0]<<" "<<serial.quat[1]<<" "<<serial.quat[2]<<" "<<serial.quat[3]<<" "<<endl;
        // Eigen::Quaterniond quat = {serial.quat[0],serial.quat[1],serial.quat[2],serial.quat[3]};
        //FIXME:注意此处mode设置
        // int mode = serial.mode;
        int mode = 0x01;
        Eigen::Quaterniond quat = {serial.quat[0],serial.quat[1],serial.quat[2],serial.quat[3]};
        Eigen::Vector3d acc = {serial.acc[0],serial.acc[1],serial.acc[2]};;
        Eigen::Vector3d gyro = {serial.gyro[0],serial.gyro[1],serial.gyro[2]};;
        MCUData mcu_status = {mode, acc, gyro, quat, timestamp};
        receive_factory.produce(mcu_status, timestamp);
        // Eigen::Vector3d vec = quat.toRotationMatrix().eulerAngles(2,1,0);
        // cout<<"Euler : "<<vec[0] * 180.f / CV_PI<<" "<<vec[1] * 180.f / CV_PI<<" "<<vec[2] * 180.f / CV_PI<<endl;
        // cout<<"transmitting..."<<endl;
    }
    return true;
}
#endif //USING_IMU_C_BOARD

/**
 * @brief 串口监视线程
 * 
 * @param serial 
 * @return true 
 * @return false 
 */
bool serialWatcher(SerialPort &serial)
{
    int last = 0;
#ifdef DEBUG_WITHOUT_COM
    #ifdef SAVE_TRANSMIT_LOG
    LOG(WARNING)<<"[SERIAL] Warning: You are not using Serial port";
    #endif //SAVE_TRANSMIT_LOG
#endif // DEBUG_WITHOUT_COM

    while(1)
    {
        sleep(0.1);
        //检测文件夹是否存在或串口需要初始化
        if (access(serial.device.path.c_str(),F_OK) == -1 || serial.need_init)
        {
            serial.need_init = true;
#ifdef DEBUG_WITHOUT_COM
            int now = clock()/CLOCKS_PER_SEC;
            if (now - last > 10)
            {
                last = now;
                fmt::print(fmt::fg(fmt::color::green), "[SERIAL] Warning: You are not using Serial port\n");
            }
            serial.withoutSerialPort();
#else
            serial.initSerialPort();
#endif //DEBUG_WITHOUT_COM
        }
    }
}


// #ifdef USING_IMU_WIT
// bool dataReceiver(IMUSerial &serial_imu, MessageFilter<MCUData> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start)
// {
//     while(1)
//     {
//         //若串口离线则跳过数据发送
//         if (serial_imu.need_init == true)
//         {
//             // cout<<"offline..."<<endl;
//             continue;
//         }
//         if (!serial_imu.readData())
//         {
//             continue;
//         }
//         auto time_cap = std::chrono::steady_clock::now();
//         auto timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
//         if (!serial_imu.is_quat_initialized)
//         {
//             continue;
//         }
//         Eigen::Quaterniond quat = serial_imu.quat;
//         Eigen::Vector3d acc = serial_imu.acc;
//         Eigen::Vector3d gyro =serial_imu.gyro;
//         MCUData imu_status = {acc, gyro, quat, timestamp};

//         receive_factory.produce(imu_status, timestamp);
//         Eigen::Vector3d vec = quat.toRotationMatrix().eulerAngles(2,1,0);
//     }
//     return true;
// }

// bool serialWatcher(SerialPort &serial, IMUSerial &serial_imu)
// {
//     while(1)
//     {
//         sleep(0.1);
//         //检测文件夹是否存在或串口需要初始化
//         if (access(serial.device.path.c_str(),F_OK) == -1 || serial.need_init)
//         {
//             serial.need_init = true;
//             serial.initSerialPort();
//         }
//         if (access(serial_imu.device.path.c_str(),F_OK) == -1 || serial_imu.need_init)
//         {
//             serial_imu.need_init = true;
//             serial_imu.initSerialPort();
//         }

//     }
// }
// #endif //USING_WIT_IMU

// #ifndef USING_IMU
// bool serialWatcher(SerialPort &serial)
// {
//     while(1)
//     {
//         sleep(0.1);
//         //检测文件夹是否存在或串口需要初始化
//         if (access(serial.device.path.c_str(),F_OK) == -1 || serial.need_init)
//         {
//             serial.need_init = true;
//             serial.initSerialPort();
//         }

//     }
// }
// #endif //USING_IMU