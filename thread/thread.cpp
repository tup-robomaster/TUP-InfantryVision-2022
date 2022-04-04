#include "thread.h"

/**
 * @brief 生产者线程
 * @param factory 工厂类
**/
bool producer(Factory<Image> &factory, MessageFilter<Eigen::Quaterniond> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start)
{
    // sleep(111);
    start_get_img:
#ifdef USING_DAHENG
    DaHengCamera DaHeng;
    DaHeng.StartDevice(1);
    // 设置分辨率
    DaHeng.SetResolution(1,1);
    //更新时间戳，设置时间戳偏移量
    DaHeng.UpdateTimestampOffset(time_start);
    // 开始采集帧
    DaHeng.SetStreamOn();
    // 设置曝光事件
    DaHeng.SetExposureTime(6000);
    // 设置
    DaHeng.SetGAIN(3, 2);
    // 是否启用自动白平衡7
    // DaHeng.Set_BALANCE_AUTO(0);
    // manual白平衡 BGR->012
    DaHeng.Set_BALANCE(0,1.56);
    DaHeng.Set_BALANCE(1,1.0);
    DaHeng.Set_BALANCE(2,1.548);
    // // Gamma
    // DaHeng.Set_Gamma(1,1.0);
    // //Color
    DaHeng.Color_Correct(1);
    // //Contrast
    // DaHeng.Set_Contrast(1,10);
    // //Saturation
    // DaHeng.Set_Saturation(0,0);
#endif //USING_DAHENG

#ifdef USING_USB_CAMERA
    VideoCapture cap("/home/tup/Desktop/red.avi");
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
#endif //SAVE_VIDEO
    while(1)
    {
        Image src;

#ifdef USING_DAHENG
        auto DaHeng_stauts = DaHeng.GetMat(src.img);
        if (!DaHeng_stauts)
        {
            goto start_get_img;
            fmt::print(fmt::fg(fmt::color::red), "[CAMERA] GetMat false return\n");
        }
        src.timestamp = DaHeng.Get_TIMESTAMP();
#endif //USING_DAHENG

#ifdef USING_USB_CAMERA
        cap >> src.img;
        src.timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        waitKey(30);
#endif //USING_USB_CAMERA

        if (src.img.empty())
            continue;

#ifdef SAVE_VIDEO
        writer.write(img);
#endif //SAVE_VIDEO

#ifdef USING_IMU
        //循环获取四元数
        while (!receive_factory.consume(src.imu, src.timestamp));
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
bool consumer(Factory<Image> &autoaim_factory,Factory<VisionData> &transmit_factory)
{
    Autoaim autoaim;
    while(1)
    {
        Image dst;
        VisionData data;

        autoaim_factory.consume(dst);
        if (autoaim.run(dst,data))
        {
            transmit_factory.produce(data);
        }

    }
    return true;
}

bool dataTransmitter(SerialPort &serial,Factory<VisionData> &transmit_factory)
{
    while(1)
    {
        VisionData transmit;
        transmit_factory.consume(transmit);
        //若串口离线则跳过数据发送
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
bool dataReceiver(SerialPort &serial, MessageFilter<Eigen::Quaterniond> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start)
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
        
        serial.get_Mode();
        auto time_cap = std::chrono::steady_clock::now();
        auto timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        // cout<<"Quad: "<<serial.quat[0]<<" "<<serial.quat[1]<<" "<<serial.quat[2]<<" "<<serial.quat[3]<<" "<<endl;
        Eigen::Quaterniond quat = {serial.quat[0],serial.quat[1],serial.quat[2],serial.quat[3]};
        receive_factory.produce(quat, timestamp);
        // Eigen::Vector3d vec = quad.toRotationMatrix().eulerAngles(2,1,0);
        // cout<<"Euler : "<<vec[0] * 180.f / CV_PI<<" "<<vec[1] * 180.f / CV_PI<<" "<<vec[2] * 180.f / CV_PI<<endl;
        // cout<<"transmitting..."<<endl;
    }
    return true;
}

bool serialWatcher(SerialPort &serial)
{
    while(1)
    {
        sleep(0.1);
        //检测文件夹是否存在或串口需要初始化
        if (access(serial.device.path.c_str(),F_OK) == -1 || serial.need_init)
        {
            serial.need_init = true;
            serial.initSerialPort();
        }

    }
}
#endif //USING_IMU_C_BOARD


#ifdef USING_IMU_WIT
bool dataReceiver(IMUSerial &serial_imu, MessageFilter<Eigen::Quaterniond> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start)
{
    while(1)
    {
        //若串口离线则跳过数据发送
        if (serial_imu.need_init == true)
        {
            // cout<<"offline..."<<endl;
            sleep(5e-3);
            continue;
        }
        if (!serial_imu.readData())
            continue;
        auto time_cap = std::chrono::steady_clock::now();
        auto timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        if (!serial_imu.is_quat_initialized)
        {
            sleep(5e-3);
            continue;
        }
        Eigen::Quaterniond quat = serial_imu.quat;
        Eigen::Vector3d acc= serial_imu.acc;
        receive_factory.produce(quat, timestamp);
        Eigen::Vector3d vec = quat.toRotationMatrix().eulerAngles(2,1,0);
        // cout<<"Acc :"<<acc[0]<<" "<<acc[1]<<" "<<acc[2]<<endl;
        cout<<"Euler : "<<vec[0] * 180.f / CV_PI<<" "<<vec[1] * 180.f / CV_PI<<" "<<vec[2] * 180.f / CV_PI<<endl;
        sleep(5e-3);
        // cout<<"transmitting..."<<endl;
    }
    return true;
}

bool serialWatcher(SerialPort &serial, IMUSerial &serial_imu)
{
    while(1)
    {
        sleep(0.1);
        //检测文件夹是否存在或串口需要初始化
        if (access(serial.device.path.c_str(),F_OK) == -1 || serial.need_init)
        {
            serial.need_init = true;
            serial.initSerialPort();
        }
        if (access(serial_imu.device.path.c_str(),F_OK) == -1 || serial_imu.need_init)
        {
            serial_imu.need_init = true;
            serial_imu.initSerialPort();
        }

    }
}
#endif //USING_WIT_IMU

#ifndef USING_IMU
bool serialWatcher(SerialPort &serial)
{
    while(1)
    {
        sleep(0.1);
        //检测文件夹是否存在或串口需要初始化
        if (access(serial.device.path.c_str(),F_OK) == -1 || serial.need_init)
        {
            serial.need_init = true;
            serial.initSerialPort();
        }

    }
}
#endif //USING_IMU