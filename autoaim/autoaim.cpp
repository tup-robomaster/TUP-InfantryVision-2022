#include "autoaim.h"
#include "debug.h"

using namespace cv;
using namespace std;

Autoaim::Autoaim()
{
    detector.initModel(model);
}

Autoaim::~Autoaim()
{
}

bool Autoaim::run(Mat &src)
{    
    auto t1=std::chrono::steady_clock::now();
    vector<Object> objects;

    if (!detector.detect(src, objects))
        return false;

    auto t2=std::chrono::steady_clock::now();
    double dr_ms=std::chrono::duration<double,std::milli>(t2-t1).count();
    for (auto object : objects) 
    {
        if (object.color == 0)
            putText(src, "B" + to_string(object.cls),object.apex[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        if (object.color == 1)
            putText(src, "R" + to_string(object.cls),object.apex[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        if (object.color == 2)
            putText(src, "N" + to_string(object.cls),object.apex[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        for(int i=0;i<4;i++)
            line(src,object.apex[i % 4],object.apex[(i + 1) % 4],Scalar(0,255,0),1);
    }
    putText(src, "FPS:" + to_string(int(1000 / dr_ms)),Point2i(20,20),FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
    imshow("dst",src);
    waitKey(1);
}
int main()
{
    Autoaim autoaim;
    Mat src;
    VideoCapture cap(0);

    while(1)
    {
        cap >> src;
        autoaim.run(src);
    }


}
