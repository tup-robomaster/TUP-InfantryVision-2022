
#include <iterator>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include <inference_engine.hpp>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include <fftw3.h>

#include "../../general/general.h"

using namespace std;
using namespace cv;
using namespace InferenceEngine;

struct Object
{
    Point2f apex[4];
    cv::Rect_<float> rect;
    int cls;
    int color;
    int area;
    float prob;
    std::vector<cv::Point2f> pts;
};

struct GridAndStride
{
    int grid0;
    int grid1;
    int stride;
};

class Detector
{
public:
    Detector();
    ~Detector();

    bool detect(Mat &src,vector<Object>& objects);
    bool initModel(string path);
private:

    Core ie;
    CNNNetwork network;                // 网络
    ExecutableNetwork executable_network;       // 可执行网络
    InferRequest infer_request;      // 推理请求
    MemoryBlob::CPtr moutput;
    string input_name;
    string output_name;
    
    Eigen::Matrix<float,3,3> transfrom_matrix;
};
