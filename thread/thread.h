//TODO:增加线程管理线程

#include "../camera/DaHengCamera.h"
#include "../autoaim/autoaim.h"
#include "../serial/serialport.h"

#include <iterator>
#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <atomic>
#include <opencv2/opencv.hpp>
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

bool producer(Factory<cv::Mat> &factory);
bool consumer(Factory<cv::Mat> &autoaim_factory,Factory<VisionData> &transmit_factory);
bool serialWatcher(SerialPort &serial);
bool dataTransmitter(SerialPort &serial,Factory<VisionData> &transmit_factory);

// class ThreadManagement
// {
// public:

// private:

// };