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
    Eigen::Vector3d centerR3d_cam;
    Eigen::Vector3d centerR3d_world;
    Eigen::Vector3d armor3d_cam;
    Eigen::Vector3d armor3d_world;
    Eigen::Vector3d euler;
};

struct FanTracker
{
    Fan last_fan;           //上一次扇叶
    bool is_last_fan_exists;//是否存在上一次扇叶
    double rotate_speed;    //角速度
    int cur_timestamp;     //上次扇叶时间戳
};