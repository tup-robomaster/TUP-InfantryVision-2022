
#include <iostream>

#include <random>
#include <vector>
#include <future>
#include <ctime>

#include <ceres/ceres.h>
#include <yaml-cpp/yaml.h>
#include <Eigen/Core>
#include <opencv2/opencv.hpp>

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

class Predictor
{
public:

    Predictor();
    ~Predictor();
    Eigen::Vector3d predict(Eigen::Vector3d xyz,  int timestamp);
private:
    Vector3d last_xyz;                                                  //最后坐标
    Vector3d last_v_xyz;                                                //最后速度
    ParticleFilter pf;                                                  //粒子滤波
    std::deque<TargetInfo> history_info;                                //目标队列

    const int max_timespan = 1000;                                       //最大时间跨度，大于该时间重置预测器
    const int max_d = 1000;      
    const int history_deque_len = 8;                                   //队列长度   
    const int bullet_speed = 30;                                        //TODO:弹速可变
    const int delay = 50;                                              //发弹延迟
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