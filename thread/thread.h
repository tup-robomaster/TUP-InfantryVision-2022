
#include "../camera/DaHengCamera.h"
#include "../autoaim/autoaim.h"
#include "../serial/serialport.h"
#include "../serial/wt61pc.h"
#include "../debug.h"

#include <iterator>
#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <atomic>
#include <opencv2/opencv.hpp>
#include <fmt/format.h>
#include <fmt/color.h>
// #include <Eigen/Core>

using namespace std;
using namespace cv;

template<typename T>
class Factory
{
private:
    std::deque<T> buffer;
    atomic_bool is_editing;
    int buffer_size;
public:
    /**
     * @brief 工厂类初始化
     * @param size 队列长度
    **/
    Factory(int size)
    {
        buffer_size = size;
        is_editing = false;
    };
    bool produce(T &product);
    bool consume(T &product);
};

template<typename T>
bool Factory<T>::produce(T &product)
{
    //当有其他线程访问时阻塞
    while(is_editing)
        sleep(0.0001);
    is_editing = true;

    if (buffer.size() < buffer_size)
        buffer.push_back(product);
    else
    {
        buffer.pop_front();
        buffer.push_back(product);
    }

    is_editing = false;
    return true;
}

template<typename T>
bool Factory<T>::consume(T &product)
{
    //队列为空时阻塞消费者
    while (buffer.empty() || is_editing)
        sleep(0.0001);
    //当有其他线程访问时阻塞
    is_editing = true;

    product = buffer.front();
    buffer.pop_front();
    is_editing = false;
    return true;
}
//-----------------------------------------------------------------
template<typename T>
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

template<typename T>
bool MessageFilter<T>::produce(T &message, int timestamp)
{
    //当有其他线程访问时阻塞
    while(is_editing)
        sleep(0.0001);
    is_editing = true;

    Product product = {message,timestamp};
    if (buffer.size() < buffer_size)
        buffer.push_back(product);
    else
    {
        buffer.pop_front();
        buffer.push_back(product);
    }

    is_editing = false;
    return true;
}

template<typename T>
bool MessageFilter<T>::consume(T &message, int timestamp)
{
    //队列为空时阻塞消费者
    while (buffer.empty() || is_editing)
        sleep(0.0001);
    //当有其他线程访问时阻塞
    is_editing = true;

    auto it = std::lower_bound(buffer.begin(),buffer.end(),timestamp,[](Product &prev, const int &timestamp)
                                                            {return prev.timestamp < timestamp;});
    if (it == buffer.end())
    {
        if(timestamp - (*buffer.end()).timestamp > 100)
            return false;
        message = (*buffer.end()).message;
        buffer.pop_back();
    }
    else
    {
        message = (*it).message;
        buffer.erase(it);
    }
    is_editing = false;
    return true;
}

bool producer(Factory<Image> &factory, MessageFilter<Eigen::Quaterniond> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start);
bool consumer(Factory<Image> &autoaim_factory,Factory<VisionData> &transmit_factory);
bool dataTransmitter(SerialPort &serial,Factory<VisionData> &transmit_factory);

#ifdef USING_IMU_C_BOARD
bool dataReceiver(SerialPort &serial, MessageFilter<Eigen::Quaterniond> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start);
bool serialWatcher(SerialPort &serial);
#endif //USING_IMU_C_BOARD
#ifdef USING_IMU_WIT
bool dataReceiver(IMUSerial &serial_imu, MessageFilter<Eigen::Quaterniond> &receive_factory, std::chrono::_V2::steady_clock::time_point time_start);
bool serialWatcher(SerialPort &serial, IMUSerial &serial_imu);
#endif //USING_IMU_WIT
#ifndef USING_IMU
bool serialWatcher(SerialPort &serial);
#endif //USING_IMU