#include <future>
#include <vector>

#include <Eigen/Core>

#include "fan_tracker.h"
#include "./detector/inference.h"
#include "../coordsolver/coordsolver.h"
#include "predictor/predictor.h"
#include "../serial/serialport.h"

const string network_path = "/home/tup/Desktop/Buff/model/buff.xml";
const string camera_param_path = "/home/tup/Desktop/Buff/params/coord_param.yaml";
// const string predict_param_path = "/home/tup/Desktop/TUP-InfantryVision-2022/params/filter/filter_param.yaml";

struct Image
{
    Mat img;
    Eigen::Quaterniond imu;
    int timestamp;//单位：ms
};

class Buff
{
public:
    Buff();
    ~Buff();

    bool run(Image &src,VisionData &data);       // 自瞄主函数
private:
    bool is_last_target_exists;
    int lost_cnt;
    int last_timestamp;
    double last_target_area;
    Point2i last_roi_center;
    std::vector<FanTracker> trackers;      //tracker
    Point2i roi_offset;
    Size2d input_size;
    std::vector<FanTracker> trackers;      //tracker
    const int max_lost_cnt = 4;//最大丢失目标帧数
    const int max_v = 4;       //最大旋转速度(rad/s)
    const int fan_length = 0.565 //大符臂长(R字中心至装甲板中心)
    const int max_delta_t = 4000; //使用同一预测器的最大时间间隔(ms)
    const double no_crop_thres = 2e-3;      //禁用ROI裁剪的装甲板占图像面积最大面积比值

    Detector detector;
    Predictor predictor;
    CoordSolver coordsolver;

    Fan chooseTargetArmor(vector<Armor> armors);
    Point2i cropImageByROI(Mat &img);
};