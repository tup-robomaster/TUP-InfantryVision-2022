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
    Eigen::MatrixXd mat_ic(4, 4);
    Eigen::MatrixXd mat_ci(4, 4);
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

    read_vector = config[param_name]["T_ic"].as<vector<float>>();
    initMatrix(mat_ic,read_vector);
    transform_ic = mat_ic;

    read_vector = config[param_name]["T_ci"].as<vector<float>>();
    initMatrix(mat_ci,read_vector);
    transform_ci = mat_ci;

    return true;
}

PnPInfo CoordSolver::pnp(Point2f apex[4], int method)
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
    Mat rmat;
    Mat tvec;
    Eigen::Matrix3d rmat_eigen;
    Eigen::Vector3d coord_world = {0, 0, 0};
    Eigen::Vector3d tvec_eigen;
    Eigen::Vector3d coord_camera;

    solvePnP(points_world, points_pic, intrinsic, dis_coeff, rvec, tvec, false, method);

    PnPInfo result;
    //Pc = R * Pw + T
    Rodrigues(rvec,rmat);
    cv2eigen(rmat, rmat_eigen);
    cv2eigen(tvec, tvec_eigen);
    //转换至相机坐标系(左手坐标系)
    result.coord = (rmat_eigen * coord_world) + tvec_eigen;
    result.euler = rotationMatrixToEulerAngles(rmat_eigen) * 180 / CV_PI;
    if (result.euler[0] <= 0)
        result.euler[0] += 360;
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
inline Eigen::Vector2d CoordSolver::calcYawPitch(Eigen::Vector3d &xyz)
{
    Eigen::Vector2d angle;

    //Yaw
    angle[0] = atan2(xyz[0], xyz[2]) * 180 / CV_PI;
    //Pitch
    angle[1] = atan2(xyz[1], sqrt(xyz[0] * xyz[0] + xyz[2] * xyz[2])) * 180 / CV_PI;

    return angle;
}

/**
 * @brief 相机坐标系至世界坐标系
 * @param point_camera 相机坐标系下坐标
 * @param rmat 由陀螺仪四元数解算出的旋转矩阵
 * @return 世界坐标系下坐标
 * **/
inline Eigen::Vector3d CoordSolver::camToWorld(Eigen::Vector3d &point_camera, Eigen::Matrix3d &rmat)
{
    //升高维度
    Eigen::Vector4d point_camera_tmp;
    Eigen::Vector4d point_imu_tmp;
    Eigen::Vector3d point_imu;
    Eigen::Vector3d point_world;

    point_camera_tmp << point_camera[0], point_camera[1], point_camera[2], 1;
    point_imu_tmp = transform_ic * point_camera_tmp;
    point_imu << point_imu_tmp[0], point_imu_tmp[1], point_imu_tmp[2];

    return rmat * point_imu;
}

/**
 * @brief 世界坐标系至相机坐标系
 * @param point_world 世界坐标系下坐标
 * @param rmat 由陀螺仪四元数解算出的旋转矩阵
 * @return 相机坐标系下坐标
 * **/
inline Eigen::Vector3d CoordSolver::worldToCam(Eigen::Vector3d &point_world, Eigen::Matrix3d &rmat)
{
    
    Eigen::Vector4d point_camera_tmp;
    Eigen::Vector4d point_imu_tmp;
    Eigen::Vector3d point_imu;
    Eigen::Vector3d point_camera;

    point_imu = rmat.reverse() * point_world;
    point_imu_tmp << point_imu[0], point_imu[1], point_imu[2], 1;
    point_camera_tmp = transform_ci * point_imu_tmp;
    point_camera << point_camera[0], point_camera[1], point_camera[2];

    return point_camera;
}

