#include <iostream>

#include <future>
#include <vector>

#include <Eigen/Core>
#include <opencv2/opencv.hpp>

#include "../general/general.h"

using namespace cv;
using namespace std;

struct Armor
{
    int id;
    int color;
    int area;
    string key;
    Point2f apex2d[4];
    Point2f center2d;
    Eigen::Vector3d center3d_cam;
    Eigen::Vector3d center3d_world;
    Eigen::Vector3d euler;
    Eigen::Vector3d predict;
};

class ArmorTracker
{
public:
    Armor prev_armor;               //上一次装甲板
    Armor last_armor;            //本次装甲板
    bool is_initialized;
    int prev_timestamp;             //上次装甲板时间戳
    int last_timestamp;          //本次装甲板时间戳
    const int max_history_len = 4;  //历史信息队列最大长度
    double velocity;
    double radius;

    std::deque<Armor> history_info;//目标队列

    ArmorTracker(Armor src,int src_timestamp);
    bool update(Armor new_armor, int new_timestamp);
};