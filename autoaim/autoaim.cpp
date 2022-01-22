#include "autoaim.h"
#include "debug.h"

using namespace cv;
using namespace std;

//使用海伦公式计算三角形面积
inline float calcTriangleArea(cv::Point2f pts[3])
{
    auto a = sqrt(pow((pts[0] - pts[1]).x, 2) + pow((pts[0] - pts[1]).y, 2));
    auto b = sqrt(pow((pts[1] - pts[2]).x, 2) + pow((pts[1] - pts[2]).y, 2));
    auto c = sqrt(pow((pts[2] - pts[0]).x, 2) + pow((pts[2] - pts[0]).y, 2));

    auto p = (a + b + c) / 2.f;

    return sqrt(p * (p - a) * (p - b) * (p - c));
}

inline float calcTetragonArea(cv::Point2f pts[4])
{
    return calcTriangleArea(&pts[0]) + calcTriangleArea(&pts[1]);
}

Autoaim::Autoaim()
{
    detector.initModel(network_path);
    coordsolver.loadParam(camera_param_path
                        ,"Coord");
}

Autoaim::~Autoaim()
{
}
/*
 * @brief 选择击打目标
 * @param  
*/
Armor Autoaim::chooseTarget(vector<Armor> &armors)
{
    //TODO:优化打击逻辑、根据颜色筛选装甲板
    float max_area = 0;
    int target_idx = 0;
    for(int i = 0; i < armors.size(); i++)
    {
        // cout<<armors.size()<<endl;
        auto area = calcTetragonArea(armors[i].apex2d);
        if (area >= max_area)
        {
            max_area = area;
            target_idx = i;
        }
    }
    return armors[target_idx];

}

bool Autoaim::run(Image &src,VisionData &data)
{    
    auto t1=std::chrono::steady_clock::now();
    vector<Object> objects;
    vector<Armor> armors;
    if (!detector.detect(src.img, objects))
        return false;
    auto t2=std::chrono::steady_clock::now();
    double dr_ms=std::chrono::duration<double,std::milli>(t2-t1).count();

    for (auto object : objects)
    {
        Armor armor;
        armor.id = object.cls;
        armor.color = object.color;
        memcpy(armor.apex2d, object.apex, 4 * sizeof(cv::Point2f));

#ifdef SHOW_ALL_ARMOR
        if (object.color == 0)
            putText(src.img, "B" + to_string(object.cls),object.apex[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        if (object.color == 1)
            putText(src.img, "R" + to_string(object.cls),object.apex[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        if (object.color == 2)
            putText(src.img, "N" + to_string(object.cls),object.apex[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        for(int i=0;i<4;i++)
            line(src.img, object.apex[i % 4], object.apex[(i + 1) % 4], Scalar(0,255,0), 1);
#endif //SHOW_ALL_ARMOR

        auto center = coordsolver.pnp(object.apex, SOLVEPNP_EPNP);

#ifdef USING_IMU
#endif //USING_IMU

        armor.center3d = center;
        armors.push_back(armor);
    }

#ifdef SHOW_FPS
    putText(src.img, "FPS:" + to_string(int(1000 / dr_ms)), Point2i(20,20), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
#endif //SHOW_FPS

#ifdef SHOW_AIM_CROSS
        line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), Scalar(0,255,0), 1);
        line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), Scalar(0,255,0), 1);
#endif //SHOW_FPS

    if (objects.size() == 0)
    {
#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
        return false;
    }
    
    auto target = chooseTarget(armors);
    auto predict_info = predictor.predict(target.center3d, src.timestamp);

#ifdef SHOW_PREDICT
    auto center2d_src = coordsolver.reproject(target.center3d);
    auto center2d_pred = coordsolver.reproject(predict_info);
    circle(src.img, center2d_src, 10, Scalar(0, 0, 255), 2);
    circle(src.img, center2d_pred, 5, Scalar(0, 255, 255), 2);
#endif //SHOW_PREDICT

    auto angle = coordsolver.getAngle(target.center3d);
    
#ifdef SHOW_IMG
    imshow("dst",src.img);
    waitKey(1);
#endif //SHOW_IMG

#ifdef PRINT_TARGET_INFO
    cout<<"-----------INFO------------"<<endl;
    cout<<"Yaw: "<<angle[0]<<endl;
    cout<<"Pitch: "<<angle[1]<<endl;
    cout<<"Dist: "<<target.center3d.norm()<<endl;
#endif //PRINT_TARGET_INFO

    data = {(float)angle[1], (float)angle[0], 0, 0, 1, 0, 1};
    return true;
}
