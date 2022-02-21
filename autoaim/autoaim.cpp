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
    predictor_param_loader.initParam(predict_param_path);
    coordsolver.loadParam(camera_param_path,"Coord");
    lost_cnt = 0;
    is_last_target_exist = false;
    // input_size = {640,384};
    input_size = {416,416};
}

Autoaim::~Autoaim()
{
}

Point2i Autoaim::cropImageByROI(Mat &img)
{
    // cout<<"lost:"<<lost_cnt<<endl;
    if (!is_last_target_exist)
    {
        //当丢失目标帧数过多或lost_cnt为初值
        if (lost_cnt > max_lost_cnt || lost_cnt == 0)
        {
            // last_roi_center = Point2i(0,0);
            return Point2i(0,0);
        }
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

/*
 * @brief 选择击打目标
 * @param  
*/
Armor Autoaim::chooseTarget(vector<Armor> &armors)
{
    //TODO:优化打击逻辑
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
    auto time_start=std::chrono::steady_clock::now();
    vector<Object> objects;
    vector<Armor> armors;
    auto input = src.img.clone();
    roi_offset = cropImageByROI(input);
    auto time_crop=std::chrono::steady_clock::now();
    if (!detector.detect(input, objects))
    {
#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
        lost_cnt++;
        is_last_target_exist = false;
        return false;
    }
    auto time_infer = std::chrono::steady_clock::now();
    for (auto object : objects)
    {
#ifdef DETECT_RED
        if (object.color != 1)
            continue;
#endif //DETECT_RED
#ifdef DETECT_BLUE
        if (object.color != 0)
            continue;
#endif //DETECT_BLUE
        Armor armor;
        armor.id = object.cls;
        armor.color = object.color;
        if (object.color == 0)
            armor.key = "B" + to_string(object.cls);
        if (object.color == 1)
            armor.key = "R" + to_string(object.cls);
        if (object.color == 2)
            armor.key = "N" + to_string(object.cls);
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
        for(int i = 0; i < 4; i++)
            line(src.img, armor.apex2d[i % 4], armor.apex2d[(i + 1) % 4], Scalar(0,255,0), 1);
#endif //SHOW_ALL_ARMOR

        auto center = coordsolver.pnp(armor.apex2d, SOLVEPNP_IPPE);

#ifdef USING_IMU
#endif //USING_IMU

        armor.center3d = center;
        armors.push_back(armor);
    }
#ifdef USING_PREDICT
    if (predictors_map.size() != 0)
    {
        //维护预测器Map，删除过久之前的装甲板
        for (auto iter = predictors_map.begin(); iter != predictors_map.end();)
        {
            //删除元素后迭代器会失效，需先行获取下一元素
            auto next = ++iter;
            iter--;
            if (src.timestamp - (*iter).second.last_target.timestamp >= max_delta_t)
            {
                predictors_map.erase(iter);
            }
            iter = next;
        }
    }


    std::list<std::future<void>> predict_tasks;
    //为装甲板分配或新建最佳预测器
    // cout<<"walking"<<endl;
    for (auto armor = armors.begin(); armor != armors.end(); ++armor)
    {
        auto predictors_with_same_key = predictors_map.count((*armor).key);
        //当不存在该类型装甲板预测器
        if (predictors_with_same_key == 0)
        {
            Predictor predictor = predictor_param_loader.generate();
            auto target_predictor = predictors_map.insert(make_pair((*armor).key, predictor));
            //创建异步预测任务
            predict_tasks.emplace_back(std::async(std::launch::async, [=]()
                                                                            {(*armor).predict = (*target_predictor).second.predict((*armor).center3d, src.timestamp);}));
                
        }
        //当存在一个该类型装甲板预测器
        else if (predictors_with_same_key == 1)
        {
            auto candidate = predictors_map.find((*armor).key);
            auto delta_t = src.timestamp - (*candidate).second.last_target.timestamp;
            auto delta_dist = ((*armor).center3d - (*candidate).second.last_target.xyz).norm();
            auto velocity = delta_dist / (delta_t * 10.f);
            if (velocity <= max_v)
            {
                //创建异步预测任务
                predict_tasks.emplace_back(std::async(std::launch::async, [=]()
                                                                            {(*armor).predict = (*candidate).second.predict((*armor).center3d, src.timestamp);}));
            }
            else
            {
                Predictor predictor = predictor_param_loader.generate();
                auto target_predictor = predictors_map.insert(make_pair((*armor).key, predictor));
                //创建异步预测任务
                predict_tasks.emplace_back(std::async(std::launch::async, [=]()
                                                                            {(*armor).predict = (*target_predictor).second.predict((*armor).center3d, src.timestamp);}));
            }
        }
        //当存在多个该类型装甲板预测器
        else
        {
            //1e9无实际意义，仅用于以非零初始化
            double min_v = 1e9;
            int min_delta_t = 1e9;
            bool is_first_loop = true;
            bool is_best_candidate_exist = false;
            std::multimap<string, Predictor>::iterator best_candidate;
            auto candiadates = predictors_map.equal_range((*armor).key);
            //遍历所有同Key预测器，匹配速度最小且更新时间最近的预测器
            for(auto iter = candiadates.first; iter != candiadates.second; ++iter)
            {
                auto delta_t = src.timestamp - (*iter).second.last_target.timestamp;
                auto delta_dist = ((*armor).center3d - (*iter).second.last_target.xyz).norm();
                auto velocity = delta_dist / (delta_t * 10.f);
                if (is_first_loop)
                {
                    if (velocity <= max_v)
                    {
                        min_delta_t = delta_t;
                        min_v = velocity;
                        best_candidate = iter;
                        is_best_candidate_exist = true;
                    }

                    is_first_loop = false;
                }
                else
                {
                    if (velocity <= max_v && delta_t <= min_delta_t)
                    {
                        min_delta_t = delta_t;
                        min_v = velocity;
                        best_candidate = iter;
                        is_best_candidate_exist = true;
                    }
                }
            }
            //
            if (is_best_candidate_exist)
            {
                predict_tasks.emplace_back(std::async(std::launch::async, [=]()
                                                                            {(*armor).predict = (*best_candidate).second.predict((*armor).center3d, src.timestamp);}));
            }
            else
            {
                Predictor predictor = predictor_param_loader.generate();
                auto target_predictor = predictors_map.insert(make_pair((*armor).key, predictor));
                //创建异步预测任务
                predict_tasks.emplace_back(std::async(std::launch::async, [=]()
                                                                            {(*armor).predict = (*target_predictor).second.predict((*armor).center3d, src.timestamp);}));
            }

        }
    }
    // cout<<"walked"<<endl;
    // 等待所有预测任务完成
    // cout<<"total"<<predict_tasks.size()<<endl;
    int f = 0;
    // waitKey(20);
    for (auto task = predict_tasks.begin(); task != predict_tasks.end(); ++task)
    {
        // cout<<"waiting"<<f++<<endl;
        (*task).wait(); 
    }
    auto target = chooseTarget(armors);
    auto predict_info = target.predict;
#endif //USING_PREDICT

#ifndef USING_PREDICT
    auto target = chooseTarget(armors);
    auto predict_info = target.center3d;
#endif //USING_PREDICT

    Point2f apex_sum;
    //装甲板中心
    for(auto apex : target.apex2d)
        apex_sum +=apex;
    last_roi_center = apex_sum / 4.f;
    lost_cnt = 0;
    is_last_target_exist = true;

#ifdef SHOW_PREDICT
    auto center2d_src = coordsolver.reproject(target.center3d);
    auto center2d_pred = coordsolver.reproject(predict_info);
    circle(src.img, center2d_src, 5, Scalar(0, 0, 255), 2);
    circle(src.img, center2d_pred, 2, Scalar(0, 255, 255), 2);
#endif //SHOW_PREDICT

#ifdef SHOW_AIM_CROSS
        line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), Scalar(0,255,0), 1);
        line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), Scalar(0,255,0), 1);
#endif //SHOW_FPS
    auto angle = coordsolver.getAngle(predict_info);
    auto time_predict = std::chrono::steady_clock::now();
    double dr_full_ms = std::chrono::duration<double,std::milli>(time_predict - time_start).count();
#ifdef SHOW_FPS
    putText(src.img, "FPS:" + to_string(int(1000 / dr_full_ms)), Point2i(20,20), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
#endif //SHOW_FPS

#ifdef SHOW_IMG
    imshow("dst",src.img);
    waitKey(1);
    
#endif //SHOW_IMG

#ifdef PRINT_LATENCY
    //降低输出频率，避免影响帧率
    if (src.timestamp % 10 == 0)
    {
        double dr_crop_ms = std::chrono::duration<double,std::milli>(time_crop - time_start).count();
        double dr_infer_ms = std::chrono::duration<double,std::milli>(time_infer - time_crop).count();
        double dr_predict_ms = std::chrono::duration<double,std::milli>(time_predict - time_infer).count();
        cout<<"-----------TIME------------"<<endl;
        cout<<"Crop: "<<dr_crop_ms<<"ms."<<endl;
        cout<<"Infer: "<<dr_infer_ms<<"ms."<<endl;
        cout<<"Predict: "<<dr_predict_ms<<"ms."<<endl;
        cout<<"Total: "<<dr_full_ms<<"ms."<<endl;
    }
#endif //PRINT_LATENCY

#ifdef PRINT_TARGET_INFO
    cout<<"-----------INFO------------"<<endl;
    cout<<"Yaw: "<<angle[0]<<endl;
    cout<<"Pitch: "<<angle[1]<<endl;
    cout<<"Dist: "<<target.center3d.norm()<<endl;
#endif //PRINT_TARGET_INFO

    data = {(float)angle[1], (float)angle[0], (float)target.center3d.norm(), 0, 1, 0, 1};
    return true;
}
