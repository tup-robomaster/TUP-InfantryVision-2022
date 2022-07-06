#include "autoaim.h"

using namespace cv;
using namespace std;

#ifdef ASSIST_LABEL
const string path_prefix = "../dataset/";
ofstream file;
#endif //ASSIST_LABEL

Autoaim::Autoaim()
{
    detector.initModel(network_path);
    predictor_param_loader.initParam(predict_param_path);
    coordsolver.loadParam(camera_param_path,camera_name);
    // cout<<"...."<<endl;
    lost_cnt = 0;
    dead_buffer_cnt = 0;
    is_last_target_exists = false;
    is_target_switched = false;
    last_target_area = 0;
    last_bullet_speed = 0;
    input_size = {416, 416};
    predictor.initParam(predictor_param_loader);

#ifdef DETECT_RED
    detect_color = RED;
#endif //DETECT_RED
#ifdef DETECT_BLUE
    detect_color = BLUE;
#endif //DETECT_BLUE

    fmt::print(fmt::fg(fmt::color::pale_violet_red), "[AUTOAIM] Autoaim init model success! Size: {} {}\n", input_size.height, input_size.width);

#ifdef SAVE_AUTOAIM_LOG
    LOG(INFO)<<"[AUTOAIM] Autoaim init model success! Size: "<<input_size.height<<" "<<input_size.width;
#endif //SAVE_AUTOAIM_LOG
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

    //FIXME:自适应大小ROI截取可能存在Bug
    auto area_ratio = last_target_area / img.size().area();
    if (!is_last_target_exists)
    {
        // cout<<lost_cnt<<endl;
        //当丢失目标帧数过多或lost_cnt为初值
        if (lost_cnt > max_lost_cnt || lost_cnt == 0 || area_ratio == 0)
        {
            return Point2i(0,0);
        }
    }
    //若目标大小大于阈值
    //丢失目标帧数较小则放大ROI区域
    // if (lost_cnt <= max_lost_cnt)
    //     area_ratio*=(1 + lost_cnt);

    if (area_ratio > no_crop_ratio)
    {
        return Point2i(0,0);
    }
    int max_expand = (img.size().height - input_size.height) / 2;
    int expand_value = (int)((area_ratio / no_crop_ratio) * max_expand) / 32 * 32;
    // cout<<"last:"<<last_roi_center<<endl;
    // Size2i cropped_size = input_size;
    Size2i cropped_size = {input_size + Size2i(expand_value, expand_value)};
    // cout<<cropped_size<<" "<<expand_value<<endl;
    if (cropped_size.width >= img.size().width)
        cropped_size.width = img.size().width;
    if (cropped_size.height >= img.size().height)
        cropped_size.height = img.size().height;

    //处理X越界
    if (last_roi_center.x <= cropped_size.width / 2)
        last_roi_center.x = cropped_size.width / 2;
    if (last_roi_center.x > (img.size().width - cropped_size.width / 2))
        last_roi_center.x = img.size().width - cropped_size.width / 2;
    //处理Y越界
    if (last_roi_center.y <= cropped_size.height / 2)
        last_roi_center.y = cropped_size.height / 2;
    if (last_roi_center.y > (img.size().height - cropped_size.height / 2))
        last_roi_center.y = img.size().height - cropped_size.height / 2;
    
    //左上角顶点
    auto offset = last_roi_center - Point2i(cropped_size.width / 2, cropped_size.height / 2);
    // auto offset = last_roi_center - Point2i(roi_width / 2, roi_height / 2);
    Rect roi_rect = Rect(offset, cropped_size);
    img(roi_rect).copyTo(img);

    // namedWindow("roi", img);
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
        // cout<<(*score).first<<"--:"<<(*score).second<<" "<<spin_status<<endl;
        LOG(INFO)<<"[SpinDetection] Current Spin score :"<<(*score).first<<" : "<<(*score).second<<" "<<spin_status;
        // 若分数过低移除此元素
        if (abs((*score).second) <= anti_spin_judge_low_thres && spin_status != UNKNOWN)
        {
            fmt::print(fmt::fg(fmt::color::red), "[SpinDetection] Removing {}.\n", (*score).first);
            LOG(INFO)<<"[SpinDetection] Removing "<<(*score).first;
            spin_status_map.erase((*score).first);
            score = spin_score_map.erase(score);
            continue;
        }
        // 
        if (spin_status != UNKNOWN)
            (*score).second = 0.978 * (*score).second - 1 * abs((*score).second) / (*score).second;
        else
            (*score).second = 0.997 * (*score).second - 1 * abs((*score).second) / (*score).second;
        //当小于该值时移除该元素
        if (abs((*score).second) < 3 || isnan((*score).second))
        {
            spin_status_map.erase((*score).first);
            score = spin_score_map.erase(score);
            continue;
        }
        else if (abs((*score).second) >= anti_spin_judge_high_thres)
        {
            (*score).second = anti_spin_judge_high_thres * abs((*score).second) / (*score).second;
            if ((*score).second > 0)
                spin_status_map[(*score).first] = CLOCKWISE;
            else if((*score).second < 0)
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
 * @brief 从ArmorTracker选出最佳Tracker
 * 
 * @param trackers Trackers
 * @param timestamp 本次时间戳 
 * @return ArmorTracker* 选中的ArmorTracker
 */
ArmorTracker* Autoaim::chooseTargetTracker(vector<ArmorTracker*> trackers, int timestamp)
{
    //TODO:优化打击逻辑
    //TODO:本逻辑为哨兵逻辑
    float max_score = 0;
    int target_idx = 0;
    int last_target_idx = -1;
    // cout<<trackers.size()<<endl;
    for (int i = 0; i < trackers.size(); i++)
    {

        //计算tracker的切换打击分数,由装甲板旋转角度,距离,面积大小决定
        if (trackers[i]->last_timestamp == timestamp)
        {
            if (trackers[i]->last_selected_timestamp == prev_timestamp && abs(prev_timestamp - timestamp) < 100)
                last_target_idx = i;
            if (trackers[i]->hit_score > max_score)
            {
                max_score = trackers[i]->hit_score;
                target_idx = i;
            }
        }
    }

    //若存在上次存在目标且分数与相差不大，选择该装甲板
    if (last_target_idx != -1 && abs(trackers[last_target_idx]->hit_score - max_score) / max_score < 0.1)
        target_idx = last_target_idx;
    return trackers[target_idx];
}

/**
 * @brief 选择击打车辆ID
 * 
 * @param armors 
 * @return int 
 */
int Autoaim::chooseTargetID(vector<Armor> &armors, int timestamp)
{
    //TODO:自瞄逻辑修改
    bool is_last_id_exists = false;
    int target_id;
    //该选择逻辑主要存在两层约束:
    //英雄约束与上次目标约束
    //若检测到危险距离内的英雄直接退出循环
    //若检测到存在上次击打目标,时间较短,且该目标运动较小,则将其选为候选目标,若遍历结束未发现危险距离内的英雄则将其ID选为目标ID.
    for (auto armor : armors)
    {
        //FIXME:该处需根据兵种修改
        //若视野中存在英雄且距离小于危险距离，直接选为目标
        if (armor.id == 1 && armor.center3d_world.norm() <= hero_danger_zone)
        {
            return armor.id;
        }
        //若存在上次击打目标,时间较短,且该目标运动较小则将其选为候选目标,若遍历结束未发现危险距离内的英雄则将其ID选为目标ID.
        else if (armor.id == last_armor.id && abs(armor.area - last_armor.area) / (float)armor.area < 0.3 && abs(timestamp - prev_timestamp) < 30)
        {
            is_last_id_exists = true;
            target_id = armor.id;
        }
    }
    //若不存在则返回面积最大的装甲板序号，即队列首元素序号
    if (is_last_id_exists)
        return target_id;
    else
        return (*armors.begin()).id;
}


/**
 * @brief 自瞄主函数
 * 
 * @param src 图像、时间戳、IMU数据
 * @param data 发送数据所用结构体
 * @return true 识别成功
 * @return false 识别失败
 */
bool Autoaim::run(TaskData &src,VisionData &data)
{
    auto time_start=std::chrono::steady_clock::now();
    vector<ArmorObject> objects;
    vector<Armor> armors;

    auto input = src.img.clone();
    // cout<<input.size<<endl;
    //若为前哨站吊射模式,直接截取图像中间部分进行处理

#ifdef USING_IMU
    Eigen::Matrix3d rmat_imu = src.quat.toRotationMatrix();
    auto vec = rotationMatrixToEulerAngles(rmat_imu);
    // cout<<"Euler : "<<vec[0] * 180.f / CV_PI<<" "<<vec[1] * 180.f / CV_PI<<" "<<vec[2] * 180.f / CV_PI<<endl;
#else
    Eigen::Matrix3d rmat_imu = Eigen::Matrix3d::Identity();
#endif //USING_IMU

#ifndef DEBUG_WITHOUT_COM
    //设置弹速,若弹速大于10m/s值,且弹速变化大于0.5m/s则更新
    if (src.bullet_speed > 10)
    {
        double bullet_speed;
        if (abs(src.bullet_speed - last_bullet_speed) < 0.5 || abs(src.bullet_speed - last_bullet_speed) > 1.5)
        {
            bullet_speed = src.bullet_speed;
            predictor.setBulletSpeed(bullet_speed);
            coordsolver.setBulletSpeed(bullet_speed);
            last_bullet_speed = bullet_speed;
            LOG(INFO)<<"SPD Updated:"<<src.bullet_speed<<" : "<<last_bullet_speed;
        }
        
    }
#endif //DEBUG_WITHOUT_COM
    // cout<<"lost:"<<lost_cnt<<endl;

#ifdef USING_ROI
    //吊射模式采用固定ROI
    if (src.mode == 2)
    {
        // cout<<zoom_offset.x<<endl;
        //若不存在目标则进行中间区域ROI，默认大小为416x416
        if (lost_cnt >= max_lost_cnt && !is_last_target_exists)
        {
            roi_offset = zoom_offset;
            // cout<<
            input(Range(zoom_offset.y, zoom_offset.y + input_size.height),
                    Range(zoom_offset.x, zoom_offset.x + input_size.width)).copyTo(input);
        }
        else
        {
            roi_offset = cropImageByROI(input);
        }
    }
    else
    {
        roi_offset = cropImageByROI(input);
    }
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
        lost_cnt++;
        is_last_target_exists = false;
        data = {(float)0, (float)0, (float)0, 0, 0, 0, 1};
        LOG(WARNING) <<"[AUTOAIM] No target detected!";
        return false;
    }
#ifdef ASSIST_LABEL
    auto img_name = path_prefix + to_string(src.timestamp) + ".jpg";
    imwrite(img_name,input);
#endif //ASSIST_LABEL
    auto time_infer = std::chrono::steady_clock::now();
    ///------------------------将对象排序，保留面积较大的对象---------------------------------
    sort(objects.begin(),objects.end(),[](ArmorObject& prev, ArmorObject& next)
                                    {return prev.area > next.area;});
    //若对象较多保留前按面积排序后的前max_armors个
    if (objects.size() > max_armors)
        objects.resize(max_armors);
    ///------------------------生成装甲板对象----------------------------------------------
    for (auto object : objects)
    {
        // cout<<"offset:"<<roi_offset<<endl;
        Armor armor;
        armor.id = object.cls;
        armor.color = object.color;
        armor.conf = object.prob;

#ifdef IGNORE_ENGINEER
        if (object.cls == 2)
            continue;
#endif //IGNORE_ENGINEER

#ifdef IGNORE_NPC
        if (object.cls == 0 || object.cls == 6 || object.cls == 7)
            continue;
#endif // IGNORE_NPC

        //放行对应颜色装甲板或灰色装甲板
        if (detect_color == RED)
            if (!(object.color == 1 || object.color == 2))
                continue;
        if (detect_color == BLUE)
            if (!(object.color == 0 || object.color == 2))
                continue;

        //如果装甲板为灰色且类别不为上次击打装甲板类别
        if (object.color == 2 && object.cls != last_armor.id)
            continue;
        if (object.color == 2 && object.cls == last_armor.id && dead_buffer_cnt >= max_dead_buffer)
            continue;
                
        //生成Key
        if (object.color == 0)
            armor.key = "B" + to_string(object.cls);
        if (object.color == 1)
            armor.key = "R" + to_string(object.cls);
        if (object.color == 2)
            armor.key = "N" + to_string(object.cls);
        if (object.color == 3)
            armor.key = "P" + to_string(object.cls);
        //生成顶点与装甲板二维中心点
        memcpy(armor.apex2d, object.apex, 4 * sizeof(cv::Point2f));
        for(int i = 0; i < 4; i++)
        {
            armor.apex2d[i] += Point2f((float)roi_offset.x,(float)roi_offset.y);
        }
        Point2f apex_sum;
        for(auto apex : armor.apex2d)
            apex_sum +=apex;
        armor.center2d = apex_sum / 4.f;
        //生成装甲板旋转矩形和ROI
        std::vector<Point2f> points_pic(armor.apex2d, armor.apex2d + 4);
        RotatedRect points_pic_rrect = minAreaRect(points_pic);        
        armor.rrect = points_pic_rrect;
        auto bbox = points_pic_rrect.boundingRect();
        auto x = bbox.x - 0.5 * bbox.width * (armor_roi_expand_ratio_width - 1);
        auto y = bbox.y - 0.5 * bbox.height * (armor_roi_expand_ratio_height - 1);
        armor.roi = Rect(x,
                        y,
                        bbox.width * armor_roi_expand_ratio_width,
                        bbox.height * armor_roi_expand_ratio_height
                        );
        //若装甲板置信度小于高阈值，需要相同位置存在过装甲板才放行
        if (armor.conf < armor_conf_high_thres)
        {
            if (last_armors.empty())
            {
                continue;
            }
            else
            {
                bool is_this_armor_available = false;
                for (auto last_armor : last_armors)
                {
                    if (last_armor.roi.contains(armor.center2d))
                    {
                        is_this_armor_available = true;
                        break;
                    }
                }
                if (!is_this_armor_available)
                {
                    // cout<<"IGN"<<endl;
                    continue;
                }
            }
        }
        // cout<<"..."<<endl;
        //进行PnP，目标较少时采取迭代法，较多时采用IPPE
        int pnp_method;
        if (objects.size() <= 2)
            pnp_method = SOLVEPNP_ITERATIVE;
        else
            pnp_method = SOLVEPNP_IPPE;
        TargetType target_type = SMALL;
        //计算长宽比,确定装甲板类型
        auto apex_wh_ratio = max(points_pic_rrect.size.height, points_pic_rrect.size.width) /
                                 min(points_pic_rrect.size.height, points_pic_rrect.size.width);
        //若大于长宽阈值或为哨兵、英雄装甲板
        //FIXME:若存在平衡步兵需要对此处步兵装甲板类型进行修改
        if (object.cls == 0 || object.cls == 1)
            target_type = BIG;
        else if (object.cls == 2 || object.cls == 3 || object.cls == 4 || object.cls == 5 || object.cls == 6)
            target_type = SMALL;
        else if (apex_wh_ratio > armor_type_wh_thres)
            target_type = BIG;
        // for (auto pic : points_pic)
        //     cout<<pic<<endl;
        auto pnp_result = coordsolver.pnp(points_pic, rmat_imu, target_type, pnp_method);
        //防止装甲板类型出错导致解算问题，距离过大或出现NAN直接跳过该装甲板
        if (pnp_result.armor_cam.norm() > 13 ||
            isnan(pnp_result.armor_cam[0]) ||
            isnan(pnp_result.armor_cam[1]) ||
            isnan(pnp_result.armor_cam[2]))
                continue;
        
        armor.type = target_type;
        armor.center3d_world = pnp_result.armor_world;
        armor.center3d_cam = pnp_result.armor_cam;
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
        data = {(float)0, (float)0, (float)0, 0, 0, 0, 1};
        LOG(WARNING) <<"[AUTOAIM] No available armor exists!";
        return false;
    }

    ///------------------------生成/分配ArmorTracker----------------------------
    new_armors_cnt_map.clear();
    //为装甲板分配或新建最佳ArmorTracker
    //注:将不会为灰色装甲板创建预测器，只会分配给现有的预测器
    for (auto armor = armors.begin(); armor != armors.end(); ++armor)
    {
        //当装甲板颜色为灰色且当前dead_buffer小于max_dead_buffer
        string tracker_key;
        if ((*armor).color == 2)
        {
            if (dead_buffer_cnt >= max_dead_buffer)
                continue;
            
            if (detect_color == RED)
                tracker_key = "R" + to_string((*armor).id);
            if (detect_color == BLUE)
                tracker_key = "B" + to_string((*armor).id);
        }
        else
        {
            tracker_key = (*armor).key;
        }

        auto predictors_with_same_key = trackers_map.count(tracker_key);
        //当不存在该类型装甲板ArmorTracker且该装甲板Tracker类型不为灰色装甲板
        if (predictors_with_same_key == 0 && (*armor).color != 2)
        {
            ArmorTracker tracker((*armor), src.timestamp);
            auto target_predictor = trackers_map.insert(make_pair((*armor).key, tracker));
            new_armors_cnt_map[(*armor).key]++;
        }
        //当存在一个该类型ArmorTracker
        else if (predictors_with_same_key == 1)
        {
            auto candidate = trackers_map.find(tracker_key);
            auto delta_t = src.timestamp - (*candidate).second.last_timestamp;
            auto delta_dist = ((*armor).center3d_world - (*candidate).second.last_armor.center3d_world).norm();
            // auto iou = (*candidate).second.last_armor.roi & (*armor)
            // auto velocity = (delta_dist / delta_t) * 1e3;
            //若匹配则使用此ArmorTracker
            if (delta_dist <= max_delta_dist && delta_t > 0 && (*candidate).second.last_armor.roi.contains((*armor).center2d))
            {
                (*candidate).second.update((*armor), src.timestamp);
            }
            //若不匹配则创建新ArmorTracker
            else if ((*armor).color != 2)
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
            double min_delta_dist = 1e9;
            int min_delta_t = 1e9;
            bool is_best_candidate_exist = false;
            std::multimap<string, ArmorTracker>::iterator best_candidate;
            auto candiadates = trackers_map.equal_range(tracker_key);
            //遍历所有同Key预测器，匹配速度最小且更新时间最近的ArmorTracker
            for (auto iter = candiadates.first; iter != candiadates.second; ++iter)
            {
                auto delta_t = src.timestamp - (*iter).second.last_timestamp;
                auto delta_dist = ((*armor).center3d_world - (*iter).second.last_armor.center3d_world).norm();
                auto velocity = (delta_dist / delta_t) * 1e3;
                
                if ((*iter).second.last_armor.roi.contains((*armor).center2d) && delta_t > 0)
                {
                    if (delta_dist <= max_delta_dist && delta_dist <= min_delta_dist &&
                     delta_t <= min_delta_t)
                    {
                        min_delta_t = delta_t;
                        min_delta_dist = delta_dist;
                        best_candidate = iter;
                        is_best_candidate_exist = true;
                    }
                }
            }
            if (is_best_candidate_exist)
            {
                auto velocity = min_delta_dist;
                auto delta_t = min_delta_t;
                (*best_candidate).second.update((*armor), src.timestamp);
            }
            else if ((*armor).color != 2)
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
            if ((src.timestamp - (*iter).second.last_timestamp) > max_delta_t)
                next = trackers_map.erase(iter);
            else
                ++next;
            iter = next;
        }
    }
    // cout<<"::"<<predictors_map.size()<<endl;
    // for (auto member : new_armors_cnt_map)
    //     cout<<member.first<<" : "<<member.second<<endl;
#ifdef USING_SPIN_DETECT
    ///------------------------检测装甲板变化情况,计算各车陀螺分数----------------------------
    for (auto cnt : new_armors_cnt_map)
    {
        //只在该类别新增装甲板时数量为1时计算陀螺分数
        if (cnt.second == 1)
        {
            auto same_armors_cnt = trackers_map.count(cnt.first);
            if (same_armors_cnt == 2)
            {
                // cout<<"1"<<endl;
                //遍历所有同Key预测器，确定左右侧的Tracker
                ArmorTracker *new_tracker = nullptr;
                ArmorTracker *last_tracker = nullptr;
                double last_armor_center;
                double last_armor_timestamp;
                double new_armor_center;
                double new_armor_timestamp;
                int best_prev_timestamp = 0;    //候选ArmorTracker的最近时间戳
                auto candiadates = trackers_map.equal_range(cnt.first);
                for (auto iter = candiadates.first; iter != candiadates.second; ++iter)
                {
                    //若未完成初始化则视为新增tracker
                    if (!(*iter).second.is_initialized && (*iter).second.last_timestamp == src.timestamp)
                    {
                        new_tracker = &(*iter).second;
                    }
                    else if ((*iter).second.last_timestamp > best_prev_timestamp && (*iter).second.is_initialized)
                    {
                        best_prev_timestamp = (*iter).second.last_timestamp;
                        last_tracker = &(*iter).second;
                    }
                    
                }
                if (new_tracker != nullptr && last_tracker != nullptr)
                {
                    new_armor_center = new_tracker->last_armor.center2d.x;
                    new_armor_timestamp = new_tracker->last_timestamp;
                    last_armor_center = last_tracker->last_armor.center2d.x;
                    last_armor_timestamp = last_tracker->last_timestamp;
                    auto spin_movement = new_armor_center - last_armor_center;
                    // auto delta_t = 
                    LOG(INFO)<<"[SpinDetection] Candidate Spin Movement Detected : "<<cnt.first<<" : "<<spin_movement;
                    if (abs(spin_movement) > 10 && new_armor_timestamp == src.timestamp && last_armor_timestamp == src.timestamp)
                    {

                        //若无该元素则插入新元素
                        if (spin_score_map.count(cnt.first) == 0)
                        {
                            spin_score_map[cnt.first] = 1000 * spin_movement / abs(spin_movement);
                        }
                        //若已有该元素且目前旋转方向与记录不同,则对目前分数进行减半惩罚
                        else if (spin_movement * spin_score_map[cnt.first] < 0)
                        {
                            spin_score_map[cnt.first] *= 0.5;
                        }
                        //若已有该元素则更新元素
                        else
                        {
                            spin_score_map[cnt.first] = anti_spin_max_r_multiple * spin_score_map[cnt.first];
                        }
                    }
                }
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
    auto target_id = chooseTargetID(armors, src.timestamp);
    string target_key;
    if (detect_color == BLUE)
        target_key = "B" + to_string(target_id);
    else if (detect_color == RED)
        target_key = "R" + to_string(target_id);
    // cout<<target_key<<endl;
    ///-----------------------------判断该装甲板是否有可用Tracker------------------------------------------
    if (trackers_map.count(target_key) == 0)
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
        lost_cnt++;
        is_last_target_exists = false;
        data = {(float)0, (float)0, (float)0, 0, 0, 0, 1};
        LOG(WARNING) <<"[AUTOAIM] No available tracker exists!";
        return false;
    }
    auto ID_candiadates = trackers_map.equal_range(target_key);
    ///---------------------------获取最终装甲板序列---------------------------------------
    bool is_target_spinning;
    Armor target;
    Eigen::Vector3d aiming_point;
    std::vector<ArmorTracker*> final_trackers;
    std::vector<Armor> final_armors;
    //TODO:反陀螺防抖(增加陀螺模式与常规模式)
    //若目标处于陀螺状态，预先瞄准目标中心，待预测值与该点距离较近时开始击打
    SpinHeading spin_status;
    if (spin_status_map.count(target_key) == 0)
    {
        spin_status = UNKNOWN;
        is_target_spinning = false;
    }
    else
    {
        spin_status = spin_status_map[target_key];
        if (spin_status != UNKNOWN)
            is_target_spinning = true;
        else
            is_target_spinning = false;
    }
    ///----------------------------------反陀螺击打---------------------------------------
    if (spin_status != UNKNOWN)
    {
        //------------------------------尝试确定旋转中心-----------------------------------
        auto available_candidates_cnt = 0;
        for (auto iter = ID_candiadates.first; iter != ID_candiadates.second; ++iter)
        {
            if ((*iter).second.last_timestamp == src.timestamp)
            {
                final_armors.push_back((*iter).second.last_armor);
                final_trackers.push_back(&(*iter).second);
            }
            else
            {
                continue;
            }
            // // 若Tracker未完成初始化，不考虑使用
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

        //判断装甲板是否切换，若切换将变量置1
        auto delta_t = src.timestamp - prev_timestamp;
        auto delta_dist = (target.center3d_world - last_armor.center3d_world).norm();
        auto velocity = (delta_dist / delta_t) * 1e3;
        if ((target.id != last_armor.id || !last_armor.roi.contains((target.center2d))) &&
            is_last_target_exists)
            is_target_switched = true;
        else
            is_target_switched = false;
#ifdef USING_PREDICT
        if (is_target_switched)
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
    // aiming_point = coordsolver.worldToCam(target.center3d_world,rmat_imu);
    aiming_point = target.center3d_cam;
#endif //USING_PREDICT
    }
    ///----------------------------------常规击打---------------------------------------
    else
    {
        for (auto iter = ID_candiadates.first; iter != ID_candiadates.second; ++iter)
        {
            // final_armors.push_back((*iter).second.last_armor);
            final_trackers.push_back(&(*iter).second);
        }
        //进行目标选择
        auto tracker = chooseTargetTracker(final_trackers, src.timestamp);
        tracker->last_selected_timestamp = src.timestamp;
        tracker->selected_cnt++;
        target = tracker->last_armor;
        //判断装甲板是否切换，若切换将变量置1
        auto delta_t = src.timestamp - prev_timestamp;
        auto delta_dist = (target.center3d_world - last_armor.center3d_world).norm();
        auto velocity = (delta_dist / delta_t) * 1e3;
        // cout<<(delta_dist >= max_delta_dist)<<" "<<!last_armor.roi.contains(target.center2d)<<endl;
        if ((target.id != last_armor.id || !last_armor.roi.contains((target.center2d))) &&
            is_last_target_exists)
            is_target_switched = true;
        else
            is_target_switched = false;


#ifdef USING_PREDICT
        //目前类别预测不是十分稳定,若之后仍有问题，可以考虑去除类别判断条件
        if (is_target_switched)
        {
            predictor.initParam(predictor_param_loader);
            // cout<<"initing"<<endl;
            aiming_point = target.center3d_cam;
        }
        else
        {
            auto aiming_point_world = predictor.predict(target.center3d_world, src.timestamp);
            // aiming_point = aiming_point_world;
            aiming_point = coordsolver.worldToCam(aiming_point_world, rmat_imu);
        }
#else
    // aiming_point = coordsolver.worldToCam(target.center3d_world,rmat_imu);
    aiming_point = target.center3d_cam;
#endif //USING_PREDICT
    }
#ifdef ASSIST_LABEL
    auto label_name = path_prefix + to_string(src.timestamp) + ".txt";
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
    cout<<to_string(src.timestamp) + " "<<content<<endl;
    file.open(label_name,std::ofstream::app);
    file<<content;
    file.close();
    usleep(5000);
#endif //ASSIST_LABEL

    if (target.color == 2)
        dead_buffer_cnt++;
    else
        dead_buffer_cnt = 0;
    //获取装甲板中心与装甲板面积以下一次ROI截取使用
    last_roi_center = target.center2d;
    // last_roi_center = Point2i(512,640);
    last_armor = target;
    lost_cnt = 0;
    prev_timestamp = src.timestamp;
    last_target_area = target.area;
    last_aiming_point = aiming_point;
    is_last_target_exists = true;
    last_armors.clear();
    last_armors = armors;
#ifdef SHOW_AIM_CROSS
    line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), {0,255,0}, 1);
    line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), {0,255,0}, 1);
#endif //SHOW_AIM_CROSS

#ifdef SHOW_ALL_ARMOR
    for (auto armor :armors)
    {
        putText(src.img, fmt::format("{:.2f}", armor.conf),armor.apex2d[3],FONT_HERSHEY_SIMPLEX, 1, {0, 255, 0}, 2);
        if (armor.color == 0)
            putText(src.img, fmt::format("B{}",armor.id),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, {255, 100, 0}, 2);
        if (armor.color == 1)
            putText(src.img, fmt::format("R{}",armor.id),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, {0, 0, 255}, 2);
        if (armor.color == 2)
            putText(src.img, fmt::format("N{}",armor.id),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, {255, 255, 255}, 2);
        if (armor.color == 3)
            putText(src.img, fmt::format("P{}",armor.id),armor.apex2d[0],FONT_HERSHEY_SIMPLEX, 1, {255, 100, 255}, 2);
        for(int i = 0; i < 4; i++)
            line(src.img, armor.apex2d[i % 4], armor.apex2d[(i + 1) % 4], {0,255,0}, 1);
        rectangle(src.img, armor.roi, {255, 0, 255}, 1);
        auto armor_center = coordsolver.reproject(armor.center3d_cam);
        circle(src.img, armor_center, 4, {0, 0, 255}, 2);
    }
#endif //SHOW_ALL_ARMOR

#ifdef SHOW_PREDICT
    auto aiming_2d = coordsolver.reproject(aiming_point);
    circle(src.img, aiming_2d, 2, {0, 255, 255}, 2);
#endif //SHOW_PREDICT

    auto angle = coordsolver.getAngle(aiming_point, rmat_imu);
    //若预测出错则直接世界坐标系下坐标作为击打点
    if (isnan(angle[0]) || isnan(angle[1]))
        angle = coordsolver.getAngle(target.center3d_cam, rmat_imu);
    auto time_predict = std::chrono::steady_clock::now();

    double dr_crop_ms = std::chrono::duration<double,std::milli>(time_crop - time_start).count();
    double dr_infer_ms = std::chrono::duration<double,std::milli>(time_infer - time_crop).count();
    double dr_predict_ms = std::chrono::duration<double,std::milli>(time_predict - time_infer).count();
    double dr_full_ms = std::chrono::duration<double,std::milli>(time_predict - time_start).count();

#ifdef SHOW_FPS
    putText(src.img, fmt::format("FPS: {}", int(1000 / dr_full_ms)), {10, 25}, FONT_HERSHEY_SIMPLEX, 1, {0,255,0});
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
    // cout<<target.center3d_world<<endl;
    // cout<<endl;
#ifdef PRINT_TARGET_INFO
    fmt::print(fmt::fg(fmt::color::gray), "-----------INFO------------\n");
    fmt::print(fmt::fg(fmt::color::blue_violet), "Yaw: {} \n",angle[0]);
    fmt::print(fmt::fg(fmt::color::golden_rod), "Pitch: {} \n",angle[1]);
    fmt::print(fmt::fg(fmt::color::green_yellow), "Dist: {} m\n",(float)target.center3d_cam.norm());
    fmt::print(fmt::fg(fmt::color::white), "Target: {} \n",target.key);
    fmt::print(fmt::fg(fmt::color::white), "Target Type: {} \n",target.type == SMALL ? "SMALL" : "BIG");
    fmt::print(fmt::fg(fmt::color::orange_red), "Is Spinning: {} \n",is_target_spinning);
    fmt::print(fmt::fg(fmt::color::orange_red), "Is Switched: {} \n",is_target_switched);
#endif //PRINT_TARGET_INFO
#ifdef SAVE_AUTOAIM_LOG
    LOG(INFO) <<"[AUTOAIM] LATENCY: "<< "Crop: " << dr_crop_ms << " ms" << " Infer: " << dr_infer_ms << " ms" << " Predict: " << dr_predict_ms << " ms" << " Total: " << dr_full_ms << " ms";
    LOG(INFO) <<"[AUTOAIM] TARGET_INFO: "<< "Yaw: " << angle[0] << " Pitch: " << angle[1] << " Dist: " << (float)target.center3d_cam.norm()
                    << " Target: " << target.key << " Is Spinning: " << is_target_spinning<< " Is Switched: " << is_target_switched;
    LOG(INFO) <<"[AUTOAIM] PREDICTED: "<<"X: "<<aiming_point[0]<<" Y: "<<aiming_point[1]<<" Z: " << aiming_point[2];
#endif //SAVE_AUTOAIM_LOG

    //若预测出错取消本次数据发送
    if (isnan(angle[0]) || isnan(angle[1]))
    {
        LOG(ERROR)<<"NAN Detected! Data Transmit Aborted!";
        return false;
    }

    data = {(float)angle[1], (float)angle[0], (float)target.center3d_cam.norm(), is_target_switched, 1, is_target_spinning, 0};
    return true;
}