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

class CoordSolver
{
public:
    CoordSolver();
    ~CoordSolver();

    bool loadParam(string coord_path,string param_name);
    Eigen::Vector3d pnp(Point2f apex[4],int method);
    Eigen::Vector3d staticCoordOffset(Eigen::Vector3d &xyz);
    Eigen::Vector2d staticAngleOffset(Eigen::Vector2d &angle);
    Eigen::Vector2d getAngle(Eigen::Vector3d &xyz);
    Eigen::Vector2d calcYawPitch(Eigen::Vector3d &xyz);

    cv::Point2f reproject(Eigen::Vector3d &xyz);
private:
    Mat intrinsic;
    Mat dis_coeff;
    Eigen::Vector3d xyz_offset;
    Eigen::Vector2d angle_offset;

    YAML::Node param_node;
};