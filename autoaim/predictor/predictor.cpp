#include "predictor.h"

Predictor::Predictor()
{
    // YAML::Node config = YAML::LoadFile("/home/tup/Desktop/TUP-Vision-Infantry-2022/params/filter/filter_param.yaml");
    // pf.initParam(config,"autoaim");
}

Predictor::~Predictor()
{
}

//TODO:支持多块装甲版同时预测
Eigen::Vector3d Predictor::predict(Eigen::Vector3d xyz, int timestamp)
{
    TargetInfo target = {xyz, (int)xyz.norm(), timestamp};
    
    if (history_info.size() < history_deque_len)
    {
        history_info.push_back(target);
        return xyz;
    }
    else if (target.timestamp - history_info.front().timestamp >= max_timespan)
    {
        history_info.empty();
        return xyz;
    }
    else if (history_info.size() == history_deque_len)
    {
        TargetInfo target = {xyz, (int)xyz.norm(), timestamp};
        history_info.pop_front();
        history_info.push_back(target);
    }

    double abc_x[3] = {0,0,0};            // abc参数的估计值
    double abc_y[3] = {0,0,0};            // abc参数的估计值
    double abc_z[3] = {0,0,0};            // abc参数的估计值

    ceres::Problem problem_x;
    ceres::Problem problem_y;
    ceres::Problem problem_z;

    ceres::Solver::Options options_x;
    ceres::Solver::Options options_y;
    ceres::Solver::Options options_z;

    ceres::Solver::Summary summary_x;                // 优化信息
    ceres::Solver::Summary summary_y;                // 优化信息
    ceres::Solver::Summary summary_z;                // 优化信息

    options_x.linear_solver_type = ceres::DENSE_QR;  // 增量方程如何求解
    options_y.linear_solver_type = ceres::DENSE_QR;  // 增量方程如何求解
    options_z.linear_solver_type = ceres::DENSE_QR;  // 增量方程如何求解

    for (auto target_info : history_info)
    {
        problem_x.AddResidualBlock (     // 向问题中添加误差项
        // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
            new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 3> ( 
                new CURVE_FITTING_COST ( target_info.timestamp - history_info.front().timestamp, target_info.xyz[0])
            ),
            nullptr,            // 核函数，这里不使用，为空
            abc_x                 // 待估计参数
        );
        problem_y.AddResidualBlock (     // 向问题中添加误差项
        // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
            new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 3> ( 
                new CURVE_FITTING_COST ( target_info.timestamp - history_info.front().timestamp, target_info.xyz[1])
            ),
            nullptr,            // 核函数，这里不使用，为空
            abc_y                 // 待估计参数
        );
        problem_z.AddResidualBlock (     // 向问题中添加误差项
        // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
            new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 3> ( 
                new CURVE_FITTING_COST ( target_info.timestamp - history_info.front().timestamp, target_info.xyz[2])
            ),
            nullptr,            // 核函数，这里不使用，为空
            abc_z                 // 待估计参数
        );
    }

    ceres::Solve(options_x, &problem_x, &summary_x);  // 开始优化
    ceres::Solve(options_y, &problem_y, &summary_y);  // 开始优化
    ceres::Solve(options_z, &problem_z, &summary_z);  // 开始优化

    auto last_dist = history_info.back().dist;

    auto delta_time_estimate = ((last_dist / 100.f) / bullet_speed) * 1000 + delay;
    auto time_estimate = delta_time_estimate + history_info.back().timestamp - history_info.front().timestamp;

    auto x_pred = 0.5 * abc_x[0] + abc_x[1] * cos(time_estimate) + abc_x[2] * sin(time_estimate);
    auto y_pred = 0.5 * abc_y[0] + abc_y[1] * cos(time_estimate) + abc_y[2] * sin(time_estimate);
    auto z_pred = 0.5 * abc_z[0] + abc_z[1] * cos(time_estimate) + abc_z[2] * sin(time_estimate);


    Vector3d result = {x_pred,y_pred,z_pred};

    return result;

    // if (last_time.size() == 0)
    // {
    //     last_time.push_back(std::chrono::steady_clock::now());
    //     last_xyz = xyz;

    //     return xyz;
    // }
    // else if (last_time.size() < 2)
    // {
    //     last_time.push_back(std::chrono::steady_clock::now());
    //     double delta_time = std::chrono::duration<double>
    //                         (last_time.back() - last_time.front()).count();

    //     last_v_xyz = (xyz - last_xyz) / delta_time;
    //     last_xyz = xyz;
    //     return xyz;
    // }
    // else

    // {
    //     if (last_time.size() == 3)
    //         last_time.pop_front();
    //     last_time.push_back(std::chrono::steady_clock::now());

        // double delta_time = std::c    Armor Autoaim::chooseTarget(vector<Armor> &armors)
        // cout<<delta_time<<endl;
        // cout<<v_xyz<<endl;
        // cout<<xyz<<endl;
        // cout<<xyz.norm()<<endl;
        // cout<<"------"<<endl;

        // last_xyz = xyz;
        // last_v_xyz = v_xyz;
        // return xyz;
    // }

}