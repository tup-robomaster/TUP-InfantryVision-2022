#include <future>
#include <vector>

#include <Eigen/Core>

#include "armor_tracker.h"
#include "detector/inference.h"
#include "predictor/predictor.h"
#include "../coordsolver/coordsolver.h"
#include "../general/general.h"
#include "../serial/serialport.h"

const string network_path = "/home/tup/Desktop/TUP-InfantryVision-2022/model/0311_416.xml";
const string camera_param_path = "/home/tup/Desktop/TUP-InfantryVision-2022/params/coord_param.yaml";
const string predict_param_path = "/home/tup/Desktop/TUP-InfantryVision-2022/params/filter/filter_param.yaml";

enum SpinHeading {UNKNOWN, CLOCKWISE, COUNTER_CLOCKWISE};

struct Image
{
    Mat img;
    Eigen::Quaterniond imu;
    int timestamp;//单位：ms
};

class Autoaim
{
public:
    Autoaim();
    ~Autoaim();

    bool run(Image &src,VisionData &data);       // 自瞄主函数
private:
    bool is_last_target_exists;
    int lost_cnt;
    int last_timestamp;
    double last_target_area;
    Point2i last_roi_center;
    Eigen::Vector3d last_aiming_point;
    Point2i roi_offset;
    Size2d input_size;
    std::vector<ArmorTracker> trackers;
    std::multimap<string, ArmorTracker> trackers_map;      //预测器Map
    std::map<string,int> new_armors_cnt_map;    //装甲板计数map，记录新增装甲板数
    std::map<string,SpinHeading> spin_status_map;    //反小陀螺，记录该车小陀螺状态
    std::map<string,double> spin_score_map;     //反小陀螺，记录各装甲板小陀螺可能性分数，大于0为逆时针旋转，小于0为顺时针旋转

    const int max_lost_cnt = 5;                 //最大丢失目标帧数
    const int max_armors = 8;                   //视野中最多装甲板数
    const int max_v = 8;                        //两次预测间最大速度(m/s)
    const int max_delta_t = 100;                //使用同一预测器的最大时间间隔(ms)

    int anti_spin_judge_high_thres = 2e4;//大于该阈值认为该车已开启陀螺
    int anti_spin_judge_low_thres = 1e4;//小于该阈值认为该车已关闭陀螺
    int anti_spin_max_r_multiple = 2;

    const double no_crop_thres = 2e-3;      //禁用ROI裁剪的装甲板占图像面积最大面积比值

    const int hero_danger_zone = 400;       //英雄危险距离阈值，检测到有小于该距离的英雄直接开始攻击

    Armor last_armor;
    Detector detector;
    Predictor predictor_param_loader;
    Predictor predictor;
    CoordSolver coordsolver;

    bool updateSpinScore();
    string chooseTargetID(vector<Armor> &armors);
    Armor chooseTargetArmor(vector<Armor> armors);
    Point2i cropImageByROI(Mat &img);
};