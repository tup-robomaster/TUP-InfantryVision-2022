#include "./detector/inference.h"
#include "../coordsolver/coordsolver.h"
#include "predictor/predictor.h"
#include "../serial/serialport.h"

const string network_path = "/home/tup/Desktop/TUP-Vision-Infantry-2022/model/nano.xml";
const string camera_param_path = "/home/tup/Desktop/TUP-Vision-Infantry-2022/params/camera.yaml";

struct Armor
{
    int id;
    int color;
    Point2f apex2d[4];
    Eigen::Vector3d center3d;
};

struct Image
{
    Mat img;
    int timestamp;//单位：ms
};

class Autoaim
{
public:
    Autoaim();
    ~Autoaim();

    bool run(Image &src,VisionData &data);       // 自瞄主函数
private:
    Detector detector;
    //TODO:预测器暂未完成
    Predictor predictor;
    CoordSolver coordsolver;

    Armor chooseTarget(vector<Armor> &armors);
};


