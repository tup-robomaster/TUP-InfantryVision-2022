#include "general.h"

/**
 * @brief 按行读取文件中内容
 * @return 以vector形式返回文件每行内容
 * **/
std::vector<string> readLines(string file_path)
{
    std::vector<string> lines;
    string line;
    ifstream file;
    file.open(file_path, ios::in);

    while(getline(file,line))
        lines.push_back(line);

    return lines;
}

/**
 * @brief 将软链接转化为实际指向路径
 * @return 实际位置的绝对路径
 * **/
std::string symbolicToReal(string path)
{
    char path_tmp[1024];
    auto len = readlink(path.c_str(), path_tmp, 1024);
    //若路径错误返回空字符串
    if (len == -1)
        return NULL;
    string path_relative(path_tmp,0,len);

    return relativeToFull(path_relative,path);
    
}

/**
 * @brief 将相对路径转化为实际路径
 * @param relative 相对路径
 * @param origin 相对路径起始位置
 * @return 绝对路径
 * **/
std::string relativeToFull(string relative,string origin)
{
    auto tree_relative = generatePathTree(relative);
    auto tree_origin = generatePathTree(origin);

    string result;

    //记数
    int relative_cnt = 0;
    for(auto dir : tree_relative)
    {
        if (dir == "..")
            relative_cnt++;
    }

    vector<string> tree_parent (tree_origin.begin(),tree_origin.end() - relative_cnt - 1);
    vector<string> tree_no_relative (tree_relative.begin() + relative_cnt,tree_relative.end());

    result = treeToPath(tree_parent) + treeToPath(tree_no_relative);

    return result;
    
}

/**
 * @brief 生成路径的树状结构
 * @param path 路径
 * @return 路径结构
 * **/
std::vector<string> generatePathTree(string path)
{
    vector<string> tree;

    //若为相对路径，在头部插入“/”便于处理
    if (path[0] == '.')
        path.insert(0,"/");

    //使用逗号表达式控制循环
    for(auto i = 0; i = path.find("/"), i != path.npos; path = path.substr(i + 1))
    {   
        if(i == 0)
            continue;
        auto sub = path.substr(0, i);
        tree.push_back(sub);
    }

    tree.push_back(path);

    return tree;
}

/**
 * @brief 由树状结构生成路径
 * @return 生成路径
 * **/
string treeToPath(std::vector<string> &tree)
{
    string result;
    for(auto node : tree)
    {
        result += "/" + node;
    }
    return result;
}

/**
 * @brief 获取输入目录上级目录
 * @return 生成路径
 * **/
string getParent(string path)
{
    int last_slash_idx = 0;
    string path_tmp = path;

    for(auto i = 0; i = path_tmp.find("/"), i != path_tmp.npos; path_tmp = path_tmp.substr(i + 1))
    {   
        last_slash_idx += i + 1;
    }
    
    if (last_slash_idx == 0)
        return path;
    else
        return path.substr(0,last_slash_idx - 1);
    
}

/**
 * @brief 海伦公式计算三角形面积
 * 
 * @param pts 三角形顶点
 * @return float 面积
 */
float calcTriangleArea(cv::Point2f pts[3])
{
    auto a = sqrt(pow((pts[0] - pts[1]).x, 2) + pow((pts[0] - pts[1]).y, 2));
    auto b = sqrt(pow((pts[1] - pts[2]).x, 2) + pow((pts[1] - pts[2]).y, 2));
    auto c = sqrt(pow((pts[2] - pts[0]).x, 2) + pow((pts[2] - pts[0]).y, 2));

    auto p = (a + b + c) / 2.f;

    return sqrt(p * (p - a) * (p - b) * (p - c));
}

/**
 * @brief 计算四边形面积
 * 
 * @param pts 四边形顶点
 * @return float 面积
 */
float calcTetragonArea(cv::Point2f pts[4])
{
    return calcTriangleArea(&pts[0]) + calcTriangleArea(&pts[1]);
}

/**
 * @brief 将旋转矩阵转化为欧拉角
 * @param R 旋转矩阵
 * @return 欧拉角
*/
Eigen::Vector3d rotationMatrixToEulerAngles(Eigen::Matrix3d &R)
{
    double sy = sqrt(R(0,0) * R(0,0) + R(1,0) * R(1,0));
    bool singular = sy < 1e-6;
    double x, y, z;
    if (!singular)
    {
        x = atan2( R(2,1), R(2,2));
        y = atan2(-R(2,0), sy);
        z = atan2( R(1,0), R(0,0));
    }
    else
    {
        x = atan2(-R(1,2), R(1,1));
        y = atan2(-R(2,0), sy);
        z = 0;
    }
    return {z, y, x};
}

/**
 * @brief 计算欧拉角间的差值
 * @param euler2    第一个欧拉角
 * @param euler1    第二个欧拉角
 * @return 欧拉角差值(euler2 - euler1)
*/
Eigen::Vector3d calcDeltaEuler(Eigen::Vector3d euler1, Eigen::Vector3d euler2)
{
    Eigen::Vector3d delta_euler;
    //将Roll表示范围由[-PI,PI]转换至[0，2PI]
    for (int i = 0; i < 3;i++)
    {
        if (euler2[i] <= 0)
            euler2[i] += CV_2PI;
        if (euler1[i] <= 0)
            euler1[i] += CV_2PI;
    }

    for (int i = 0; i < 3;i++)
    {
        if (euler2[i] > 0 && euler2[i] < (CV_PI / 2) && euler1[i] > (3 * CV_PI / 2))
            delta_euler[i] = CV_2PI + euler2[i] - euler1[i];
        else if (euler2[i] > (3 * CV_PI / 2) && euler1[i] > 0 && euler1[i] < (CV_PI / 2))
            delta_euler[i] = -CV_2PI + euler2[i] - euler1[i];
        else
            delta_euler[i] = euler2[i] - euler1[i];
    }

    return delta_euler;
}

/**
 * @brief 计算旋转矩阵
 * @param theta 欧拉角(顺序Roll-Yaw-Pitch)
 * @return 旋转矩阵
*/
Eigen::Matrix3d eulerToRotationMatrix(Eigen::Vector3d &theta)
{
    Eigen::Matrix3d R_x;
    Eigen::Matrix3d R_y;
    Eigen::Matrix3d R_z;
    // Calculate rotation about x axis
    R_x <<
        1,       0,              0,
        0,       cos(theta[2]),   -sin(theta[2]),
        0,       sin(theta[2]),   cos(theta[2]);
    // Calculate rotation about y axis
    R_y <<
        cos(theta[1]),    0,      sin(theta[1]),
        0,               1,      0,
        -sin(theta[1]),   0,      cos(theta[1]);
    // Calculate rotation about z axis
    R_z <<
        cos(theta[0]),    -sin(theta[0]),      0,
        sin(theta[0]),    cos(theta[0]),       0,
        0,               0,                  1;
    // Combined rotation matrix
    return R_z * R_y * R_x;
}

/**
 * @brief 将欧拉角转换为轴角
 * 
 * @param euler 欧拉角
 * @return Eigen::AngleAxis 
 */
Eigen::AngleAxisd eulerToAngleAxisd(Eigen::Vector3d euler)
{
    Eigen::AngleAxisd rotVec;
    Eigen::AngleAxisd roll_angle(Eigen::AngleAxisd(euler(0), Eigen::Vector3d::UnitZ()));
    Eigen::AngleAxisd yaw_angle(Eigen::AngleAxisd(euler(1), Eigen::Vector3d::UnitY()));
    Eigen::AngleAxisd pitch_angle(Eigen::AngleAxisd(euler(2), Eigen::Vector3d::UnitX()));

    rotVec = roll_angle * yaw_angle * pitch_angle;
    return rotVec;
}

/**
 * @brief 将角度限制在[-PI,PI]的范围内
 * @return 处理后的角度
*/
double rangedAngleRad(double &angle)
{
    if (fabs(angle) >= CV_PI)
    {
        angle -= (angle / fabs(angle)) * CV_2PI;
        angle = rangedAngleRad(angle);
    }
    return angle;
}