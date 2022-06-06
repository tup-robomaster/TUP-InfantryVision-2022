
#include <iostream>

#include <ctime>
#include <future>
#include <random>
#include <vector>

#include <ceres/ceres.h>
#include <Eigen/Core>
#include <matplotlibcpp.h>
#include <opencv2/opencv.hpp>
#include <yaml-cpp/yaml.h>


#include "../../debug.h"
#include "../../general/general.h"

namespace plt = matplotlibcpp;

using namespace std;
using namespace cv;
using namespace plt;

class BuffPredictor
{
private:
    struct CURVE_FITTING_COST
    {
        CURVE_FITTING_COST (double x, double y) : _x ( x ), _y ( y ) {}
        // 残差的计算
        template <typename T>
        bool operator() (
            const T* const params,     // 模型参数，有3维
            T* residual ) const     // 残差
        {
            residual[0] = T (_y) - params[0] * ceres::sin(params[1] * T (_x) + params[2]); // f(x) = a * sin(ω * t + θ)
            return true;
        }
        const double _x, _y;    // x,y数据
    };

    //目标信息
    struct TargetInfo
    {
        double speed;
        int dist;
        int timestamp;
    };

    struct PredictStatus
    {
        bool xyz_status[3];
    };

private:
    double params[4];
    std::deque<TargetInfo> history_info;                                    //目标队列
    const int max_timespan = 90000;                                         //最大时间跨度，大于该时间重置预测器(ms)
    const int max_cost = 40;                                                 //TODO:回归函数最大Cost
    const int max_v = 3;                                                  //设置最大速度,单位rad/s
    const int max_a = 3;                                                  //设置最大角加速度,单位rad/s^2
    const int history_deque_len = 80;                                       //队列长度   
    const int bullet_speed = 28;                                            //TODO:弹速可变
    const int delay = 200;                                                  //发弹延迟
    const int window_size = 2;                                              //滑动窗口大小

public:
    TargetInfo last_target;                                                  //最后目标
    int mode;                                                               //预测器模式，0为小符，1为大符
    bool is_params_confirmed;

    BuffPredictor();
    ~BuffPredictor();
    bool predict(double speed, int dist, int timestamp, double &result);
    double calcAimingAngleOffset(double params[4], double t0, double t1, int mode);
    double shiftWindowFilter(int start_idx);
};


