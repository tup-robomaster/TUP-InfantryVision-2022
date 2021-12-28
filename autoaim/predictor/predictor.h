
#include <iostream>

#include <random>
#include <vector>
#include <ctime>

#include <yaml-cpp/yaml.h>
#include <Eigen/Core>
#include <opencv2/opencv.hpp>

#include "../../general/general.h"
#include "particle_filter.h"

using namespace std;
using namespace cv;

class Predictor
{
public:

    Predictor();
    ~Predictor();
    Eigen::Vector3d predict(Eigen::Vector3d xyz);
private:
    Vector3d last_xyz;                                                  //最后坐标
    Vector3d last_v_xyz;                                                //最后速度
    deque<std::chrono::_V2::steady_clock::time_point> last_time;       //最后坐标时间戳队列
    ParticleFilter pf;                                              //粒子滤波
};