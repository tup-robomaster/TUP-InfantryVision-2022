#include "./detector/inference.h"

string model = "/home/tup/Desktop/TUP-Vision-Infantry-2022/model/nano.xml";

class Autoaim
{
public:
    Autoaim();
    ~Autoaim();

    bool run(Mat &src);       // 自瞄主函数
private:
    Detector detector;
};


