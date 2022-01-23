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

    auto t1=std::chrono::steady_clock::now();
    TargetInfo target = {xyz, (int)xyz.norm(), timestamp};
    
    if (history_info.size() < history_deque_len)
    {
        history_info.push_back(target);
        return xyz;
    }
    else if (target.timestamp - history_info.front().timestamp >= max_timespan)
    {
        cout<<"Invaild timespan:"<<target.timestamp - history_info.front().timestamp<<endl;
        history_info.pop_front();
        history_info.push_back(target);
        return xyz;
    }
    else if (history_info.size() == history_deque_len)
    {
        TargetInfo target = {xyz, (int)xyz.norm(), timestamp};
        history_info.pop_front();
        history_info.push_back(target);
    }

    double params_x[4] = {1,1,1,0};            // 参数的估计值
    double params_y[4] = {1,1,1,0};            // 参数的估计值
    double params_z[4] = {1,1,1,0};            // 参数的估计值

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
            new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 4> ( 
                new CURVE_FITTING_COST ( target_info.timestamp - history_info.front().timestamp, target_info.xyz[0])
            ),
            nullptr,            // 核函数，这里不使用，为空
            params_x                 // 待估计参数
        );
        problem_y.AddResidualBlock (     // 向问题中添加误差项
        // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
            new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 4> ( 
                new CURVE_FITTING_COST ( target_info.timestamp - history_info.front().timestamp, target_info.xyz[1])
            ),
            nullptr,            // 核函数，这里不使用，为空
            params_y                 // 待估计参数
        );
        problem_z.AddResidualBlock (     // 向问题中添加误差项
        // 使用自动求导，模板参数：误差类型，输出维度，输入维度，维数要与前面struct中一致
            new ceres::AutoDiffCostFunction<CURVE_FITTING_COST, 1, 4> ( 
                new CURVE_FITTING_COST ( target_info.timestamp - history_info.front().timestamp, target_info.xyz[2])
            ),
            nullptr,            // 核函数，这里不使用，为空
            params_z                 // 待估计参数
        );
    }

    //异步计算
    auto status_solve_x = std::async(std::launch::async, [&](){ceres::Solve(options_x, &problem_x, &summary_x);});
    auto status_solve_y = std::async(std::launch::async, [&](){ceres::Solve(options_y, &problem_y, &summary_y);});
    auto status_solve_z = std::async(std::launch::async, [&](){ceres::Solve(options_z, &problem_z, &summary_z);});

    auto last_dist = history_info.back().dist;
    auto delta_time_estimate = ((last_dist / 100.f) / bullet_speed) * 1000 + delay;
    auto time_estimate = delta_time_estimate + history_info.back().timestamp - history_info.front().timestamp;

    // cout<<"t"<<time_estimate<<endl;
    // for (auto target_info : history_info)
    // {
    //     cout<<target_info.timestamp<<" : "<<target_info.xyz[0]<<endl;
    // }

    status_solve_x.wait();
    status_solve_y.wait();
    status_solve_z.wait();

    // cout<<history_info.back().xyz<<endl;
    // cout<<"estimated a1,a2,b1,w = ";
    // for ( auto a:params_x ) cout<<a<<" ";
    // cout<<endl;

    // cout<<summary_x.BriefReport()<<endl;
    // cout<<summary_y.BriefReport()<<endl;
    // cout<<summary_z.BriefReport()<<endl;

    // auto t2=std::chrono::steady_clock::now();
    // double dr_ms=std::chrono::duration<double,std::milli>(t2-t1).count();
    // if(target.timestamp % 100 == 0)
    // cout<<dr_ms<<endl;

    Vector3d result = {0,0,0};

    auto x_pred = 1e2 * params_x[0] + 1e4 * params_x[1] * cos(params_x[3] * time_estimate) + 1e4 * params_x[2] * sin(params_x[3] * time_estimate);
    auto y_pred = 1e2 * params_y[0] + 1e4 * params_y[1] * cos(params_y[3] * time_estimate) + 1e4 * params_y[2] * sin(params_y[3] * time_estimate);
    auto z_pred = 1e2 * params_z[0] + 1e4 * params_z[1] * cos(params_z[3] * time_estimate) + 1e4 * params_z[2] * sin(params_z[3] * time_estimate);


    result = {x_pred,y_pred,z_pred};
    // cout<<result<<endl;
    // cout<<"---"<<endl;
    return result;
}

