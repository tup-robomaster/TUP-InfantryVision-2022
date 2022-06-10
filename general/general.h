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

#ifdef CAMERA_ID_73
    const string camera_name = "KE0200110073";
#else
    #ifdef CAMERA_ID_75
        const string camera_name = "KE0200110075";
    #else
        const string camera_name = "KE0200110076";
    #endif
#endif

enum TargetType {SMALL, BIG, BUFF};
/**
 * @brief 存储任务所需数据的结构体
 * 
 */
struct TaskData
{
    int mode;
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

float calcDistance(Point2f&, Point2f&);
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