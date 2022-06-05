#include <iostream>

#include <future>
#include <vector>

#include <Eigen/Core>
#include <opencv2/opencv.hpp>

#include "../general/general.h"

using namespace cv;
using namespace std;

struct Fan
{
    int id;
    int color;
    string key;
    Point2f apex2d[5];

    cv::Point2f centerR2d;
    cv::Point2f armor2d;
    Eigen::Vector3d centerR3d_cam;
    Eigen::Vector3d centerR3d_world;
    Eigen::Vector3d armor3d_cam;
    Eigen::Vector3d armor3d_world;
    Eigen::Vector3d euler;
};

// struct FanTracker
// {
//     Fan last_fan;           //上一次扇叶
//     bool is_last_fan_exists;//是否存在上一次扇叶
//     double rotate_speed;    //角速度
//     int last_timestamp;     //上次扇叶时间戳
// };

class FanTracker
{
public:
    Fan prev_fan;                           //上一次装甲板
    Fan last_fan;                           //本次装甲板
    bool is_prev_fan_exists;                //是否存在上一次扇叶
    bool is_initialized;                    //是否完成初始化
    double rotate_speed;                    //角速度
    const int max_history_len = 3;          //队列长度
    int prev_timestamp;                     //上次装甲板时间戳
    int last_timestamp;                     //本次装甲板时间戳

    std::deque<Fan> history_info;//目标队列

    FanTracker(Fan src,int src_timestamp);
    bool update(Fan new_fan, int new_timestamp);
};