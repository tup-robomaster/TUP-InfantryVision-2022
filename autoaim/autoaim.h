#include <future>
#include <vector>

#include <Eigen/Core>

#include "./detector/inference.h"
#include "../coordsolver/coordsolver.h"
#include "predictor/predictor.h"
#include "../serial/serialport.h"

const string network_path = "/home/tup/Desktop/TUP-Vision-Infantry-2022/model/yolox.xml";
const string camera_param_path = "/home/tup/Desktop/TUP-Vision-Infantry-2022/params/coord_param.yaml";
const string predict_param_path = "/home/tup/Desktop/TUP-Vision-Infantry-2022/params/filter/filter_param.yaml";

struct Image
{
    Mat img;
    Eigen::Quaternionf imu;
    int timestamp;//单位：ms
};

class Autoaim
{
public:
    Autoaim();
    ~Autoaim();

    bool run(Image &src,VisionData &data);       // 自瞄主函数
private:
    int lost_cnt;
    bool is_last_target_exist;
    std::multimap<string, Predictor> predictors_map;      //预测器Map
    Point2i roi_offset;
    Point2i last_roi_center;
    Size2d input_size;
    const int max_lost_cnt = 2;//最大丢失目标帧数
    const int max_v = 20;       //两次预测间最大速度(m/s)
    const int max_delta_t = 200; //使用同一预测器的最大时间间隔(ms)

    Detector detector;
    Predictor predictor_param_loader;
    CoordSolver coordsolver;
    Armor chooseTarget(vector<Armor> &armors);
    Point2i cropImageByROI(Mat &img);
};


