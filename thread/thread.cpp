#include "thread.h"

/**
 * @brief 生产者线程
 * @param factory 工厂类
**/
bool producer(Factory<Image> &factory)
{
    DaHengCamera DaHeng;
    auto time_start = std::chrono::steady_clock::now();

    DaHeng.StartDevice(1);
    // 设置分辨率
    DaHeng.SetResolution(1,1);
    // 开始采集帧
    DaHeng.SetStreamOn();
    // 设置曝光事件
    DaHeng.SetExposureTime(10000);
    // 设置
    DaHeng.SetGAIN(3, 16);
    // 是否启用自动曝光
    // DaHeng.Set_BALANCE_AUTO(1);
    while(1)
    {
        Image src;
        Mat img;
        auto time_cap = std::chrono::steady_clock::now();

        DaHeng.GetMat(img);
        if (img.empty())
            continue;

        src.img = img;
        src.timestamp = (int)(std::chrono::duration<double,std::milli>(time_cap - time_start).count());
        
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
            sleep(0.01);
            continue;
        }
        serial.TransformData(transmit);
        serial.send();
        // cout<<"transmitting..."<<endl;
    }
    return true;
}

bool serialWatcher(SerialPort &serial)
{
    while(1)
    {
        //检测文件夹是否存在或串口需要初始化
        if (access(serial.device.path.c_str(),F_OK) == -1 || serial.need_init)
        {
            serial.need_init = true;
            serial.initSerialPort();
        }

    }
}