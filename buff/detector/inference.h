
#include "../../debug.h"

#include <iterator>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include <inference_engine.hpp>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>

#include "../../general/general.h"

using namespace std;
using namespace cv;
using namespace InferenceEngine;

struct BuffObject
{
    Point2f apex[5];
    cv::Rect_<float> rect;
    int cls;
    int color;
    float prob;
    std::vector<cv::Point2f> pts;
};

class BuffDetector
{

private:
    Core ie;
    CNNNetwork network;                // 网络
    ExecutableNetwork executable_network;       // 可执行网络
    InferRequest infer_request;      // 推理请求
    MemoryBlob::CPtr moutput;
    string input_name;
    string output_name;
    
    Eigen::Matrix<float,3,3> transfrom_matrix;

public:
    BuffDetector();
    ~BuffDetector();

    bool detect(Mat &src,vector<BuffObject>& objects);
    bool initModel(string path);

};
