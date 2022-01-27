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

Point2i Autoaim::cropImageByROI(Mat &img)
{
    // cout<<"lost:"<<lost_cnt<<endl;
    if (lost_cnt > 2)
    {
        return Point2f(0,0);
    }
    //处理X越界
    if (last_roi_center.x <= input_size.width / 2)
        last_roi_center.x = input_size.width / 2;
    else if (last_roi_center.x > (img.size().width - input_size.width / 2))
        last_roi_center.x = img.size().width - input_size.width / 2;
    //处理Y越界
    if (last_roi_center.y <= input_size.height / 2)
        last_roi_center.y = input_size.height / 2;
    else if (last_roi_center.y > (img.size().height - input_size.height / 2))
        last_roi_center.y = img.size().height - input_size.height / 2;

    //左上角顶点
    auto offset = last_roi_center - Point2i(input_size.width / 2, input_size.height / 2);
    Rect roi_rect = Rect(offset, input_size);
    img(roi_rect).copyTo(img);

    return offset;
}

Autoaim::Autoaim()
{
    detector.initModel(network_path);
    coordsolver.loadParam(camera_param_path
                        ,"Coord");
    lost_cnt = 0;
    input_size = {640,384};
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
    auto input = src.img.clone();
    roi_offset = cropImageByROI(input);
    // cout<<roi_offset<<endl;
    if (!detector.detect(input, objects))
    {
#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
        lost_cnt++;
        return false;
    }
    auto t2=std::chrono::steady_clock::now();
    double dr_ms=std::chrono::duration<double,std::milli>(t2-t1).count();

    for (auto object : objects)
    {
        Armor armor;
        armor.id = object.cls;
        armor.color = object.color;
        memcpy(armor.apex2d, object.apex, 4 * sizeof(cv::Point2f));
        for(int i = 0; i < 4; i++)
        {
            armor.apex2d[i] += Point2f((float)roi_offset.x,(float)roi_offset.y);
        }

#ifdef SHOW_ALL_ARMOR
        if (object.color == 0)
            putText(src.img, "B" + to_string(object.cls),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        if (object.color == 1)
            putText(src.img, "R" + to_string(object.cls),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        if (object.color == 2)
            putText(src.img, "N" + to_string(object.cls),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        for(int i=0;i<4;i++)
            line(src.img, armor.apex2d[i % 4], armor.apex2d[(i + 1) % 4], Scalar(0,255,0), 1);
#endif //SHOW_ALL_ARMOR

        auto center = coordsolver.pnp(armor.apex2d, SOLVEPNP_IPPE);

#ifdef USING_IMU
#endif //USING_IMU

        armor.center3d = center;
        armors.push_back(armor);
    }

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
    auto predict_info = predictor.predict(target.center3d, src.timestamp);;
    Point2f apex_sum;
    for(auto apex : target.apex2d)
        apex_sum +=apex;
    last_roi_center = apex_sum / 4.f;
    lost_cnt = 0;

#ifdef SHOW_PREDICT
    auto center2d_src = coordsolver.reproject(target.center3d);
    auto center2d_pred = coordsolver.reproject(predict_info);
    circle(src.img, center2d_src, 5, Scalar(0, 0, 255), 2);
    circle(src.img, center2d_pred, 2, Scalar(0, 255, 255), 2);
#endif //SHOW_PREDICT

    auto angle = coordsolver.getAngle(predict_info);

#ifdef SHOW_FPS
    putText(src.img, "FPS:" + to_string(int(1000 / dr_ms)), Point2i(20,20), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
#endif //SHOW_FPS

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

    data = {(float)angle[1], (float)angle[0], (float)target.center3d.norm(), 0, 1, 0, 1};
    return true;
}
