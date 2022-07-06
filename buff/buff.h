#include <future>
#include <vector>

#include <Eigen/Core>

#include "../debug.h"

#include "fan_tracker.h"
#include "./predictor/predictor.h"
#include "./detector/inference.h"
#include "../coordsolver/coordsolver.h"
#include "../serial/serialport.h"
# include "../general/general.h"

class Buff
{
public:
    Buff();
    ~Buff();

    bool run(TaskData &src,VisionData &data);       // 自瞄主函数
private:
    const string network_path = "../model/buff.xml";
    const string camera_param_path = "../params/coord_param.yaml";
    
    bool is_last_target_exists;
    int lost_cnt;
    int last_timestamp;
    double last_target_area;
    double last_bullet_speed;
    Point2i last_roi_center;
    Point2i roi_offset;
    Size2d input_size;
    std::vector<FanTracker> trackers;      //tracker
    const int max_lost_cnt = 4;//最大丢失目标帧数
    const int max_v = 4;       //最大旋转速度(rad/s)
    const int max_delta_t = 100; //使用同一预测器的最大时间间隔(ms)
    const double fan_length = 0.7; //大符臂长(R字中心至装甲板中心)
    const double no_crop_thres = 2e-3;      //禁用ROI裁剪的装甲板占图像面积最大面积比值

    Fan last_fan;

    BuffDetector detector;
    BuffPredictor predictor;
    CoordSolver coordsolver;

    bool chooseTarget(vector<Fan> &fans, Fan &target);
    Point2i cropImageByROI(Mat &img);
};