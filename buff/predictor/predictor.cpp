#include "predictor.h"

BuffPredictor::BuffPredictor()
{
    is_params_confirmed = false;
    params[0] = 0;
    params[1] = 0; 
    params[2] = 0; 
    params[3] = 0; 
}

BuffPredictor::~BuffPredictor()
{
}

/**
 * @brief 预测
 * @param speed 旋转速度
 * @param dist 距离
 * @param timestamp 时间戳
 * @param result 结果输出
 * @return 是否预测成功
*/
bool BuffPredictor::predict(double speed, int dist, int timestamp, double &result)
{
    auto t1=std::chrono::steady_clock::now();
    TargetInfo target = {speed, dist, timestamp};
    //当时间跨度过长视作目标已更新，需清空历史信息队列
    if (history_info.size() == 0 || target.timestamp - history_info.front().timestamp >= max_timespan)
    {
        history_info.clear();
        history_info.push_back(target);
        last_target = target;
        is_params_confirmed = false;
        return false;
    }

    //输入数据前进行滤波
    auto delta_speed = target.speed - last_target.speed;
    auto delta_t = timestamp - last_target.timestamp;
    auto accel = 1e3 * delta_speed / delta_t;//计算加速度,单位rad/s^2
    if (abs(accel) > max_a || abs(speed) > max_v)
    {
        history_info.push_back(target);
        auto filtered_speed = shiftWindowFilter(history_info.size() - window_size - 1);
        history_info.pop_back();
        target = {filtered_speed, dist, timestamp};
    }

    if (history_info.size() < history_deque_len)    
    {
        history_info.push_back(target);
        last_target = target;
        return false;
    }
    else
    {
        history_info.pop_front();
        history_info.push_back(target);
    }

    //计算旋转方向
    double rotate_speed_sum = 0;
    int rotate_sign;
    for (auto target_info : history_info)
        rotate_speed_sum += target_info.speed;
    params[3] = rotate_speed_sum / history_info.size();
    // cout<<rotate_speed_sum<<endl;

    //TODO:小符模式不需要额外计算,也可增加判断，小符模式给定恒定转速进行击打
    //若为大符模式且函数未确定
    if (mode == 1)
    {
        //拟合函数: f(x) = a * sin(ω * t + θ)
        if (!is_params_confirmed)
        {
            ceres::Problem problem;
            ceres::Solver::Options options;
            ceres::Solver::Summary summary;       // 优化信息
            double params_fitting[3] = {1, 1, 1};

            //旋转方向，逆时针为正
            if (rotate_speed_sum / fabs(rotate_speed_sum) >= 0)
                rotate_sign = 1;
            else
                rotate_sign = -1;

            for (auto target_info : history_info)
            {
                problem.AddResidualBlock (     // 向问题中添加误差项
                // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
                    new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 3> ( 
                        new CURVE_FITTING_COST ((float)(target_info.timestamp) / 1e3,
                                                                    (target_info.speed - params[3]) * rotate_sign)
                    ),
                    new ceres::CauchyLoss(0.5),
                    params_fitting                 // 待估计参数
                );
            }

            //设置上下限
            problem.SetParameterLowerBound(params_fitting,0,0.5);
            problem.SetParameterUpperBound(params_fitting,0,2);
            problem.SetParameterLowerBound(params_fitting,1,1);
            problem.SetParameterUpperBound(params_fitting,1,3);
            problem.SetParameterLowerBound(params_fitting,2,-CV_PI);
            problem.SetParameterUpperBound(params_fitting,2,CV_PI);

            ceres::Solve(options, &problem, &summary);
            // cout<<"cost:"<<summary.final_cost<<endl;
            if (summary.final_cost < max_cost)
            {
                params[0] = params_fitting[0] * rotate_sign;
                params[1] = params_fitting[1];
                params[2] = params_fitting[2];
                is_params_confirmed = true;
                cout<<"Confirmed!"<<endl;
            }
            else
            {
                return false;
            }
        }
        else
        {            
            ceres::Problem problem;
            ceres::Solver::Options options;
            ceres::Solver::Summary summary;       // 优化信息
            double phase;

            for (auto target_info : history_info)
            {
                problem.AddResidualBlock(     // 向问题中添加误差项
                // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
                    new ceres::AutoDiffCostFunction<CURVE_FITTING_COST_PHASE, 1, 1> ( 
                        new CURVE_FITTING_COST_PHASE ((float)(target_info.timestamp) / 1e3,
                                                                    (target_info.speed - params[3]) * rotate_sign, params[0], params[1])
                    ),
                    new ceres::CauchyLoss(0.5),
                    &phase;                 // 待估计参数
                );
            }

            //设置上下限

            ceres::Solve(options, &problem, &summary);
            cout<<"cost:"<<summary.final_cost<<endl;
            if (summary.final_cost < max_cost)
            {
                params[2] = phase;
            }
            else
            {
                return false;
            }
        }
        // cout<<summary.BriefReport()<<endl;
    }

    // for (auto param : params)
    //     cout<<param<<" ";
    // cout<<"\n"<<"--------------------"<<endl;
    float delta_time_estimate = ((double)dist / bullet_speed) * 1e3 + delay;
    float timespan = history_info.back().timestamp - history_info.front().timestamp;
    float time_estimate = delta_time_estimate + timespan;
    result = calcAimingAngleOffset(params, timespan / 1e3, time_estimate / 1e3, mode);
    last_target = target;

#ifdef DRAW_PREDICT
    std::vector<double> plt_time;
    std::vector<double> plt_speed;
    std::vector<double> plt_fitted;
    for (auto target_info : history_info)
    {
        auto t = (float)(target_info.timestamp) / 1e3;
        plt_time.push_back(t);
        plt_speed.push_back(target_info.speed);
        plt_fitted.push_back(params[0] * sin (params[1] * t + params[2]) + params[3]);
    }
    plt::clf();
    plt::plot(plt_time, plt_speed,"bx");
    plt::plot(plt_time, plt_fitted,"r-");
    // plt::show();
    plt::pause(0.1);
#endif //DRAW_PREDICT
    return true;
}

/**
 * @brief 计算角度提前量
 * @param params 拟合方程参数
 * @param t0 积分下限
 * @param t1 积分上限
 * @param mode 模式
 * @return 角度提前量(rad)
*/
double BuffPredictor::calcAimingAngleOffset(double params[4], double t0, double t1 , int mode)
{
    auto a = params[0];
    auto omega = params[1];
    auto theta = params[2];
    auto b = params[3];
    double theta1;
    double theta0;

    //f(x) = a * sin(ω * t + θ) + b
    //对目标函数进行积分
    if (mode == 0)//适用于小符模式
    {
        theta0 = b * t0;
        theta1 = b * t1;
    }
    else
    {
        theta0 = (b * t0 - (a / omega) * cos(omega * t0 + theta));
        theta1 = (b * t1 - (a / omega) * cos(omega * t1 + theta));
    }

    return theta1 - theta0;
}

/**
 * @brief 滑窗滤波
 * 
 * @param start_idx 开始位置 
 * @return double 滤波结果
 */
inline double BuffPredictor::shiftWindowFilter(int start_idx=0)
{
    //TODO:修改传入参数，由start_idx改为max_iter
    //计算最大迭代次数
    auto max_iter = int(history_info.size() - start_idx) - window_size + 1;
    // cout<<max_iter<<endl;

    if (max_iter <= 0 || start_idx < 0)
        return history_info.back().speed;
    // cout<<start_idx<<":"<<history_info.at(start_idx).speed<<endl;
    // cout<<start_idx + 1<<":"<<history_info.at(start_idx + 1).speed<<endl;
    // cout<<start_idx + 2<<":"<<history_info.at(start_idx + 2).speed<<endl;
    // cout<<start_idx + 3<<":"<<history_info.at(start_idx + 3).speed<<endl;
    
    double total_sum = 0;
    // cout<<max_iter<<endl;
    for (int i = 0; i < max_iter; i++)
    {
        // cout<<"i:"<<i<<endl;
        double sum = 0;
        for (int j = 0; j < window_size; j++)
            sum += history_info.at(start_idx + i + j).speed;
        total_sum += sum / window_size;
        // cout<<total_sum<<endl;
    }
    return total_sum / max_iter;
}

/**
 * @brief 设置弹速
 * 
 * @param speed 传入弹速
 * @return true 
 * @return false 
 */
bool BuffPredictor::setBulletSpeed(double speed)
{
    bullet_speed = speed;
    return true;
}