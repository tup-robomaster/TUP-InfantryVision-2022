
# pragma once
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
#include "../../filter/particle_filter.h"
namespace plt = matplotlibcpp;

using namespace std;
using namespace cv;
using namespace plt;

const string pf_path = "../params/filter/filter_param.yaml";
class BuffPredictor
{
private:
    struct CURVE_FITTING_COST
    {
        CURVE_FITTING_COST (double x, double y) : _x ( x ), _y ( y ) {}
        // 残差的计算
        template <typename T>
        bool operator() (
            const T* params,     // 模型参数，有3维
            T* residual) const     // 残差
        {
            residual[0] = T (_y) - params[0] * ceres::sin(params[1] * T (_x) + params[2]) - params[3]; // f(x) = a * sin(ω * t + θ) + b
            return true;
        }
        const double _x, _y;    // x,y数据

    };
    struct CURVE_FITTING_COST_PHASE
    {
        CURVE_FITTING_COST_PHASE (double x, double y, double a, double omega, double dc) : _x (x), _y (y), _a(a), _omega(omega), _dc(dc){}
        // 残差的计算
        template <typename T>
        bool operator() (
        const T* phase,     // 模型参数，有1维
        T* residual) const     // 残差
        {
            residual[0] = T (_y) - T (_a) * ceres::sin(T(_omega) * T (_x) + phase[0]) - T(_dc); // f(x) = a * sin(ω * t + θ)
            return true;
        }
        const double _x, _y, _a, _omega, _dc;    // x,y数据
    };

    //目标信息
    struct TargetInfo
    {
        double speed;
        double dist;
        int timestamp;
    };

    struct PredictStatus
    {
        bool xyz_status[3];
    };

private:
    double params[4];
    double bullet_speed = 28;                                            
    std::deque<TargetInfo> history_info;                                    //目标队列
    const int max_timespan = 20000;                                         //最大时间跨度，大于该时间重置预测器(ms)
    const double max_rmse = 0.4;                                               //TODO:回归函数最大Cost
    const int max_v = 3;                                                  //设置最大速度,单位rad/s
    const int max_a = 8;                                                  //设置最大角加速度,单位rad/s^2
    const int history_deque_len_cos = 250;                                  //大符全部参数拟合队列长度
    const int history_deque_len_phase = 100;                                  //大符相位参数拟合队列长度
    const int history_deque_len_uniform = 100;                                  //小符转速求解队列长度
    const int delay_small = 175;                                                  //小符发弹延迟
    const int delay_big = 100;                                              //大符发弹延迟
    const int window_size = 2;                                              //滑动窗口大小

public:
    TargetInfo last_target;                                                  //最后目标
    ParticleFilter pf;
    ParticleFilter pf_param_loader;
    int mode;                                                               //预测器模式，0为小符，1为大符
    int last_mode;
    bool is_params_confirmed;

    BuffPredictor();
    ~BuffPredictor();
    bool predict(double speed, double dist, int timestamp, double &result);
    double calcAimingAngleOffset(double params[4], double t0, double t1, int mode);
    double shiftWindowFilter(int start_idx);
    bool setBulletSpeed(double speed);
    double evalRMSE(double params[4]);
    double evalMAPE(double params[4]);

};


