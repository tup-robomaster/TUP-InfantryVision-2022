#include <iterator>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>
#include <yaml-cpp/yaml.h>

#include "../general/general.h"

using namespace std;
using namespace cv;

struct PnPInfo
{
    Eigen::Vector3d coord;
    Eigen::Vector3d euler;
};

class CoordSolver
{
public:
    CoordSolver();
    ~CoordSolver();

    bool loadParam(string coord_path,string param_name);
    PnPInfo pnp(Point2f apex[4], int method);
    Eigen::Vector3d staticCoordOffset(Eigen::Vector3d &xyz);
    Eigen::Vector3d camToWorld(Eigen::Vector3d &point_camera, Eigen::Matrix3d &rmat);
    Eigen::Vector3d worldToCam(Eigen::Vector3d &point_world, Eigen::Matrix3d &rmat);
    Eigen::Vector2d staticAngleOffset(Eigen::Vector2d &angle);
    Eigen::Vector2d getAngle(Eigen::Vector3d &xyz);
    Eigen::Vector2d calcYawPitch(Eigen::Vector3d &xyz);

    cv::Point2f reproject(Eigen::Vector3d &xyz);
private:
    Mat intrinsic;
    Mat dis_coeff;
    Eigen::Vector3d xyz_offset;
    Eigen::Vector2d angle_offset;
    Eigen::Matrix4d transform_ic;
    Eigen::Matrix4d transform_ci;

    YAML::Node param_node;

    const int armor_type_wh_thres = 7;      //大小装甲板长宽比阈值
};