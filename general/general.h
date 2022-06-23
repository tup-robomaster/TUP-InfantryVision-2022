#pragma once

#include <unistd.h>

#include <iostream>
#include <fstream> 

#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <yaml-cpp/yaml.h>
#include <Eigen/Core>
#include <Eigen/Dense>

#include <fmt/format.h>
#include <fmt/color.h>

#include <glog/logging.h>

#include "../debug.h"

using namespace std;
using namespace cv;

// const string camera_name = "KE0200110076";  // 哨兵下云台
const string camera_name = "KE0200110075";     // 步兵（4号)
// const string camera_name = "KE0200110074";  // 步兵（5号）
// const string camera_name = "KE0200110073";  // 英雄
// const string camera_name = "MV_Sentry";     // 哨兵上云台
// const string camera_name = "00J90630561";     // 5号blancer步兵


enum TargetType {SMALL, BIG, BUFF};
/**
 * @brief 存储任务所需数据的结构体
 * 
 */
struct TaskData
{
    int mode;
    double bullet_speed;
    Mat img;
    Eigen::Quaterniond quat;
    int timestamp;//单位：ms
};

struct GridAndStride
{
    int grid0;
    int grid1;
    int stride;
};

template<typename T>
bool initMatrix(Eigen::MatrixXd &matrix,std::vector<T> &vector)
{
    int cnt = 0;
    for(int row = 0;row < matrix.rows();row++)
    {
        for(int col = 0;col < matrix.cols();col++)
        {
            matrix(row,col) = vector[cnt];
            cnt++;
        }
    }
    return true;
}

float calcTriangleArea(cv::Point2f pts[3]);
float calcTetragonArea(cv::Point2f pts[4]);
double rangedAngleRad(double &angle);

std::string symbolicToReal(string path);
std::string relativeToFull(string relative,string src);
string treeToPath(std::vector<string> &tree);
string getParent(string path);

std::vector<string> readLines(string file_path);
std::vector<string> generatePathTree(string path);

Eigen::Vector3d rotationMatrixToEulerAngles(Eigen::Matrix3d &R);

Eigen::Vector3d calcDeltaEuler(Eigen::Vector3d euler1, Eigen::Vector3d euler2);
Eigen::AngleAxisd eulerToAngleAxisd(Eigen::Vector3d euler);
Eigen::Matrix3d eulerToRotationMatrix(Eigen::Vector3d &theta);