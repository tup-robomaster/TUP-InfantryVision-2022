#include "autoaim.h"

using namespace cv;
using namespace std;

#ifdef ASSIST_LABEL
int cnt = 0;
const string path_prefix = "/home/tup/Desktop/TUP-Vision-Infantry-2022/data/";
ofstream file;
#endif //ASSIST_LABEL

Autoaim::Autoaim()
{
    detector.initModel(network_path);
    predictor_param_loader.initParam(predict_param_path);
    coordsolver.loadParam(camera_param_path,"KE0200110076");
    // cout<<"...."<<endl;
    lost_cnt = 0;
    is_last_target_exists = false;
    last_target_area = 0;
    input_size = {416,416};
    auto predictor_tmp = predictor_param_loader.generate();
    predictor.initParam(predictor_param_loader);
}

Autoaim::~Autoaim()
{
}

/**
 * @brief 根据上次装甲板位置截取ROI
 * 
 * @param img 所需处理的图像
 * @return ** Point2i ROI中心点
 */
#ifdef USING_ROI
Point2i Autoaim::cropImageByROI(Mat &img)
{
    // cout<<"lost:"<<lost_cnt<<endl;
    //若上次不存在目标
    if (!is_last_target_exists)
    {
        //当丢失目标帧数过多或lost_cnt为初值
        if (lost_cnt > max_lost_cnt || lost_cnt == 0)
        {
            return Point2i(0,0);
        }
    }
    //若目标大小大于阈值
    // cout<<last_target_area / img.size().area()<<endl;
    if ((last_target_area / img.size().area()) > no_crop_thres)
    {
        return Point2i(0,0);
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
#endif //USING_ROI

/**
 * @brief 更新陀螺Score，函数关系在MATLAB中测试得出，在程序帧率恒定100fps
 * 的假设下，该函数关系可以在转速为 5rad/s -- 15rad/s 的情况下，
 * 在10到12次装甲板切换后识别出陀螺状态，无切换约0.5s-1s后自动退出陀螺状态
 * 
 * @return true 更新分数成功
 */
bool Autoaim::updateSpinScore()
{    
    for (auto score = spin_score_map.begin(); score != spin_score_map.end();)
    {
        SpinHeading spin_status;
        //若Status_Map不存在该元素

        if (spin_score_map.count((*score).first) == 0)
            spin_status = UNKNOWN;
        else
            spin_status = spin_status_map[(*score).first];
        // cout<<(*score).first<<"--"<<(*score).second<<" "<<spin_status<<endl;

        // 若分数过低移除此元素
        if ((*score).second <= anti_spin_judge_low_thres && spin_status != UNKNOWN)
        {
            fmt::print(fmt::fg(fmt::color::red), "[SpinScore] Removing\n");
            spin_status_map.erase((*score).first);
            score = spin_score_map.erase(score);
            continue;
        }
        // 
        if (spin_status != UNKNOWN)
            (*score).second = 0.978 * (*score).second;
        else
            (*score).second = 0.998 * (*score).second - 1 * abs((*score).second) / (*score).second;
        //当小于该值时移除该元素
        if (abs((*score).second) < 3)
        {
            spin_status_map.erase((*score).first);
            score = spin_score_map.erase(score);
            continue;
        }
        else if (abs((*score).second) >= anti_spin_judge_high_thres)
        {
            (*score).second = anti_spin_judge_high_thres;
            if ((*score).second < 0)
                spin_status_map[(*score).first] = CLOCKWISE;
            else if((*score).second > 0)
                spin_status_map[(*score).first] = COUNTER_CLOCKWISE;
        }
        ++score;
    }
    // cout<<"++++++++++++++++++++++++++"<<endl;
    // for (auto status : spin_status_map)
    // {
    //     cout<<status.first<<" : "<<status.second<<endl;
    // }
    return true;
}

/**
 * @brief 拟合空间球体
 * 
 * @param pts  空间点
 * @param plane 空间平面 Z = AX + BY + C ,分别为A,B,C
 * @return Eigen::Vector4d （X - a) ^ 2 +（X - b) ^ 2 + (X - c) ^ 2 = R ^ 2 返回A,B,C,R
 */
Eigen::Vector4d leastSquareFitSphere(std::vector<Eigen::Vector3d> pts, Eigen::Vector3d plane)
{
    auto size = pts.size();
    Eigen::MatrixXd A(size,3);
    Eigen::VectorXd e(size);
    //初始化参数
    for (int i = 0; i < pts.size(); i++)
    {
        //Aa = -2x - 2Az
        A(i,0) = -2 * pts[i][0] - 2 * plane[0] * pts[i][2];
        //Ab = -2y - 2Bz
        A(i,1) = -2 * pts[i][1] - 2 * plane[1] * pts[i][2];
        //Ac = 1
        A(i,2) = 1;
        //e = 2Cz - (x^2 + y^2 + z^2)
        e[i] = 2 * plane[2] * pts[i][2] - pts[i].squaredNorm();
    }
    Eigen::Vector3d result;
    //求解最小二乘问题
    result = (A.transpose() * A).inverse() * A.transpose() * e;
    auto alpha = result[0];
    auto beta = result[1];
    auto gamma = plane[0] * result[0] + plane[1] * result[1] + plane[2];
    auto R = sqrt(result[2] - pow(alpha,2) - pow(beta,2) - pow(gamma,2));
    Eigen::Vector4d final_result = {alpha,beta,gamma,R};
    return final_result;
}

/**
 * @brief 最小二乘法拟合空间平面
 * 
 * @param pts 拟合所用点
 * @return Eigen::Vector3d Z = AX + BY + C，分别传回A,B,C
 */
Eigen::Vector3d leastSquareFitPlane(std::vector<Eigen::Vector3d> pts)
{
    auto size = pts.size();
    Eigen::MatrixXd A(size,3);
    Eigen::VectorXd e(size);
    //初始化参数
    for (int i = 0; i < pts.size(); i++)
    {
        A(i,0) = pts[i][0];
        A(i,1) = pts[i][1];
        A(i,2) = 1;
        e[i] = pts[i][2];
    }
    Eigen::Vector3d result;
    //求解最小二乘问题
    result = (A.transpose() * A).inverse() * A.transpose() * e;
    return result;
}

/**
 * @brief 
 * 
 * @param pts 拟合所用点
 * @return Eigen::Vector4d （X - a) ^ 2 +（X - b) ^ 2 + (X - c) ^ 2 = R ^ 2 返回A,B,C,R
 */
Eigen::Vector4d FitSpaceCircle(std::vector<Eigen::Vector3d> pts)
{
    //Z = AX + BY + C
    auto plane = leastSquareFitPlane(pts);
    //拟合空间球体，约束条件为圆心在平面上之前解出的平面上，进行最小二乘法
    return leastSquareFitSphere(pts,plane);
}

/**
 * @brief 从装甲板中选择最终目标
 * 
 * @param armors 装甲板vector
 * @return Armor 所选取的装甲板
 */
Armor Autoaim::chooseTargetArmor(vector<Armor> armors)
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

/**
 * @brief 选择击打车辆ID
 * 
 * @param armors 
 * @return string 
 */
string Autoaim::chooseTargetID(vector<Armor> &armors)
{
    for (auto armor : armors)
    {
        //若视野中存在英雄且距离小于危险距离，直接选为目标
        if (armor.id == 1 && armor.center3d_world.norm() <= hero_danger_zone)
        {
            return armor.key;
        }
    }
    //若不存在则返回面积最大的装甲板序号，即队列首元素序号
    return (*armors.begin()).key;
}


/**
 * @brief 自瞄主函数
 * 
 * @param src 图像、时间戳、IMU数据
 * @param data 发送数据所用结构体
 * @return true 识别成功
 * @return false 识别失败
 */
bool Autoaim::run(Image &src,VisionData &data)
{
    auto time_start=std::chrono::steady_clock::now();
    vector<Object> objects;
    vector<Armor> armors;
    auto input = src.img;

#ifdef USING_IMU
    Eigen::Matrix3d rmat_imu = src.imu.toRotationMatrix();
#else
    Eigen::Matrix3d rmat_imu = Eigen::Matrix3d::Identity();
#endif //USING_IMU

#ifdef USING_ROI
    roi_offset = cropImageByROI(input);
#endif  //USING_ROI

    auto time_crop=std::chrono::steady_clock::now();
    //若未检测到目标
    if (!detector.detect(input, objects))
    {
#ifdef SHOW_AIM_CROSS
        line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), {0,255,0}, 1);
        line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), {0,255,0}, 1);
#endif //SHOW_AIM_CROSS
#ifdef SHOW_IMG
        namedWindow("dst",0);
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
#ifdef USING_SPIN_DETECT
        updateSpinScore();
#endif //USING_SPIN_DETECT
    // if (src.timestamp % 10 == 0)
    // {
    //     auto time_infer = std::chrono::steady_clock::now();
    //     double dr_crop_ms = std::chrono::duration<double,std::milli>(time_crop - time_start).count();
    //     double dr_infer_ms = std::chrono::duration<double,std::milli>(time_infer - time_crop).count();
    //     fmt::print(fmt::fg(fmt::color::gray), "-----------TIME------------\n");
    //     fmt::print(fmt::fg(fmt::color::blue_violet), "Crop: {} ms\n"   ,dr_crop_ms);
    //     fmt::print(fmt::fg(fmt::color::golden_rod), "Infer: {} ms\n",dr_infer_ms);
    // }
        lost_cnt++;
        is_last_target_exists = false;
        last_target_area = 0;
        return false;
    }
#ifdef ASSIST_LABEL
    auto img_name = path_prefix + to_string(cnt) + ".png";
    imwrite(img_name,input);
#endif //ASSIST_LABEL
    auto time_infer = std::chrono::steady_clock::now();
    ///------------------------将对象排序，保留面积较大的对象---------------------------------
    sort(objects.begin(),objects.end(),[](Object& prev, Object& next)
                                    {return prev.area > next.area;});
    //若对象较多保留前按面积排序后的前max_armors个
    if (objects.size() > max_armors)
        objects.resize(max_armors);
    ///------------------------生成装甲板对象----------------------------------------------
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
        if (object.color == 3)
            armor.key = "P" + to_string(object.cls);
        memcpy(armor.apex2d, object.apex, 4 * sizeof(cv::Point2f));
        for(int i = 0; i < 4; i++)
        {
            armor.apex2d[i] += Point2f((float)roi_offset.x,(float)roi_offset.y);
        }

        Point2f apex_sum;
        for(auto apex : armor.apex2d)
            apex_sum +=apex;
        armor.center2d = apex_sum / 4.f;

        // auto pnp_result = coordsolver.pnp(armor.apex2d, rmat_imu, SOLVEPNP_ITERATIVE);
        auto pnp_result = coordsolver.pnp(armor.apex2d, rmat_imu, SOLVEPNP_IPPE);
        // auto pnp_result = coordsolver.pnp(armor.apex2d, rmat_imu, SOLVEPNP_IPPE_SQUARE);

        armor.center3d_world = pnp_result.coord_world;
        armor.center3d_cam = pnp_result.coord_cam;
        armor.euler = pnp_result.euler;
        armor.area = object.area;
        armors.push_back(armor);
    }
    //若无合适装甲板
    if (armors.empty())
    {
#ifdef SHOW_AIM_CROSS
        line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), Scalar(0,255,0), 1);
        line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), Scalar(0,255,0), 1);
#endif //SHOW_AIM_CROSS
#ifdef SHOW_IMG
        namedWindow("dst",0);
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
#ifdef USING_SPIN_DETECT
        updateSpinScore();
#endif //USING_SPIN_DETECT
        lost_cnt++;
        is_last_target_exists = false;
        last_target_area = 0;
        return false;
    }
    ///------------------------生成/分配ArmorTracker----------------------------
    new_armors_cnt_map.clear();
    //为装甲板分配或新建最佳ArmorTracker

    for (auto armor = armors.begin(); armor != armors.end(); ++armor)
    {
        auto predictors_with_same_key = trackers_map.count((*armor).key);
        //当不存在该类型装甲板ArmorTracker
        if (predictors_with_same_key == 0)
        {
            ArmorTracker tracker((*armor), src.timestamp);
            auto target_predictor = trackers_map.insert(make_pair((*armor).key, tracker));
            new_armors_cnt_map[(*armor).key]++;
        }
        //当存在一个该类型ArmorTracker
        else if (predictors_with_same_key == 1)
        {
            auto candidate = trackers_map.find((*armor).key);
            auto delta_t = src.timestamp - (*candidate).second.prev_timestamp;
            auto delta_dist = ((*armor).center3d_world - (*candidate).second.prev_armor.center3d_world).norm();
            auto velocity = (delta_dist / delta_t) * 1e3;
            //若匹配则使用此ArmorTracker
            if (velocity <= max_v)
            {
                (*candidate).second.update((*armor), src.timestamp);
            }
            //若不匹配则创建新ArmorTracker
            else
            {
                ArmorTracker tracker((*armor), src.timestamp);
                trackers_map.insert(make_pair((*armor).key, tracker));
                new_armors_cnt_map[(*armor).key]++;
            }
        }
        //当存在多个该类型装甲板ArmorTracker
        else
        {
            //1e9无实际意义，仅用于以非零初始化
            double min_v = 1e9;
            int min_delta_t = 1e9;
            bool is_best_candidate_exist = false;
            std::multimap<string, ArmorTracker>::iterator best_candidate;
            auto candiadates = trackers_map.equal_range((*armor).key);
            //遍历所有同Key预测器，匹配速度最小且更新时间最近的ArmorTracker
            for (auto iter = candiadates.first; iter != candiadates.second; ++iter)
            {
                auto delta_t = src.timestamp - (*iter).second.prev_timestamp;
                auto delta_dist = ((*armor).center3d_world - (*iter).second.prev_armor.center3d_world).norm();
                auto velocity = (delta_dist / delta_t) * 1e3;
                if (velocity <= max_v && velocity <= min_v && delta_t <= min_delta_t)
                {
                    min_delta_t = delta_t;
                    min_v = velocity;
                    best_candidate = iter;
                    is_best_candidate_exist = true;
                }
            }
            if (is_best_candidate_exist)
            {
                auto velocity = min_v;
                auto delta_t = min_delta_t;
                (*best_candidate).second.update((*armor), src.timestamp);
            }
            else
            {
                ArmorTracker tracker((*armor), src.timestamp);
                trackers_map.insert(make_pair((*armor).key, tracker));
                new_armors_cnt_map[(*armor).key]++;
            }

        }
    }
    if (trackers_map.size() != 0)
    {
        //维护预测器Map，删除过久之前的装甲板
        for (auto iter = trackers_map.begin(); iter != trackers_map.end();)
        {
            //删除元素后迭代器会失效，需先行获取下一元素
            auto next = iter;
            // cout<<(*iter).second.last_timestamp<<"  "<<src.timestamp<<endl;
            if ((*iter).second.last_timestamp - src.timestamp > max_delta_t)
                next = trackers_map.erase(iter);
            else
                ++next;
            iter = next;
        }
    }
    // cout<<"::"<<predictors_map.size()<<endl;
#ifdef USING_SPIN_DETECT
    ///------------------------检测装甲板变化情况,计算各车陀螺分数----------------------------
    for (auto cnt : new_armors_cnt_map)
    {
        if (cnt.second == 1)
        {
            auto same_armors_cnt = trackers_map.count(cnt.first);
            //当存在同时存在两块同类别装甲板时才进入陀螺状态识别
            if (same_armors_cnt == 2)
            {
                auto candidates = trackers_map.equal_range(cnt.first);
                double last_armor_center;
                double new_armor_center;

                auto candidate = candidates.first;
                //确定新增装甲板与历史装甲板
                if ((*candidate).second.is_initialized)
                {
                    last_armor_center = (*candidate).second.last_armor.center3d_cam[0];
                    ++candidate;
                    if (!(*candidate).second.is_initialized)
                        new_armor_center = (*candidate).second.last_armor.center3d_cam[0];
                    else
                        continue;
                }
                else
                {
                    new_armor_center = (*candidate).second.last_armor.center3d_cam[0];
                    ++candidate;
                    if ((*candidate).second.is_initialized)
                        last_armor_center = (*candidate).second.last_armor.center3d_cam[0];
                    else
                        continue;
                }

                auto spin_movement = new_armor_center - last_armor_center;

                if (spin_score_map[cnt.first] == 0 && abs(spin_movement) > 0.05)
                    spin_score_map[cnt.first] = 100 * spin_movement / abs(spin_movement);
                else if (abs(spin_movement) > 0.05)
                    spin_score_map[cnt.first] = 2 * spin_score_map[cnt.first];
            }
        }
    }
    ///------------------更新反陀螺socre_map，更新各车辆陀螺状态-----------------------------
    updateSpinScore();
    // cout<<"-----------------------"<<endl;
    // for (auto status : spin_status_map)
    // {
    //     cout<<status.first<<" : "<<status.second<<endl;
    // }
#endif //USING_SPIN_DETECT
    ///-----------------------------判断击打车辆------------------------------------------
    auto target_id = chooseTargetID(armors);
    auto ID_candiadates = trackers_map.equal_range(target_id);
    ///---------------------------获取最终装甲板序列---------------------------------------
    bool is_target_spinning;
    Armor target;
    Eigen::Vector3d aiming_point;
    std::vector<ArmorTracker*> final_trackers;
    std::vector<Armor> final_armors;
    //TODO:反陀螺防抖(增加陀螺模式与常规模式)
    //若目标处于陀螺状态，预先瞄准目标中心，待预测值与该点距离较近时开始击打
    SpinHeading spin_status;
    if (spin_status_map.count(target_id) == 0)
    {
        spin_status = UNKNOWN;
        is_target_spinning = false;
    }
    else
    {
        spin_status = spin_status_map[target_id];
        is_target_spinning = true;
    }
    ///----------------------------------反陀螺击打---------------------------------------
    if (spin_status != UNKNOWN)
    {
        //------------------------------尝试确定旋转中心-----------------------------------
        auto available_candidates_cnt = 0;
        for (auto iter = ID_candiadates.first; iter != ID_candiadates.second; ++iter)
        {
            final_armors.push_back((*iter).second.last_armor);
            //若Tracker未完成初始化，不考虑使用
            // if (!(*iter).second.is_initialized || (*iter).second.history_info.size() < 3)
            // {
            //     continue;
            // }
            // else
            // {
            //     final_trackers.push_back(&(*iter).second);
            //     available_candidates_cnt++;
            // }
        }
        // if (available_candidates_cnt == 0)
        // {
        //     cout<<"Invalid"<<endl;   
        // }
        // else
        // {   //TODO:改进旋转中心识别方法
        //     //FIXME:目前在目标小陀螺时并移动时，旋转中心的确定可能存在问题，故该语句块中的全部计算结果均暂未使用
        //     //-----------------------------计算陀螺旋转半径--------------------------------------
        //     Eigen::Vector3d rotate_center_cam = {0,0,0};
        //     Eigen::Vector3d rotate_center_car = {0,0,0};
        //     for(auto tracker : final_trackers)
        //     {
        //         std::vector<Eigen::Vector3d> pts;
        //         for (auto pt : tracker->history_info)
        //         {
        //             pts.push_back(pt.center3d_world);
        //         }
        //         auto sphere = FitSpaceCircle(pts);
        //         auto radius = sphere[3];
        //         if (tracker->radius == 0)
        //             tracker->radius = radius;
        //         else//若不为初值，尝试进行半径平均以尽量误差
        //             tracker->radius = (tracker->radius + radius) / 2;
        //         //-----------------------------计算陀螺中心与预瞄点-----------------------------------
        //         //此处世界坐标系指装甲板世界坐标系，而非车辆世界坐标系
        //         Eigen::Vector3d rotate_center_world = {0,
        //                             sin(25 * 180 / CV_PI) * tracker->radius,
        //                             - cos(25 * 180 / CV_PI) * tracker->radius};
        //         auto rotMat = eulerToRotationMatrix(tracker->prev_armor.euler);
        //         //Pc = R * Pw + T
        //         rotate_center_cam = (rotMat * rotate_center_world) + tracker->prev_armor.center3d_cam;
        //         rotate_center_car += coordsolver.worldToCam(rotate_center_cam, rmat_imu);
        //     }
        //     //求解旋转中心
        //     rotate_center_car /= final_trackers.size();
        // }
        //若存在一块装甲板
        if (final_armors.size() == 1)
        {
            target = final_armors.at(0);
        }
        //若存在两块装甲板
        else if (final_armors.size() == 2)
        {
            //对最终装甲板进行排序，选取与旋转方向相同的装甲板进行更新
            sort(final_armors.begin(),final_armors.end(),[](Armor& prev, Armor& next)
                                {return prev.center3d_cam[0] < next.center3d_cam[0];});
            //若顺时针旋转选取右侧装甲板更新
            if (spin_status == CLOCKWISE)
                target = final_armors.at(1);
            //若逆时针旋转选取左侧装甲板更新
            else if (spin_status == COUNTER_CLOCKWISE)
                target = final_armors.at(0);
        }
#ifdef USING_PREDICT
        Eigen::Vector3d predict_value;
        auto delta_t = src.timestamp - last_timestamp;
        auto delta_dist = (target.center3d_world - last_armor.center3d_world).norm();
        auto velocity = (delta_dist / delta_t) * 1e3;
        if (target.key != last_armor.key || velocity > max_v)
        {
            predictor.initParam(predictor_param_loader);
            aiming_point = target.center3d_cam;
        }
        else
        {
            auto aiming_point_world = predictor.predict(target.center3d_world, src.timestamp);
            // aiming_point = aiming_point_world;
            aiming_point = coordsolver.worldToCam(aiming_point_world, rmat_imu);
        }
#endif //USING_PREDICT

#ifndef USING_PREDICT
    aiming_point = coordsolver.worldToCam(target.center3d_world,rmat_imu);
    // aiming_point = target.center3d_cam;#endif //USING_PREDICT
#endif //USING_PREDICT        
    }
    ///----------------------------------常规击打---------------------------------------
    else
    {
        for (auto iter = ID_candiadates.first; iter != ID_candiadates.second; ++iter)
        {
            final_armors.push_back((*iter).second.last_armor);
        }
        //选取最大的装甲板进行击打
        target = chooseTargetArmor(final_armors);

#ifdef USING_PREDICT
        auto delta_t = src.timestamp - last_timestamp;
        auto delta_dist = (target.center3d_world - last_armor.center3d_world).norm();
        auto velocity = (delta_dist / delta_t) * 1e3;

        if (target.key != last_armor.key || velocity > max_v)
        {
            predictor.initParam(predictor_param_loader);
            aiming_point = target.center3d_cam;
        }
        else
        {
            auto aiming_point_world = predictor.predict(target.center3d_world, src.timestamp);
            // aiming_point = aiming_point_world;
            aiming_point = coordsolver.worldToCam(aiming_point_world, rmat_imu);
        }
#else
    aiming_point = coordsolver.worldToCam(target.center3d_world,rmat_imu);
#endif //USING_PREDICT
    }
#ifdef ASSIST_LABEL
    auto label_name = path_prefix + to_string(cnt) + ".txt";
    string content;

    int cls = 0;
    if (target.id == 7)
        cls = 9 * target.color - 1;
    if (target.id != 7)
        cls = target.id + target.color * 9;
    
    content.append(to_string(cls) + " ");
    for (auto apex : target.apex2d)
    {
        content.append(to_string((apex.x - roi_offset.x) / input_size.width));
        content.append(" ");
        content.append(to_string((apex.y - roi_offset.y) / input_size.height));
        content.append(" ");
    }
    content.pop_back();
    cout<<to_string(cnt) + " "<<content<<endl;
    file.open(label_name,std::ofstream::app);
    file<<content;
    file.close();
    cnt++;
    sleep(0.05);
#endif //ASSIST_LABEL
    //获取装甲板中心与装甲板面积以下一次ROI截取使用
    last_roi_center = target.center2d;
    last_armor = target;
    last_timestamp = src.timestamp;
    last_target_area = target.area;
    last_aiming_point = aiming_point;
    is_last_target_exists = true;

#ifdef SHOW_AIM_CROSS
        line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), {0,255,0}, 1);
        line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), {0,255,0}, 1);
#endif //SHOW_AIM_CROSS

#ifdef SHOW_ALL_ARMOR
    for (auto armor :armors)
    {
        if (armor.color == 0)
            putText(src.img, fmt::format("B{}",armor.id),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, {255, 100, 100}, 2);
        if (armor.color == 1)
            putText(src.img, fmt::format("R{}",armor.id),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, {0, 0, 255}, 2);
        if (armor.color == 2)
            putText(src.img, fmt::format("N{}",armor.id),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, {255, 255, 255}, 2);
        if (armor.color == 3)
            putText(src.img, fmt::format("P{}",armor.id),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, {255, 100, 255}, 2);
        for(int i = 0; i < 4; i++)
            line(src.img, armor.apex2d[i % 4], armor.apex2d[(i + 1) % 4], {0,255,0}, 1);
        auto armor_center = coordsolver.reproject(armor.center3d_cam);
        circle(src.img, armor_center, 4, {0, 0, 255}, 2);
    }
#endif //SHOW_ALL_ARMOR

#ifdef SHOW_PREDICT
    auto aiming_2d = coordsolver.reproject(aiming_point);
    circle(src.img, aiming_2d, 2, {0, 255, 255}, 2);
#endif //SHOW_PREDICT

    auto angle = coordsolver.getAngle(aiming_point,rmat_imu);
    auto time_predict = std::chrono::steady_clock::now();

    double dr_crop_ms = std::chrono::duration<double,std::milli>(time_crop - time_start).count();
    double dr_infer_ms = std::chrono::duration<double,std::milli>(time_infer - time_crop).count();
    double dr_predict_ms = std::chrono::duration<double,std::milli>(time_predict - time_infer).count();
    double dr_full_ms = std::chrono::duration<double,std::milli>(time_predict - time_start).count();

#ifdef SHOW_FPS
    putText(src.img, fmt::format("FPS: {}",int(1000 / dr_full_ms)), {10, 25}, FONT_HERSHEY_SIMPLEX, 1, {0,255,0});
#endif //SHOW_FPS

#ifdef SHOW_IMG
    namedWindow("dst",0);
    imshow("dst",src.img);
    waitKey(1);
#endif //SHOW_IMG

#ifdef PRINT_LATENCY
    //降低输出频率，避免影响帧率
    if (src.timestamp % 10 == 0)
    {
        fmt::print(fmt::fg(fmt::color::gray), "-----------TIME------------\n");
        fmt::print(fmt::fg(fmt::color::blue_violet), "Crop: {} ms\n"   ,dr_crop_ms);
        fmt::print(fmt::fg(fmt::color::golden_rod), "Infer: {} ms\n",dr_infer_ms);
        fmt::print(fmt::fg(fmt::color::green_yellow), "Predict: {} ms\n",dr_predict_ms);
        fmt::print(fmt::fg(fmt::color::orange_red), "Total: {} ms\n",dr_full_ms);
    }
#endif //PRINT_LATENCY

#ifdef PRINT_TARGET_INFO
    fmt::print(fmt::fg(fmt::color::gray), "-----------INFO------------\n");
    fmt::print(fmt::fg(fmt::color::blue_violet), "Yaw: {} \n",angle[0]);
    fmt::print(fmt::fg(fmt::color::golden_rod), "Pitch: {} \n",angle[1]);
    fmt::print(fmt::fg(fmt::color::green_yellow), "Dist: {} m\n",(float)target.center3d_cam.norm());
    fmt::print(fmt::fg(fmt::color::white), "Target: {} \n",target.key);
    fmt::print(fmt::fg(fmt::color::orange_red), "Is Spinning: {} \n",is_target_spinning);
#endif //PRINT_TARGET_INFO
#ifdef SAVE_AUTOAIM_LOG
    LOG(INFO) << "Crop: " << dr_crop_ms << " ms" << " Infer: " << dr_infer_ms << " ms" << " Predict: " << dr_predict_ms << " ms" << " Total: " << dr_full_ms << " ms";
    LOG(INFO) << "Yaw: " << angle[0] << " Pitch: " << angle[1] << " Dist: " << (float)target.center3d_cam.norm() << " Target: " << target.key << " Is Spinning: " << is_target_spinning;
#endif //SAVE_AUTOAIM_LOG

    data = {(float)angle[1], (float)angle[0], (float)target.center3d_cam.norm(), 0, 1, is_target_spinning, 1};
    return true;
}
