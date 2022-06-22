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
    DaHeng.SetExposureTime(3500);
    // 设置1
    DaHeng.SetGAIN(3, 16);
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

#ifdef USING_HIK
    start_get_img:

    HaiKangCamera HaiKang;

    HaiKang.StartDevice(0);
    // 设置分辨率
    HaiKang.SetResolution(1440, 1080);
    //更新时间戳，设置时间戳偏移量
    HaiKang.UpdateTimestampOffset(time_start);
    // 开始采集帧
    HaiKang.SetStreamOn();
    // 设置曝光事件
    HaiKang.SetExposureTime(8000);
    // 设置1
    // HaiKang.SetGAIN(0, 16);
    // HaiKang.SetGAIN(1, 8);
    // HaiKang.SetGAIN(2, 8);
    HaiKang.SetGAIN(3, 16);
    // 是否启用自动白平衡7
    // HaiKang.Set_BALANCE_AUTO(0);
    // manual白平衡 BGR->012
    HaiKang.Set_BALANCE(0, 1690);
    HaiKang.Set_BALANCE(1, 1024);
    HaiKang.Set_BALANCE(2, 2022);
#endif //USING_HIK

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
    // VideoCapture cap("/home/tup/Desktop/TUP-InfantryVision-2022-buff/RH.avi");
    VideoCapture cap("/home/tup/sample.avi");
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
    auto writer = cv::VideoWriter(path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30.0, cv::Size(width, height));    // Avi format
    std::future<void> write_video;
    bool is_first_loop = true;
    #ifdef SAVE_LOG_ALL
        LOG(INFO) << "[SAVE_VIDEO] Save video to " << path;
    #endif //SAVE_LOG_ALL
#endif //SAVE_VIDEO

#ifdef RECORD_DATA
    /*============ video_writer ===========*/
    int frame_cnt = 0;
    char now[64];
    std::time_t tt;
    struct tm *ttime;
    int width = 1280;
    int height = 1024;
    tt = time(nullptr);
    ttime = localtime(&tt);
    strftime(now, 64, "%Y-%m-%d_%H_%M_%S", ttime);  // 以时间为名字
    std::string now_string(now);
    const std::string storage_location = "../record/" + now_string;
    //Create dir to store datas.
    mkdir(storage_location.c_str(), S_IRWXU);
    fmt::print(fmt::fg(fmt::color::green), "[RECORD] Created directory :{}\n",storage_location);
    string data_file_pth = storage_location + "/" + "data.txt";
    string video_file_pth = storage_location + "/" + "video.avi";
    ofstream data;
    data.open(data_file_pth, std::ofstream::app);
    auto writer = cv::VideoWriter(video_file_pth, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 60.0, cv::Size(width, height));    // Avi format
#endif //RECORD_DATA
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

#ifdef USING_HIK
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
#endif //USING_HIK

#ifdef USING_VIDEO
        cap >> src.img;
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
        if(frame_cnt % 10 == 0)
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

#ifndef DEBUG_WITHOUT_COM
        //获取下位机数据
        MCUData mcu_status;
        if (!receive_factory.consume(mcu_status, src.timestamp))
            continue;
        src.quat = mcu_status.quat;
        src.mode = mcu_status.mode;
        src.bullet_speed = mcu_status.bullet_speed;
#endif


        //用于辅助标注
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
        // dst.mode = mode;
#endif // DEBUG_WITHOUT_COM

#ifdef SAVE_TRANSMIT_LOG
    // cout<<mode<<"..."<<last_mode<<endl;
    if (mode != last_mode)
    {
        LOG(INFO)<<"[CONSUMER] Mode switched to "<< mode;
        fmt::print(fmt::fg(fmt::color::pale_violet_red), "[CONSUMER] Mode switched to {}\n", mode);
        last_mode = mode;
    }
#endif //SAVE_TRANSMIT_LOG
        //1:自瞄模式
        //2:前哨站顶部装甲板识别模式
        //3:小能量机关模式
        //4.大能量机关模式
        if (mode == 1 || mode == 2)
        {
            autoaim.run(dst, data);
            transmit_factory.produce(data);
        }
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
            usleep(5000);
            continue;
        }
        //数据读取不成功进行循环
        while (!serial.get_Mode())
            ;
        auto time_cap = std::chrono::steady_clock::now();
        auto timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        // cout<<"Quat: "<<serial.quat[0]<<" "<<serial.quat[1]<<" "<<serial.quat[2]<<" "<<serial.quat[3]<<" "<<endl;
        // Eigen::Quaterniond quat = {serial.quat[0],serial.quat[1],serial.quat[2],serial.quat[3]};
        //FIXME:注意此处mode设置
        int mode = serial.mode;
        float bullet_speed = serial.bullet_speed;
        
        // int mode = 2;
        Eigen::Quaterniond quat = {serial.quat[0],serial.quat[1],serial.quat[2],serial.quat[3]};
        Eigen::Vector3d acc = {serial.acc[0],serial.acc[1],serial.acc[2]};;
        Eigen::Vector3d gyro = {serial.gyro[0],serial.gyro[1],serial.gyro[2]};;
        MCUData mcu_status = {mode, acc, gyro, quat, bullet_speed, timestamp};
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
                fmt::print(fmt::fg(fmt::color::orange), "[SERIAL] Warning: You are not using Serial port\n");
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