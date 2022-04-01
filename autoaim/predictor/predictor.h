
#include <iostream>

#include <ctime>
#include <future>
#include <random>
#include <vector>

#include <ceres/ceres.h>
#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include <yaml-cpp/yaml.h>


#include "debug.h"
#include "../../general/general.h"
#include "particle_filter.h"

using namespace std;
using namespace cv;

//目标信息
struct TargetInfo
{
    Eigen::Vector3d xyz;
    int dist;
    int timestamp;
};

struct PredictStatus
{
    bool xyz_status[3];
};

class Predictor
{
public:
    TargetInfo last_target;                                                  //最后目标

    Predictor();
    ~Predictor();
    bool initParam(Predictor &predictor_loader);
    bool initParam(string coord_path);
    Predictor generate();
    Eigen::Vector3d predict(Eigen::Vector3d xyz,  int timestamp);
    Eigen::Vector3d shiftWindowFilter(int start_idx);
    PredictStatus predict_pf_run(TargetInfo target, Vector3d &result, int time_estimated);
    PredictStatus predict_fitting_run(Vector3d &result, int time_estimated);
private:
    bool fitting_disabled;                                                  //当前是否禁用拟合


    ParticleFilter pf_x;                                                  //粒子滤波
    ParticleFilter pf_y;                                                  //粒子滤波
    ParticleFilter pf_z;                                                  //粒子滤波
    std::deque<TargetInfo> history_info;                                //目标队列

    const int max_timespan = 1000;                                       //最大时间跨度，大于该时间重置预测器(ms)
    const int max_cost = 1e-2;                                            //回归函数最大Cost
    const int max_v = 8;                                                //设置最大速度,单位m/s
    const int history_deque_len = 10;                                   //队列长度    
    const int bullet_speed = 30;                                        //TODO:弹速可变
    const int delay = 50;                                              //发弹延迟(ms)
    const int window_size = 3;                                         //滑动窗口大小

};

struct CURVE_FITTING_COST
{
    CURVE_FITTING_COST (double x, double y) : _x ( x ), _y ( y ) {}
    // 残差的计算
    template <typename T>
    bool operator() (
        const T* const params,     // 模型参数，有3维
        T* residual ) const     // 残差
    {
        residual[0] = T (_y) - 1e2 * params[0] - 1e4 * params[1] * ceres::cos(params[3] * T (_x)) - 1e4 * params[2] * ceres::sin(params[3] * T (_x)); // f(x) = a0 + a1 * cos(wx) + b1 * sin(wx) 
        return true;
    }
    const double _x, _y;    // x,y数据
};