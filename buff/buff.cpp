#include "buff.h"

using namespace cv;
using namespace std;

Buff::Buff()
{
    detector.initModel(network_path);
    coordsolver.loadParam(camera_param_path,"KE0200110076");
    lost_cnt = 0;
    is_last_target_exists = false;
    // input_size = {640,384};
    input_size = {416,416};
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

#ifdef USING_IMU
    Eigen::Matrix3d rmat_imu = src.quat.toRotationMatrix();
#else
    Eigen::Matrix3d rmat_imu = Eigen::Matrix3d::Identity();
#endif //USING_IMU

//TODO:修复ROI
#ifdef USING_ROI
    roi_offset = cropImageByROI(input);
#endif  //USING_ROI

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
        auto pnp_result = coordsolver.pnp(points_pic, rmat_imu, SOLVEPNP_EPNP);
        fan.centerR2d = fan.apex2d[2];

        fan.armor3d_cam = pnp_result.armor_cam;
        fan.armor3d_world = pnp_result.armor_world;
        fan.centerR3d_cam = pnp_result.R_cam;
        fan.centerR3d_world = pnp_result.R_world;
        fan.euler = pnp_result.euler;

        fans.push_back(fan);
    }
    ///------------------------生成/分配FanTracker----------------------------
    //TODO:增加防抖
    std::vector<FanTracker> trackers_tmp;
    //为扇叶分配或新建最佳FanTracker
    for (auto fan = fans.begin(); fan != fans.end(); ++fan)
    {
        if (trackers.size() == 0)
        {
            FanTracker fan_tracker;
            fan_tracker.last_fan = (*fan);
            fan_tracker.last_timestamp = src.timestamp;
            fan_tracker.is_last_fan_exists = false;//初始化
            fan_tracker.rotate_speed = 0;
            trackers_tmp.push_back(fan_tracker);
        }
        else
        {
            for (auto iter = trackers.begin(); iter != trackers.end(); iter++)
            {
                double delta_t = src.timestamp - (*iter).last_timestamp;
                double delta_theta;
                auto current_roll = (*fan).euler[0];
                auto last_roll = (*iter).last_fan.euler[0];
                
                //TODO:使用轴角所测得的旋转角度存在一定问题，噪声较大，未排查
                auto delta_angle_axised = eulerToAngleAxisd((*fan).euler - (*iter).last_fan.euler);
                double rotate_speed = delta_angle_axised.angle() / delta_t * 1e3;//计算角速度(rad/s)
                // cout<<delta_angle_axised.angle<<endl;
                //将Roll表示范围由[-180,180]转换至[0，360]
                // if (current_roll <= 0)
                //     current_roll += CV_2PI;
                // if (last_roll <= 0)
                //     last_roll += CV_2PI;
                // //计算delta_theta使用
                // if ((*fan).euler[0] > 0 && (*fan).euler[0] < (0.5 * CV_PI) && (*iter).last_fan.euler[0] > (1.5 * CV_PI))
                //     delta_theta = CV_2PI + (*fan).euler[0] - (*iter).last_fan.euler[0];
                // else if ((*fan).euler[0] > (1.5 * CV_PI) && (*iter).last_fan.euler[0] > 0 && (*iter).last_fan.euler[0] < (0.5 * CV_PI))
                //     delta_theta = -CV_2PI + (*fan).euler[0] - (*iter).last_fan.euler[0];
                // else
                //     delta_theta = (*fan).euler[0] - (*iter).last_fan.euler[0];
                // double rotate_speed = delta_theta / delta_t * 1e3;//计算角速度(rad/s)
                if (fabs(rotate_speed) <= max_v)
                {
                    FanTracker fan_tracker = (*iter);
                    fan_tracker.last_fan = (*fan);
                    fan_tracker.last_timestamp = src.timestamp;
                    fan_tracker.is_last_fan_exists = true;
                    fan_tracker.rotate_speed = rotate_speed;
                    trackers_tmp.push_back(fan_tracker);
                    break;
                }
                else if (trackers.size() < fans.size())
                {
                    FanTracker fan_tracker;
                    fan_tracker.last_fan = (*fan);
                    fan_tracker.last_timestamp = src.timestamp;
                    fan_tracker.is_last_fan_exists = false;//初始化
                    fan_tracker.rotate_speed = 0;
                    trackers_tmp.push_back(fan_tracker);
                    break;
                }
            }
        }
    }
    trackers = trackers_tmp;
    ///------------------------检测待激活扇叶是否存在----------------------------
    Fan target;
    bool is_target_exists = chooseTarget(fans, target);
    if (!is_target_exists)
    {
#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
        lost_cnt++;
        is_last_target_exists = false;
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
        if (tracker.is_last_fan_exists)
        {
            rotate_speed_sum += tracker.rotate_speed;
            r_center_sum += tracker.last_fan.centerR3d_world;
            avail_tracker_cnt++;
        }
    }
    if (avail_tracker_cnt == 0)
    {
#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
        return false;
    }
    mean_rotate_speed = rotate_speed_sum / avail_tracker_cnt;
    mean_r_center = r_center_sum / avail_tracker_cnt;
    double theta_offset;
    //FIXME:加入模式切换
    ///------------------------进行预测----------------------------
    // predictor.mode = 1;
    if (src.mode == 0x03)
        predictor.mode = 0;
    else if (src.mode == 0x04)
        predictor.mode == 1;
    if (!predictor.predict(mean_rotate_speed, int(mean_r_center.norm()), src.timestamp, theta_offset))
    {
#ifdef SHOW_IMG
        imshow("dst",src.img);
        waitKey(1);
#endif //SHOW_IMG
        return false;
    }
    ///------------------------计算击打点----------------------------
    //将角度转化至[-PI,PI范围内]
    theta_offset = rangedAngleRad(theta_offset);
    // cout<<theta_offset<<endl;
    //由offset生成欧拉角和旋转矩阵
    Eigen::Vector3d hit_point_world = {sin(theta_offset) * fan_length, (cos(theta_offset) - 1) * fan_length,0};
    // cout<<hit_point_world<<endl;
    Eigen::Vector3d hit_point_cam = {0,0,0};
    // Eigen::Vector3d euler_rad = target.euler;
    Eigen::Vector3d euler_rad = target.euler;
    auto rotMat = eulerToRotationMatrix(euler_rad);
    //Pc = R * Pw + T
    hit_point_world = (rotMat * hit_point_world) + target.armor3d_world;
    hit_point_cam = coordsolver.worldToCam(hit_point_world, rmat_imu);
    auto r_center_cam = coordsolver.worldToCam(target.centerR3d_cam, rmat_imu);
    // auto r_center_cam = coordsolver.worldToCam(mean_r_center, rmat_imu);
    auto center2d_src = coordsolver.reproject(r_center_cam);
    auto target2d = coordsolver.reproject(hit_point_cam);
    lost_cnt = 0;
    last_roi_center = center2d_src;
    is_last_target_exists = true;

#ifdef SHOW_ALL_FANS
    for (auto fan : fans)
    {
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
    circle(src.img, target2d, 5, Scalar(0, 255, 0), 2);
#endif //SHOW_PREDICT

    auto angle = coordsolver.getAngle(hit_point_cam,rmat_imu);
    auto time_predict = std::chrono::steady_clock::now();
    double dr_full_ms = std::chrono::duration<double,std::milli>(time_predict - time_start).count();

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
        double dr_crop_ms = std::chrono::duration<double,std::milli>(time_crop - time_start).count();
        double dr_infer_ms = std::chrono::duration<double,std::milli>(time_infer - time_crop).count();
        double dr_predict_ms = std::chrono::duration<double,std::milli>(time_predict - time_infer).count();
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
    fmt::print(fmt::fg(fmt::color::green_yellow), "Dist: {} m\n",(float)target.armor3d_cam.norm());
#endif //PRINT_TARGET_INFO

#ifdef SAVE_BUFF_LOG
    // LOG(INFO) <<"[BUFF] LATENCY: "<< "Crop: " << dr_crop_ms << " ms" << " Infer: " << dr_infer_ms << " ms" << " Predict: " << dr_predict_ms << " ms" << " Total: " << dr_full_ms << " ms";
    // LOG(INFO) <<"[BUFF] TARGET_INFO: "<< "Yaw: " << angle[0] << " Pitch: " << angle[1] << " Dist: " << (float)target.center3d_cam.norm();
#endif //SAVE_BUFF_LOG

    data = {(float)angle[1], (float)angle[0], (float)target.armor3d_cam.norm(), 0, 1, 1, 1};
    return true;
}
