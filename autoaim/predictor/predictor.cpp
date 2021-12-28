#include "predictor.h"

Predictor::Predictor()
{
    YAML::Node config = YAML::LoadFile("/home/tup/Desktop/TUP-Vision-Infantry-2022/params/filter/filter_param.yaml");
    pf.initParam(config,"autoaim");
}

Predictor::~Predictor()
{
}

Eigen::Vector3d Predictor::predict(Eigen::Vector3d xyz)
{
    
    if (last_time.size() == 0)
    {
        // TODO:该处时间存储为暂时性的解决方案，正确的时间差计算方式应为取采图时间戳之差
        last_time.push_back(std::chrono::steady_clock::now());
        last_xyz = xyz;

        return xyz;
    }
    else if (last_time.size() < 2)
    {
        last_time.push_back(std::chrono::steady_clock::now());
        double delta_time = std::chrono::duration<double>
                            (last_time.back() - last_time.front()).count();

        last_v_xyz = (xyz - last_xyz) / delta_time;
        last_xyz = xyz;
        return xyz;
    }
    else

    {
        if (last_time.size() == 3)
            last_time.pop_front();
        last_time.push_back(std::chrono::steady_clock::now());

        // double delta_time = std::c    Armor Autoaim::chooseTarget(vector<Armor> &armors)
        // cout<<delta_time<<endl;
        // cout<<v_xyz<<endl;
        // cout<<xyz<<endl;
        // cout<<xyz.norm()<<endl;
        // cout<<"------"<<endl;

        last_xyz = xyz;
        // last_v_xyz = v_xyz;
        return xyz;
    }

}