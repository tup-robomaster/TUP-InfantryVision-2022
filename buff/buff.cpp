#include "buff.h"

using namespace cv;
using namespace std;

#ifdef ASSIST_LABEL
const string path_prefix = "../dataset/";
#endif //ASSIST_LABEL


Buff::Buff()
{
    detector.initModel(network_path);
    coordsolver.loadParam(camera_param_path,camera_name);
    lost_cnt = 0;
    is_last_target_exists = false;
    // input_size = {640,384};
    input_size = {640, 640};
    last_bullet_speed = 0;
    fmt::print(fmt::fg(fmt::color::pale_violet_red), "[BUFF] Buff init model success! Size: {} {}\n", input_size.height, input_size.width);

#ifdef SAVE_BUFF_LOG
    LOG(INFO)<<"[BUFF] Buff init model success! Size: "<<input_size.height<<" "<<input_size.width;
#endif //SAVE_BUFF_LOG
}

Buff::~Buff()
{
}

/**
 * @brief 根据上次装甲板位置截取ROI
 * 
 * @param img 所需处理的图像
 * @return ** Point2i ROI中心点
 */
#ifdef USING_ROI
Point2i Buff::cropImageByROI(Mat &img)
{
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

/*
 * @brief 选择击打目标
 * @param  
*/
bool Buff::chooseTarget(vector<Fan> &fans, Fan &target)
{
    float max_area = 0;
    int target_idx = 0;
    int target_fan_cnt = 0;
    for (auto fan : fans)
    {
        if (fan.id == 1)
        {
            target = fan;
            target_fan_cnt++;
        }
    }
    if (target_fan_cnt == 1)
        return true;
    else
        return false;
}

/**
 * @brief 自瞄主函数
 * 
 * @param src 图像、时间戳、IMU数据
 * @param data 发送数据所用结构体
 * @return true 识别成功
 * @return false 识别失败
 */
bool Buff::run(TaskData &src,VisionData &data)
{
    auto time_start=std::chrono::steady_clock::now();
    vector<BuffObject> objects;
    vector<Fan> fans;
    auto input = src.img;

#ifndef DEBUG_WITHOUT_COM
    if (src.bullet_speed > 10)
    {
        double bullet_speed;
        if (abs(src.bullet_speed - last_bullet_speed) < 0.5 || abs(src.bullet_speed - last_bullet_speed) > 1.5)
        {
            bullet_speed = src.bullet_speed;
            predictor.setBulletSpeed(bullet_speed);
            coordsolver.setBulletSpeed(bullet_speed);
            last_bullet_speed = bullet_speed;
        }
        
    }
    LOG(INFO)<<"SPD:"<<src.bullet_speed<<" : "<<last_bullet_speed;
#endif

#ifdef USING_IMU
    Eigen::Matrix3d rmat_imu = src.quat.toRotationMatrix();
#else
    Eigen::Matrix3d rmat_imu = Eigen::Matrix3d::Identity();
#endif //USING_IMU

//TODO:修复ROI
#ifdef USING_ROI
    roi_offset = cropImageByROI(input);
#endif  //USING_ROI
#ifdef ASSIST_LABEL
    auto img_name = path_prefix + to_string(src.timestamp) + ".jpg";
    imwrite(img_name,input);
#endif //ASSIST_LABEL
    auto time_crop=std::chrono::steady_clock::now();
    //若未检测到目标
    if (!detector.detect(input, objects))
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
        last_target_area = 0;
        data = {(float)0, (float)0, (float)0, 0, 0, 0, 1};
        LOG(WARNING) <<"[BUFF] No target detected!";
        return false;
    }
    auto time_infer = std::chrono::steady_clock::now();
    ///------------------------生成扇叶对象----------------------------------------------
    for (auto object : objects)
    {
#ifdef DETECT_BUFF_RED
        if (object.color != 1)
            continue;
#endif //DETECT_BUFF_RED
#ifdef DETECT_BUFF_BLUE
        if (object.color != 0)
            continue;
#endif //DETECT_BUFF_BLUE
        Fan fan;
        fan.id = object.cls;
        fan.color = object.color;
        fan.conf = object.prob;
        if (object.color == 0)
            fan.key = "B" + string(object.cls == 0 ? "Activated" : "Target");
        if (object.color == 1)
            fan.key = "R" + string(object.cls == 0 ? "Activated" : "Target");
        memcpy(fan.apex2d, object.apex, 5 * sizeof(cv::Point2f));
        for(int i = 0; i < 5; i++)
        {
            fan.apex2d[i] += Point2f((float)roi_offset.x,(float)roi_offset.y);
        }

        std::vector<Point2f> points_pic(fan.apex2d, fan.apex2d + 5);
        TargetType target_type = BUFF;
        auto pnp_result = coordsolver.pnp(points_pic, rmat_imu, target_type, SOLVEPNP_ITERATIVE);

        fan.armor3d_cam = pnp_result.armor_cam;
        fan.armor3d_world = pnp_result.armor_world;
        fan.centerR3d_cam = pnp_result.R_cam;
        fan.centerR3d_world = pnp_result.R_world;

        fan.euler = pnp_result.euler;
        fan.rmat = pnp_result.rmat;

        fans.push_back(fan);
    }
    ///------------------------生成/分配FanTracker----------------------------
    if (trackers.size() != 0)
    {
        //维护Tracker队列，删除过旧的Tracker
        for (auto iter = trackers.begin(); iter != trackers.end();)
        {
            //删除元素后迭代器会失效，需先行获取下一元素
            auto next = iter;
            // cout<<(*iter).second.last_timestamp<<"  "<<src.timestamp<<endl;
            if ((src.timestamp - (*iter).last_timestamp) > max_delta_t)
                next = trackers.erase(iter);
            else
                ++next;
            iter = next;
        }
    }
    //TODO:增加防抖
    std::vector<FanTracker> trackers_tmp;
    //为扇叶分配或新建最佳FanTracker
    for (auto fan = fans.begin(); fan != fans.end(); ++fan)
    {
        if (trackers.size() == 0)
        {
            FanTracker fan_tracker((*fan), src.timestamp);
            trackers_tmp.push_back(fan_tracker);
        }
        else
        {
            //1e9无实际意义，仅用于以非零初始化
            double min_v = 1e9;
            int min_last_delta_t = 1e9;
            bool is_best_candidate_exist = false;
            std::vector<FanTracker>::iterator best_candidate;
            for (auto iter = trackers.begin(); iter != trackers.end(); iter++)
            {
                double delta_t;
                Eigen::AngleAxisd angle_axisd;
                double rotate_speed;
                double sign;
                //----------------------------计算角度,求解转速----------------------------
                //若该扇叶完成初始化,且隔一帧时间较短
                if ((*iter).is_initialized && (src.timestamp - (*iter).prev_timestamp) < max_delta_t)
                {
                    delta_t = src.timestamp - (*iter).prev_timestamp;
                    //目前扇叶到上一次扇叶的旋转矩阵
                    auto relative_rmat = (*iter).prev_fan.rmat.transpose() * (*fan).rmat;
                    angle_axisd = Eigen::AngleAxisd(relative_rmat);
                    auto rotate_axis_world = (*iter).last_fan.rmat * angle_axisd.axis();
                    // auto rotate_axis_world = (*fan).rmat * angle_axisd.axis();
                    // auto rotate_axis_world = (*iter).last_fan.rmat  * angle_axisd.axis();
                    sign = ((*fan).centerR3d_world.dot(rotate_axis_world) > 0 ) ? 1 : -1;
                }
                else
                {
                    delta_t = src.timestamp - (*iter).last_timestamp;
                    //目前扇叶到上一次扇叶的旋转矩阵
                    auto relative_rmat = (*iter).last_fan.rmat.transpose() * (*fan).rmat;
                    //TODO:使用点乘判断旋转方向
                    angle_axisd = Eigen::AngleAxisd(relative_rmat);
                    auto rotate_axis_world = (*fan).rmat * angle_axisd.axis();
                    // auto rotate_axis_world = angle_axisd.axis();
//                     auto rotate_axis_world = (*iter).last_fan.rmat  * angle_axisd.axis();
                    sign = ((*fan).centerR3d_world.dot(rotate_axis_world) > 0 ) ? 1 : -1;
                }
                // cout<<sign<<endl;
                // cout<<delta_angle_axisd.angle()<< " : "<<delta_angle_axised<<endl;
                // cout<<delta_t<<endl;
                // cout<<"..."<<endl;
                // cout<<(*fan).centerR3d_world<<endl;
                // cout<<rotate_axis_world.normalized()<<(*fan).centerR3d_world.normalized()<<endl;
                // cout<<endl;
                rotate_speed = sign * (angle_axisd.angle()) / delta_t * 1e3;//计算角速度(rad/s)
                // cout<<angle_axisd.axis()<<endl;
                // cout<<en1dl;
                // cout<<rotate_speed<<endl;
                if (abs(rotate_speed) <= max_v && abs(rotate_speed) <= min_v && (src.timestamp - (*iter).last_timestamp) <= min_last_delta_t)
                {
                    min_last_delta_t = src.timestamp - (*iter).last_timestamp;
                    min_v = rotate_speed;
                    best_candidate = iter;
                    is_best_candidate_exist = true;
                }
                // if (fabs(rotate_speed) <= max_v)
                // {
                //     (*iter).update((*fan), src.timestamp);
                //     (*iter).rotate_speed = rotate_speed;
                //     break;
                // }
            }
            if (is_best_candidate_exist)
            {
                (*best_candidate).update((*fan), src.timestamp);
                (*best_candidate).rotate_speed = min_v;
            }
            else
            {
                FanTracker fan_tracker((*fan), src.timestamp);
                trackers_tmp.push_back(fan_tracker);
            }
        }
    }
    for (auto new_tracker : trackers_tmp)
    {
        trackers.push_back(new_tracker);
    }
    ///------------------------检测待激活扇叶是否存在----------------------------
    Fan target;
    bool is_target_exists = chooseTarget(fans, target);

    //若不存在待击打扇叶则返回false
    if (!is_target_exists)
    {
#ifdef SHOW_ALL_FANS
        for (auto fan : fans)
        {
            putText(src.img, fmt::format("{:.2f}", fan.conf),fan.apex2d[4],FONT_HERSHEY_SIMPLEX, 1, {0, 255, 0}, 2);
            if (fan.color == 0)
                putText(src.img, fmt::format("{}",fan.key), fan.apex2d[0], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
            if (fan.color == 1)
                putText(src.img, fmt::format("{}",fan.key), fan.apex2d[0], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
            for(int i = 0; i < 5; i++)
                line(src.img, fan.apex2d[i % 5], fan.apex2d[(i + 1) % 5], Scalar(0,255,0), 1);
            auto fan_armor_center = coordsolver.reproject(fan.armor3d_cam);
            circle(src.img, fan_armor_center, 4, {0, 0, 255}, 2);
        }
#endif //SHOW_ALL_FANS

#ifdef SHOW_AIM_CROSS
        line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), Scalar(0,255,0), 1);
        line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), Scalar(0,255,0), 1);
#endif //SHOW_FPS

#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
        lost_cnt++;
        is_last_target_exists = false;
        data = {(float)0, (float)0, (float)0, 0, 0, 0, 1};
        LOG(WARNING) <<"[BUFF] No available target fan or Multiple target fan detected!";
        return false;
    }

    int avail_tracker_cnt = 0;
    double rotate_speed_sum = 0;
    double mean_rotate_speed = 0;
    Eigen::Vector3d r_center_sum = {0, 0, 0};
    Eigen::Vector3d mean_r_center = {0, 0, 0};

    //计算平均转速与平均R字中心坐标
    for(auto tracker: trackers)
    {
        if (tracker.is_last_fan_exists && tracker.last_timestamp == src.timestamp)
        {
            rotate_speed_sum += tracker.rotate_speed;
            r_center_sum += tracker.last_fan.centerR3d_world;
            avail_tracker_cnt++;
        }
    }
    //若不存在可用的扇叶则返回false
    if (avail_tracker_cnt == 0)
    {
#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
        LOG(WARNING) <<"[BUFF] No available fan tracker exist!";
        data = {(float)0, (float)0, (float)0, 0, 0, 0, 1};
        lost_cnt++;
#endif //SHOW_IMG
        return false;
    }
    mean_rotate_speed = rotate_speed_sum / avail_tracker_cnt;
    mean_r_center = r_center_sum / avail_tracker_cnt;
    double theta_offset = 0;
    ///------------------------进行预测----------------------------
    // predictor.mode = 1;
    if (src.mode == 3)
        //进入小能量机关识别模式
        predictor.mode = 0;
    else if (src.mode == 4)
        //进入大能量机关识别模式
        predictor.mode = 1;
    // cout<<src.mode<<":"<<predictor.mode<<endl;
    // cout<<mean_rotate_speed<<endl;
    if (!predictor.predict(mean_rotate_speed, mean_r_center.norm(), src.timestamp, theta_offset))
    {
#ifdef SHOW_ALL_FANS
        for (auto fan : fans)
        {
            putText(src.img, fmt::format("{:.2f}", fan.conf),fan.apex2d[4],FONT_HERSHEY_SIMPLEX, 1, {0, 255, 0}, 2);
            if (fan.color == 0)
                putText(src.img, fmt::format("{}",fan.key), fan.apex2d[0], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
            if (fan.color == 1)
                putText(src.img, fmt::format("{}",fan.key), fan.apex2d[0], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
            for(int i = 0; i < 5; i++)
                line(src.img, fan.apex2d[i % 5], fan.apex2d[(i + 1) % 5], Scalar(0,255,0), 1);
            auto fan_armor_center = coordsolver.reproject(fan.armor3d_cam);
            circle(src.img, fan_armor_center, 4, {0, 0, 255}, 2);
        }
#endif //SHOW_ALL_FANS

#ifdef SHOW_AIM_CROSS
            line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), Scalar(0,255,0), 1);
            line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), Scalar(0,255,0), 1);
#endif //SHOW_FPS

#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
        LOG(WARNING) <<"[BUFF] Predictor is still progressing!";
        data = {(float)0, (float)0, (float)0, 0, 0, 0, 1};
        return false;
    }

    ///------------------------计算击打点----------------------------
    //将角度转化至[-PI,PI范围内]
    // cout<<theta_offset<<endl;
    // theta_offset = rangedAngleRad(theta_offset);
    // cout<<theta_offset<<endl;
    //由offset生成欧拉角和旋转矩阵
    Eigen::Vector3d hit_point_world = {sin(theta_offset) * fan_length, (cos(theta_offset) - 1) * fan_length,0};
    // cout<<hit_point_world<<endl;
    Eigen::Vector3d hit_point_cam = {0,0,0};
    // Eigen::Vector3d euler_rad = target.euler;
    // Eigen::Vector3d euler_rad = target.euler;
    //Pc = R * Pw + T
    hit_point_world = (target.rmat * hit_point_world) + target.armor3d_world;
    hit_point_cam = coordsolver.worldToCam(hit_point_world, rmat_imu);
    auto r_center_cam = coordsolver.worldToCam(target.centerR3d_world, rmat_imu);
    // auto r_center_cam = coordsolver.worldToCam(mean_r_center, rmat_imu);
    auto center2d_src = coordsolver.reproject(r_center_cam);
    auto target2d = coordsolver.reproject(hit_point_cam);

    auto angle = coordsolver.getAngle(hit_point_cam,rmat_imu);

    //-----------------判断扇叶是否发生切换-------------------------
    bool is_switched = false;
    auto delta_t = src.timestamp - last_timestamp;
    auto relative_rmat = last_fan.rmat.transpose() * target.rmat;
    //TODO:使用点乘判断旋转方向
    auto angle_axisd = Eigen::AngleAxisd(relative_rmat);
    // sign = ((*fan).centerR3d_world.dot(angle_axisd.axis()) > 0 ) ? 1 : -1;
    auto rotate_speed = (angle_axisd.angle()) / delta_t * 1e3;//计算角速度(rad/s)
    if (abs(rotate_speed) > max_v)
        is_switched = true;

    lost_cnt = 0;
    last_roi_center = center2d_src;
    last_timestamp = src.timestamp;
    last_fan = target;
    is_last_target_exists = true;

    //若预测出错取消本次数据发送
    if (isnan(angle[0]) || isnan(angle[1]))
    {
#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
        LOG(ERROR) <<"[BUFF] NAN Detected!";
        data = {(float)0, (float)0, (float)0, 0, 0, 0, 1};
#endif //SHOW_IMG
        return false;
    }
    
    auto time_predict = std::chrono::steady_clock::now();
    double dr_full_ms = std::chrono::duration<double,std::milli>(time_predict - time_start).count();
    double dr_crop_ms = std::chrono::duration<double,std::milli>(time_crop - time_start).count();
    double dr_infer_ms = std::chrono::duration<double,std::milli>(time_infer - time_crop).count();
    double dr_predict_ms = std::chrono::duration<double,std::milli>(time_predict - time_infer).count();

#ifdef SHOW_ALL_FANS
    for (auto fan : fans)
    {
        putText(src.img, fmt::format("{:.2f}", fan.conf),fan.apex2d[4],FONT_HERSHEY_SIMPLEX, 1, {0, 255, 0}, 2);
        if (fan.color == 0)
            putText(src.img, fmt::format("{}",fan.key), fan.apex2d[0], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        if (fan.color == 1)
            putText(src.img, fmt::format("{}",fan.key), fan.apex2d[0], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        for(int i = 0; i < 5; i++)
            line(src.img, fan.apex2d[i % 5], fan.apex2d[(i + 1) % 5], Scalar(0,255,0), 1);
        auto fan_armor_center = coordsolver.reproject(fan.armor3d_cam);
        circle(src.img, fan_armor_center, 4, {0, 0, 255}, 2);
    }
#endif //SHOW_ALL_FANS

#ifdef SHOW_AIM_CROSS
        line(src.img, Point2f(src.img.size().width / 2, 0), Point2f(src.img.size().width / 2, src.img.size().height), Scalar(0,255,0), 1);
        line(src.img, Point2f(0, src.img.size().height / 2), Point2f(src.img.size().width, src.img.size().height / 2), Scalar(0,255,0), 1);
#endif //SHOW_FPS

#ifdef SHOW_PREDICT
    circle(src.img, center2d_src, 5, Scalar(0, 0, 255), 2);
    circle(src.img, target2d, 5, Scalar(255, 255, 255), 2);
#endif //SHOW_PREDICT


#ifdef SHOW_FPS
    putText(src.img, fmt::format("FPS: {}",int(1000 / dr_full_ms)), {10, 25}, FONT_HERSHEY_SIMPLEX, 1, {0,255,0});
#endif //SHOW_FPS

#ifdef SHOW_IMG
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
    fmt::print(fmt::fg(fmt::color::green_yellow), "Dist: {} m\n",(float)hit_point_cam.norm());
    fmt::print(fmt::fg(fmt::color::orange_red), "Is Switched: {} \n",is_switched);
#endif //PRINT_TARGET_INFO

#ifdef SAVE_BUFF_LOG
    LOG(INFO) <<"[BUFF] LATENCY: "<< "Crop: " << dr_crop_ms << " ms" << " Infer: " << dr_infer_ms << " ms" << " Predict: " << dr_predict_ms << " ms" << " Total: " << dr_full_ms << " ms";
    LOG(INFO) <<"[BUFF] TARGET_INFO: "<< "Yaw: " << angle[0] << " Pitch: " << angle[1] << " Dist: " << (float)hit_point_cam.norm()<<"Is Switched:"<<is_switched;
#endif //SAVE_BUFF_LOG
    data = {(float)angle[1], (float)angle[0], (float)hit_point_cam.norm(), is_switched, 1, 1, 1};
    return true;
}
