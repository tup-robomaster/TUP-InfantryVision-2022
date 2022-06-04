#include "thread.h"

/**
 * @brief 生产者线程
 * @param factory 工厂类
**/
bool producer(Factory<TaskData> &factory, MessageFilter<MCUData> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start)
{
    constexpr string config_name = "../params/config.yaml";
    constexpr string param_name = "param_name";

#ifdef USING_DAHENG
    YAML::Node config = YAML::LoadFile(config_name);
    if(config.IsNull())
    {
        throw openFileDefault();
    }

    start_get_img:
    
    // 初始化大恒相机参数
    DaHengCamera DaHeng;
    
    DaHeng.StartDevice(1);
    // 设置分辨率
    DaHeng.SetResolution(1,1);
    //更新时间戳，设置时间戳偏移量
    DaHeng.UpdateTimestampOffset(time_start);
    // 开始采集帧
    DaHeng.SetStreamOn();
    // 设置曝光事件
<<<<<<< HEAD
    DaHeng.SetExposureTime(config[param_name]["Exposure_time"].as<int>()); 
    // 设置
    DaHeng.SetGAIN(3, config[param_name]["Exposure_gain"].as<double>());
=======
    DaHeng.SetExposureTime(6000);
    // 设置1
    DaHeng.SetGAIN(3, 14);
>>>>>>> main
    // 是否启用自动白平衡7
    // DaHeng.Set_BALANCE_AUTO(0);
    // manual白平衡 BGR->012
    DaHeng.Set_BALANCE(0, config[param_name]["B_balance"].as<double>());
    DaHeng.Set_BALANCE(1, config[param_name]["G_balance"].as<double>());
    DaHeng.Set_BALANCE(2, config[param_name]["R_balance"].as<double>());
    // // Gamma
    // DaHeng.Set_Gamma(1,1.0);
    // //Color
    // DaHeng.Color_Correct(1);
    // //Contrast
    // DaHeng.Set_Contrast(1,10);
    // //Saturation
    // DaHeng.Set_Saturation(0,0);
#endif //USING_DAHENG

#ifdef USING_HAIKANG
    start_get_img:

    HaiKangCamera HaiKang;

    HaiKang.StartDevice(0);
    // 设置分辨率
    HaiKang.SetResolution(1,1);
    //更新时间戳，设置时间戳偏移量
    HaiKang.UpdateTimestampOffset(time_start);
    // 开始采集帧
    HaiKang.SetStreamOn();
    // 设置曝光事件
    HaiKang.SetExposureTime(8000);
    // 设置1
    HaiKang.SetGAIN(3, 5);
    // 是否启用自动白平衡7
    // HaiKang.Set_BALANCE_AUTO(0);
    // manual白平衡 BGR->012
    HaiKang.Set_BALANCE(0, 1700);
    HaiKang.Set_BALANCE(1, 1000);
    HaiKang.Set_BALANCE(2, 1950);
#endif

#ifdef USING_USB_CAMERA
    VideoCapture cap(0);
    // VideoCapture cap("/home/tup/Desktop/TUP-InfantryVision-2022-buff/RH.avi");
    fmt::print(fmt::fg(fmt::color::green), "[CAMERA] Open USB Camera success\n");
    #ifdef SAVE_LOG_ALL
        LOG(INFO) << "[CAMERA] Open USB Camera success";
    #endif //SAVE_LOG_ALL

    // auto time_start = std::chrono::steady_clock::now();
#endif //USING_USB_CAMERA


#ifdef USING_VIDEO
    sleep(6);//防止网络加载完成前视频开始播放
    VideoCapture cap("/home/tup/Desktop/TUP-InfantryVision-2022-buff/RH.avi");
    // VideoCapture cap("/home/tup/Desktop/TUP-InfantryVision-2022-buff/sample.mp4");
#endif //USING_VIDEO

    fmt::print(fmt::fg(fmt::color::green), "[CAMERA] Set param finished\n");
#ifdef SAVE_VIDEO
    /*============ video_writer ===========*/
    int frame_cnt = 0;
    const std::string &storage_location = "../data/";
    char now[64];
    std::time_t tt;
    struct tm *ttime;
    int width = 1280;
    int height = 1024;
    tt = time(nullptr);
    ttime = localtime(&tt);
    strftime(now, 64, "%Y-%m-%d_%H_%M_%S", ttime);  // 以时间为名字
    std::string now_string(now);
    std::string path(std::string(storage_location + now_string).append(".avi"));
    auto writer = cv::VideoWriter(path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30.0, cv::Size(width, height));    // Avi form
    std::future<void> write_video;
    bool is_first_loop = true;
    #ifdef SAVE_LOG_ALL
        LOG(INFO) << "[SAVE_VIDEO] Save video to " << path;
    #endif //SAVE_LOG_ALL
#endif //SAVE_VIDEO
    while(1)
    {
        TaskData src;
        auto time_cap = std::chrono::steady_clock::now();

#ifdef USING_DAHENG
        auto DaHeng_stauts = DaHeng.GetMat(src.img);
        if (!DaHeng_stauts)
        {
            fmt::print(fmt::fg(fmt::color::red), "[CAMERA] GetMat false return\n");

            #ifdef SAVE_LOG_ALL
                LOG(ERROR) << "[CAMERA] GetMat false return";
            #endif //SAVE_LOG_ALL

            goto start_get_img;
        }
        src.timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        // src.timestamp = DaHeng.Get_TIMESTAMP();
#endif //USING_DAHENG
        auto HaiKang_stauts = HaiKang.GetMat(src.img);
        if (!HaiKang_stauts)
        {
            fmt::print(fmt::fg(fmt::color::red), "[CAMERA] GetMat false return\n");

            #ifdef SAVE_LOG_ALL
                LOG(ERROR) << "[CAMERA] GetMat false return";
            #endif //SAVE_LOG_ALL

            goto start_get_img;
        }
        src.timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
#ifdef USING_HAIKANG

#endif

#ifdef USING_VIDEO
        cap >> src.img;
        auto time_cap = std::chrono::steady_clock::now();
        src.timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        // sleep(0.02);
        waitKey(33.3);
#endif //USING_VIDEO

#ifdef USING_USB_CAMERA
        cap >> src.img;
        src.timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
#endif //USING_USB_CAMERA

        if (src.img.empty())
        {
#ifdef SAVE_LOG_ALL
            LOG(ERROR) << "[CAMERA] Get empty image";
#endif //SAVE_LOG_ALL
            continue;
        }

#ifdef SAVE_VIDEO
        frame_cnt++;
        if(frame_cnt % 5 == 0)
        {
            frame_cnt = 0;
            //异步读写加速,避免阻塞生产者
            if (is_first_loop)
                is_first_loop = false;
            else
                write_video.wait();
            write_video = std::async(std::launch::async, [&, src](){writer.write(src.img);});
        }
#endif //SAVE_VIDEO

#ifdef USING_IMU
        //获取下位机数据
        MCUData mcu_status;
        if (!receive_factory.consume(mcu_status, src.timestamp))
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

#ifdef DEBUG_WITHOUT_COM
        mode = 1;
#endif //DEBUG_WITHOUT_COM

#ifdef SAVE_TRANSMIT_LOG
    // cout<<mode<<"..."<<last_mode<<endl;
    if (mode != last_mode)
    {
        LOG(INFO)<<"[CONSUMER] Mode switched to "<< mode;
        fmt::print(fmt::fg(fmt::color::pale_violet_red), "[CONSUMER] Mode switched to {}\n", mode);
        last_mode = mode;
    }
#endif //SAVE_TRANSMIT_LOG

        if (mode == 1)
        {
            autoaim.run(dst, data);
            transmit_factory.produce(data);
        }
        //进入能量机关打击模式，3为小能量机关，4为大能量机关
        else if (mode == 3 || mode == 4)
        {
            buff.run(dst, data);
            transmit_factory.produce(data);
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
            #ifndef DEBUG_WITHOUT_COM
                #ifdef SAVE_LOG_ALL
                    LOG(ERROR) << "[TRANSMITTER] Serial offline, trying to reconnect...";
                #endif //SAVE_LOG_ALL
            #endif //DEBUG_WITHOUT_COM
            usleep(5000);
            continue;
        }
<<<<<<< HEAD
<<<<<<< HEAD

#ifdef DEBUG_WITHOUT_COM
        //发送数据格式转换
=======
        
>>>>>>> main
=======
>>>>>>> main
        serial.TransformData(transmit);
        try
        {
            serial.send();
        }
        catch(BaseException& e)
        {
            e.what();
        }
        // cout<<"transmitting..."<<endl;
#endif

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
            usleep(5000);
            continue;
        }
        //数据读取不成功进行循环 TODO:无串口调试模式应该不开启此线程
#ifndef DEBUG_WITHOUT_COM
        while (!serial.get_Mode())
            ;
#endif //DEBUG_WITHOUT_COM
        auto time_cap = std::chrono::steady_clock::now();
        auto timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        // cout<<"Quat: "<<serial.quat[0]<<" "<<serial.quat[1]<<" "<<serial.quat[2]<<" "<<serial.quat[3]<<" "<<endl;
        // Eigen::Quaterniond quat = {serial.quat[0],serial.quat[1],serial.quat[2],serial.quat[3]};
        //FIXME:注意此处mode设置
        int mode = serial.mode;
        // int mode = 1;
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
        sleep(1);
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
            try
            {
                serial.initSerialPort();
                
            }
            catch(BaseException& e)
            {
                e.what();
            }
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