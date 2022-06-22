
#include "../camera/DaHengCamera.h"
#include "../hik_camera/include/HaiKangCamera.h"
#include "../autoaim/autoaim.h"
#include "../buff/buff.h"
#include "../serial/serialport.h"
#include "../serial/wt61pc.h"
#include "../debug.h"

#include <iterator>
#include <thread>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <iostream>
#include <atomic>
#include <opencv2/opencv.hpp>

// #include <Eigen/Core>

using namespace std;
using namespace cv;

struct MCUData
{
    int mode;
    Eigen::Vector3d acc;
    Eigen::Vector3d gyro;
    Eigen::Quaterniond quat;
    double bullet_speed;
    int timestamp;
};

template <typename T>
class Factory
{
private:
    std::deque<T> buffer;
    int buffer_size;
    mutex lock;

public:
    /**
     * @brief 工厂类初始化
     * @param size 队列长度
    **/
    Factory(int size)
    {
        buffer_size = size;
    };
    bool produce(T &product);
    bool consume(T &product);
};

template <typename T>
bool Factory<T>::produce(T &product)
{

    lock.lock();
    if (buffer.size() < buffer_size)
        buffer.push_back(product);
    else
    {
        buffer.pop_front();
        buffer.push_back(product);
    
    }
    lock.unlock();

    return true;
}

template <typename T>
bool Factory<T>::consume(T &product)
{
    while (1)
    {
        lock.lock();
        if (!buffer.empty())
            break;
        lock.unlock();
        usleep(1e3);
    }
    product = buffer.front();
    buffer.pop_front();
    lock.unlock();

    return true;
}
//-----------------------------------------------------------------
template <typename T>
class MessageFilter
{
private:
    struct Product
    {
        T message;
        int timestamp;
    };
    std::deque<Product> buffer;
    atomic_bool is_editing;
    mutex lock;
    int buffer_size;
public:
    /**
     * @brief 工厂类初始化
     * @param size 队列长度
    **/
    MessageFilter(int size)
    {
        buffer_size = size;
        is_editing = false;
    };
    bool produce(T &message, int timestamp);
    bool consume(T &message, int timestamp);
};

template <typename T>
bool MessageFilter<T>::produce(T &message, int timestamp)
{
    lock.lock();
    Product product = {message, timestamp};
    if (buffer.size() < buffer_size)
        buffer.push_back(product);
    else
    {
        buffer.pop_front();
        buffer.push_back(product);
    }
    lock.unlock();

    return true;
}

template <typename T>
bool MessageFilter<T>::consume(T &message, int timestamp)
{
    //队列为空时阻塞消费者
    while (1)
    {
        lock.lock();
        if (!buffer.empty())
            break;
        lock.unlock();
        usleep(1e3);
    }
    // int cnt = 0;
    // for (auto info : buffer)
    // {
        
    //     cout<<cnt++<<" : "<<info.timestamp<<endl;
    // }
    auto it = std::lower_bound(buffer.begin(), buffer.end(), timestamp, [](Product &prev, const int &timestamp)
                               { return prev.timestamp < timestamp; });
    if (it == buffer.end())
    {
        //时间戳时间差大于10ms则认为该帧不可用
        if (abs((buffer.back().timestamp - timestamp)) > 10)
        {
            buffer.pop_front();
            lock.unlock();
            return false;            
        }
        else
        {
            message = (buffer.back()).message;
            buffer.pop_front();
            lock.unlock();
            return true;
        }
    }
    else
    {
        it--;
        message = (*it).message;
        buffer.erase(it);
    }
    lock.unlock();
    // cout<<(*it).timestamp<<":"<<timestamp<<"|"<<buffer.size()<<endl;
    // cout<<"///////////////////////////////////"<<endl;
    return true;
}

bool producer(Factory<TaskData> &factory, MessageFilter<MCUData> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start);
bool consumer(Factory<TaskData> &task_factory, Factory<VisionData> &transmit_factory);
bool dataTransmitter(SerialPort &serial, Factory<VisionData> &transmit_factory);

#ifdef USING_IMU_C_BOARD
bool dataReceiver(SerialPort &serial, MessageFilter<MCUData> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start);
bool serialWatcher(SerialPort &serial);
#endif // USING_IMU_C_BOARD
#ifdef USING_IMU_WIT
bool dataReceiver(IMUSerial &serial_imu, MessageFilter<MCUData> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start);
bool serialWatcher(SerialPort &serial, IMUSerial &serial_imu);
#endif // USING_IMU_WIT
#ifndef USING_IMU
bool serialWatcher(SerialPort &serial);
#endif // USING_IMU