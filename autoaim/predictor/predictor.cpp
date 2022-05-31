#include "predictor.h"

#ifdef DRAW_PREDICT
int cnt = 0;
cv::Mat pic_x = cv::Mat::zeros(500, 2000, CV_8UC3);
cv::Mat pic_y = cv::Mat::zeros(500, 2000, CV_8UC3);
cv::Mat pic_z = cv::Mat::zeros(500, 2000, CV_8UC3);
#endif //DRAW_PREDICT

ArmorPredictor::ArmorPredictor()
{
}

ArmorPredictor::~ArmorPredictor()
{
}

ArmorPredictor ArmorPredictor::generate()
{
    ArmorPredictor new_predictor;
    new_predictor.pf.initParam(pf);
    new_predictor.fitting_disabled = false;

    return new_predictor;
}

bool ArmorPredictor::initParam(ArmorPredictor &predictor_loader)
{
    history_info.clear();
    pf.initParam(predictor_loader.pf);
    fitting_disabled = false;
    
    return true;
}

bool ArmorPredictor::initParam(string coord_path)
{
    YAML::Node config = YAML::LoadFile(coord_path);
    pf.initParam(config,"autoaim");
    fitting_disabled = false;
    
    return true;
}


Eigen::Vector3d ArmorPredictor::predict(Eigen::Vector3d xyz, int timestamp)
{
    auto t1=std::chrono::steady_clock::now();
    TargetInfo target = {xyz, (int)xyz.norm(), timestamp};
    
    //当队列长度小于3，仅更新队列
    if (history_info.size() < 3)
    {
        history_info.push_back(target);
        last_target = target;
        return xyz;
    }
    auto d_xyz = target.xyz - last_target.xyz;
    auto delta_t = timestamp - last_target.timestamp;
    auto last_dist = history_info.back().dist;
    // auto delta_time_estimate = (last_dist / bullet_speed) * 1e3 + delay;
    auto delta_time_estimate = (last_dist / bullet_speed) * 1e3 + delay;
    auto time_estimate = delta_time_estimate + history_info.back().timestamp - history_info.front().timestamp;
    //如速度过大,可认为为噪声干扰,进行滑窗滤波滤除
    // if (((d_xyz.norm() / delta_t) * 1e3) >= max_v)
    // {
    //     history_info.push_back(target);
    //     auto filtered_xyz = shiftWindowFilter(history_info.size() - window_size - 1);
    //     target = {filtered_xyz, (int)filtered_xyz.norm(), timestamp};
    //     history_info.pop_back();
    // }
    // auto filtered_xyz = shiftWindowFilter(history_info.size() - window_size - 1);
    // filtered_xyz << xyz[0], xyz[1], filtered_xyz[2];
    // target = {filtered_xyz, (int)filtered_xyz.norm(), timestamp};
    // history_info.pop_back();

    //当队列长度不足时不使用拟合
    if (history_info.size() < min_fitting_len)
    {
        history_info.push_back(target);
        fitting_disabled = true;
    }
    //当队列时间跨度过长时不使用拟合
    else if (target.timestamp - history_info.front().timestamp >= max_timespan)
    {
        history_info.pop_front();
        history_info.push_back(target);
        fitting_disabled = true;
    }
    else if (history_info.size() <= history_deque_len)
    {
        history_info.push_back(target);
        fitting_disabled = false;
        //若队列过长，移除首元素
        if (history_info.size() > history_deque_len)
            history_info.pop_front();
    }
    
#ifdef DISABLE_FITTING
    fitting_disabled = true;
#endif //DISABLE_FITTING

    Eigen::Vector3d result = {0, 0, 0};
    Eigen::Vector3d result_pf = {0, 0, 0};
    Eigen::Vector3d result_fitting = {0, 0, 0};
    PredictStatus is_pf_available;
    PredictStatus is_fitting_available;
    //需注意粒子滤波使用相对时间（自上一次检测时所经过ms数），拟合使用自首帧所经过时间
    if (fitting_disabled)
    {
        auto is_pf_available = predict_pf_run(target, result_pf, delta_time_estimate);
    }
    else
    {
        auto get_pf_available = std::async(std::launch::deferred, [=, &result_pf](){return predict_pf_run(target, result_pf, delta_time_estimate);});
        auto get_fitting_available = std::async(std::launch::deferred, [=, &result_fitting](){return predict_fitting_run(result_fitting, time_estimate);});

        is_pf_available = get_pf_available.get();
        is_fitting_available = get_fitting_available.get();
    }

    //进行融合
    if (is_fitting_available.xyz_status[0] && !fitting_disabled)
        result[0] = result_fitting[0];
    else
        result[0] = result_pf[0];

    if (is_fitting_available.xyz_status[1] && !fitting_disabled)
        result[1] = result_fitting[1];
    else
        result[1] = result_pf[1];

    if (is_fitting_available.xyz_status[2] && !fitting_disabled)
        result[2] = result_fitting[2];
    else
        result[2] = xyz[2];
    auto t2=std::chrono::steady_clock::now();
    double dr_ms=std::chrono::duration<double,std::milli>(t2-t1).count();
    // if(timestamp % 10 == 0)
    // cout<<dr_ms<<endl;

#ifdef DRAW_PREDICT
    if (cnt < 2000)
    {
        auto x = cnt * 5;
        cv::circle(pic_x,cv::Point2f((timestamp) / 10,xyz[0] * 100 + 200),1,cv::Scalar(0,0,255),1);
        cv::circle(pic_x,cv::Point2f((timestamp + delta_time_estimate) / 10,result_pf[0] * 100 + 200),1,cv::Scalar(0,255,0),1);
        cv::circle(pic_x,cv::Point2f((timestamp + delta_time_estimate) / 10,result_fitting[0] * 100 + 200),1,cv::Scalar(255,255,0),1);
        // cv::circle(pic_x,cv::Point2f((timestamp + delta_time_estimate) / 10,result[0]+ 200),1,cv::Scalar(255,255,255),1);


        cv::circle(pic_y,cv::Point2f((timestamp) / 10,xyz[1] * 100 + 200),1,cv::Scalar(0,0,255),1);
        cv::circle(pic_y,cv::Point2f((timestamp + delta_time_estimate) / 10,result_pf[1] * 100 + 200),1,cv::Scalar(0,255,0),1);
        cv::circle(pic_y,cv::Point2f((timestamp + delta_time_estimate) / 10,result_fitting[1] * 100 + 200),1,cv::Scalar(255,255,0),1);
        // cv::circle(pic_y,cv::Point2f((timestamp + delta_time_estimate) / 10,result[1]+ 200),1,cv::Scalar(255,255,255),1);

        cv::circle(pic_z,cv::Point2f((timestamp) / 10,xyz[2] * 100 + 200),1,cv::Scalar(0,0,255),1);
        cv::circle(pic_z,cv::Point2f((timestamp + delta_time_estimate) / 10,result_pf[2] * 100 + 200),1,cv::Scalar(0,255,0),1);
        cv::circle(pic_z,cv::Point2f((timestamp + delta_time_estimate) / 10,result_fitting[2] * 100),1,cv::Scalar(255,255,0),1);
        // cv::circle(pic_z,cv::Point2f((timestamp + delta_time_estimate) / 10,result[2]),1,cv::Scalar(255,255,255),1);
        cnt++;
    }
    cv::imshow("result_x",pic_x);
    cv::imshow("result_y",pic_y);
    cv::imshow("result_z",pic_z);
    cv::waitKey(1);
#endif //DRAW_PREDICT
    last_target = target;
    return result;
}


inline Eigen::Vector3d ArmorPredictor::shiftWindowFilter(int start_idx=0)
{
    //计算最大迭代次数
    auto max_iter = int(history_info.size() - start_idx) - window_size + 1;
    Eigen::Vector3d total_sum = {0, 0, 0};
    // cout<<history_info.size()<<endl;
    // cout<<max_iter<<endl;
    // cout<<start_idx<<endl;
    if (max_iter == 0 || start_idx < 0)
        return history_info.back().xyz;
    
    for (int i = 0; i < max_iter; i++)
    {
        Eigen::Vector3d sum = {0,0,0};
        for (int j = 0; j < window_size; j++)
            sum += history_info.at(start_idx + i + j).xyz;
        total_sum += sum / window_size;
    }
    // cout<<history_info.back().xyz<<endl;
    // cout<<total_sum / max_iter<<endl;
    // cout<<endl;
    return total_sum / max_iter;
}

/**
 * @brief 进行一次粒子滤波预测
 * @param target 本次预测目标信息
 * @param result 输出结果引用
 * @param time_estimated 本次预测所需的时间提前量
 * @return 预测结果状态
 * **/
ArmorPredictor::PredictStatus ArmorPredictor::predict_pf_run(TargetInfo target, Vector3d &result, int time_estimated)
{
    PredictStatus is_available;
    //使用线性运动模型
    Eigen::VectorXd measure_v (2);

    Eigen::Vector3d v_sum = {0, 0, 0};
    Eigen::Vector3d v_xyz = {0, 0, 0};
    //使用逐差法求解现在速度
    int max_iter = 2;
    int back_idx = history_info.size() - 1;

    for (int i = 0; i < max_iter; i++)
    {
        auto next = history_info[back_idx - i];
        auto prev = history_info[back_idx - i - 2];
        v_sum += (next.xyz - prev.xyz) / (next.timestamp - prev.timestamp) * 1e3; 
    }
    v_xyz = v_sum / max_iter;
    // cout<<v_xyz.norm()<<endl;

    measure_v << v_xyz[0], v_xyz[1];

    is_available.xyz_status[0] = pf.is_ready;
    is_available.xyz_status[1] = pf.is_ready;

    //Update
    pf.update(measure_vx)
    auto result_v = pf.predict();


    result << result_v[0], result_v[1], 0;
    result = result  * (time_estimated / 1000.f) + target.xyz;


    return is_available;
}

ArmorPredictor::PredictStatus ArmorPredictor::predict_fitting_run(Vector3d &result, int time_estimated)
{
    double params_x[4] = {1,1,1,0};            // 参数的估计值
    double params_y[4] = {1,1,1,0};            // 参数的估计值

    ceres::Problem problem_x;
    ceres::Problem problem_y;

    ceres::Solver::Options options_x;
    ceres::Solver::Options options_y;

    ceres::Solver::Summary summary_x;                // 优化信息
    ceres::Solver::Summary summary_y;                // 优化信息

    options_x.linear_solver_type = ceres::DENSE_QR;  // 增量方程如何求解
    options_y.linear_solver_type = ceres::DENSE_QR;  // 增量方程如何求解

    //求直流分量
    Eigen::Vector3d sum = {0,0,0};
    for (auto target_info : history_info)
    {
        sum += target_info.xyz;
    }
    auto dc = sum / history_info.size();
    params_x[0] = dc[0];
    params_y[0] = dc[1];
    
    for (auto target_info : history_info)
    {
        problem_x.AddResidualBlock (     // 向问题中添加误差项
        // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
            new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 3> ( 
                new CURVE_FITTING_COST ( target_info.timestamp - history_info.front().timestamp, target_info.xyz[0] - params_x[0])
            ),
            nullptr,            // 核函数，这里不使用，为空
            &params_x[1]                 // 待估计参数
        );
        problem_y.AddResidualBlock (     // 向问题中添加误差项
        // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
            new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 3> ( 
                new CURVE_FITTING_COST ( target_info.timestamp - history_info.front().timestamp, target_info.xyz[1] - params_y[0])
            ),
            nullptr,            // 核函数，这里不使用，为空
            &params_y[1]                 // 待估计参数
        );
    }

    //异步计算
    auto status_solve_x = std::async(std::launch::deferred, [&](){ceres::Solve(options_x, &problem_x, &summary_x);});
    auto status_solve_y = std::async(std::launch::deferred, [&](){ceres::Solve(options_y, &problem_y, &summary_y);});

    status_solve_x.wait();
    status_solve_y.wait();

    auto x_cost = summary_x.final_cost;
    auto y_cost = summary_y.final_cost;
    // cout<<x_cost<<endl;

    PredictStatus is_available;

    is_available.xyz_status[0] = (x_cost <= max_cost);
    is_available.xyz_status[1] = (y_cost <= max_cost);
    // cout<<z_cost<<endl;

    auto x_pred = params_x[0] + params_x[1] * cos(params_x[3] * time_estimated) + params_x[2] * sin(params_x[3] * time_estimated);
    auto y_pred = params_y[0] + params_y[1] * cos(params_y[3] * time_estimated) + params_y[2] * sin(params_y[3] * time_estimated);

    result = {x_pred,y_pred,dc[2]};
    return is_available;
}