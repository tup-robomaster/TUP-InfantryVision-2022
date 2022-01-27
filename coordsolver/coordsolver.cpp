#include "coordsolver.h"

CoordSolver::CoordSolver()
{
}

CoordSolver::~CoordSolver()
{   
}

bool CoordSolver::loadParam(string coord_path,string param_name)
{
    YAML::Node config = YAML::LoadFile(coord_path);

    Eigen::MatrixXd mat_intrinsic(3, 3);
    Eigen::MatrixXd mat_coeff(1, 5);
    Eigen::MatrixXd mat_xyz_offset(1,3);
    Eigen::MatrixXd mat_angle_offset(1,2);

    //初始化内参矩阵
    auto read_vector = config[param_name]["Intrinsic"].as<vector<float>>();
    initMatrix(mat_intrinsic,read_vector);
    eigen2cv(mat_intrinsic,intrinsic);

    //初始化畸变矩阵
    read_vector = config[param_name]["Coeff"].as<vector<float>>();
    initMatrix(mat_coeff,read_vector);
    eigen2cv(mat_coeff,dis_coeff);

    read_vector = config[param_name]["xyz_offset"].as<vector<float>>();
    initMatrix(mat_xyz_offset,read_vector);
    xyz_offset = mat_xyz_offset.transpose();

    read_vector = config[param_name]["angle_offset"].as<vector<float>>();
    initMatrix(mat_angle_offset,read_vector);
    angle_offset = mat_angle_offset.transpose();

    return true;
}

Eigen::Vector3d CoordSolver::pnp(Point2f apex[4], int method)
{
    std::vector<Point3d> points_world;
    std::vector<Point2f> points_pic(apex,apex + 4);
    std::vector<Point2f> vecotr_apex = {apex[0], apex[1], apex[2], apex[3]};

    RotatedRect points_pic_rrect = minAreaRect(vecotr_apex);
    auto apex_wh_ratio = max(points_pic_rrect.size.height, points_pic_rrect.size.width) / min(points_pic_rrect.size.height, points_pic_rrect.size.width);


    if(apex_wh_ratio > armor_type_wh_thres)
        points_world = {
            {-11.25,2.7,0},
            {-11.25,-2.7,0},
            {11.25,-2.7,0},
            {11.25,2.7,0}};
    else
        points_world = {
            {-6.6,2.7,0},
            {-6.6,-2.7,0},
            {6.6,-2.7,0},
            {6.6,2.7,0}};

    Mat rvec;
    Mat tvec;

    solvePnP(points_world, points_pic, intrinsic, dis_coeff, rvec, tvec, false, method);

    Eigen::Vector3d result;
    cv2eigen(tvec, result);
    return result;
}


Eigen::Vector2d CoordSolver::getAngle(Eigen::Vector3d &xyz)
{
    auto xyz_offseted = staticCoordOffset(xyz);
    auto angle = calcYawPitch(xyz_offseted);
    auto angle_offseted = staticAngleOffset(angle);
    return angle_offseted;

}


//TODO:加入陀螺仪旋转矩阵
cv::Point2f CoordSolver::reproject(Eigen::Vector3d &xyz)
{
    Eigen::Matrix3d mat_intrinsic;
    cv2eigen(intrinsic, mat_intrinsic);
    //(u,v,1)^T = (1/Z) * K * (X,Y,Z)^T
    auto result = (100.f / xyz[2]) * mat_intrinsic * (xyz/ 100.f);//解算前进行单位转换
    return cv::Point2f(result[0], result[1]);
}

inline Eigen::Vector3d CoordSolver::staticCoordOffset(Eigen::Vector3d &xyz)
{
    return xyz + xyz_offset;
}

inline Eigen::Vector2d CoordSolver::staticAngleOffset(Eigen::Vector2d &angle)
{
    return angle + angle_offset;
}

/**
 * @brief 计算目标Yaw,Pitch角度
 * @return Yaw与Pitch
*/
Eigen::Vector2d CoordSolver::calcYawPitch(Eigen::Vector3d &xyz)
{
    Eigen::Vector2d angle;

    //Yaw
    angle[0] = atan2(xyz[0], xyz[2]) * 180 / CV_PI;
    //Pitch
    angle[1] = atan2(xyz[1], sqrt(xyz[0] * xyz[0] + xyz[2] * xyz[2])) * 180 / CV_PI;

    return angle;
}

Eigen::Matrix3d eulerToRotationMatrix(Eigen::Vector3d &theta)
{
    Eigen::Matrix3d R_x;
    Eigen::Matrix3d R_y;
    Eigen::Matrix3d R_z;
    // Calculate rotation about x axis
    R_x <<
        1,       0,              0,
        0,       cos(theta[0]),   -sin(theta[0]),
        0,       sin(theta[0]),   cos(theta[0]);
    // Calculate rotation about y axis
    R_y <<
        cos(theta[1]),    0,      sin(theta[1]),
        0,               1,      0,
        -sin(theta[1]),   0,      cos(theta[1]);
    // Calculate rotation about z axis
    R_z <<
        cos(theta[2]),    -sin(theta[2]),      0,
        sin(theta[2]),    cos(theta[2]),       0,
        0,               0,                  1;
    // Combined rotation matrix
    return R_z * R_y * R_x;
}

