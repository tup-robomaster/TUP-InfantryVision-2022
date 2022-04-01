#include "particle_filter.h"


bool randomlizedGaussianColwise(Eigen::MatrixXd &matrix,Eigen::MatrixXd &cov)
{
    std::random_device rd;
    default_random_engine e(rd());
    std::vector<normal_distribution<double>> normal_distribution_list;

    for (int i = 0; i<cov.cols(); i++)
    {
        normal_distribution<double> n(0,cov(i,i));
        normal_distribution_list.push_back(n);
    }


    for (int col = 0; col < matrix.cols(); col++)
    {
        // cout<<normal_distribution_list[col](e)<<endl;
        for(int row = 0;row < matrix.rows();row++)
        {
            auto tmp = normal_distribution_list[col](e);
            matrix(row,col) = tmp;
            // matrix(row,col) = 1;
        }
    }

    return true;
}

ParticleFilter::ParticleFilter(YAML::Node &config,const string param_name)
{
    initParam(config,param_name);
}

bool ParticleFilter::initParam(YAML::Node &config,const string param_name)
{
    //初始化向量长度与粒子数
    vector_len = config[param_name]["vector_len"].as<int>();
    num_particle = config[param_name]["num_particle"].as<int>();
    Eigen::MatrixXd process_noise_cov_tmp(vector_len,vector_len);
    
    //初始化过程噪声矩阵
    auto read_vector = config[param_name]["process_noise"].as<vector<float>>();
    initMatrix(process_noise_cov_tmp,read_vector);
    process_noise_cov = process_noise_cov_tmp;

    //初始化粒子矩阵及粒子权重
    matrix_particle = Eigen::MatrixXd::Zero(num_particle, vector_len);
    randomlizedGaussianColwise(matrix_particle, process_noise_cov);
    matrix_weights = Eigen::MatrixXd::Ones(num_particle, 1) / float(num_particle);
    is_ready = false;
    
    return true;
}

ParticleFilter::ParticleFilter()
{
}

ParticleFilter::~ParticleFilter()
{
}

bool ParticleFilter::initParam(ParticleFilter parent)
{
    vector_len = parent.vector_len;
    num_particle = parent.num_particle;
    process_noise_cov = parent.process_noise_cov;
    matrix_particle = parent.matrix_particle;
    matrix_weights = parent.matrix_weights;
    is_ready = false;

    return true;
}

Eigen::VectorXd ParticleFilter::predict()
{
    Eigen::VectorXd particles_weighted = matrix_particle.transpose() * matrix_weights;
    return particles_weighted;
}

bool ParticleFilter::correct(Eigen::VectorXd measure)
{
    Eigen::MatrixXd gaussian = Eigen::MatrixXd::Zero(num_particle, vector_len);
    Eigen::MatrixXd mat_measure = measure.replicate(1,num_particle).transpose();

    if (is_ready)
    {
    //     auto mean_weights = (matrix_particle - mat_measure).mean();
        //序列重要性采样
        // matrix_weights = (matrix_particle - mat_measure).rowwise().squaredNorm();
        matrix_weights = (matrix_particle - mat_measure).rowwise().norm();
        // matrix_weights = matrix_weights + 10 * Eigen::MatrixXd::Ones(num_particle, 1);//滤除高频噪声
        // matrix_weights = (matrix_particle - mat_measure).rowwise().;
        matrix_weights = Eigen::MatrixXd::Ones(num_particle, 1).array() / matrix_weights.array();
        matrix_weights /= matrix_weights.sum();
        //重采样
        resample();

        return true;
    }
    else
    {
        matrix_particle += mat_measure;
        is_ready = true;
        return false;
    }

}

bool ParticleFilter::resample()
{    
    
    //重采样采用低方差采样,复杂度为O(N),较轮盘法的O(NlogN)更小,实现可参考<Probablistic Robotics>
    std::random_device rd;
    default_random_engine e(rd());
    std::uniform_real_distribution<> random {0.0, 1.f / num_particle};

    int i = 0;
    double c = matrix_weights(0,0);
    auto r = random(e);
    Eigen::MatrixXd matrix_particle_tmp = matrix_particle;

    for (int m = 1; m <= num_particle; m++)
    {
        auto u = r + (m - 1) * (1.f / num_particle);
        // 当 u > c 不进行采样
        while (u > c)
        {
            i++;
            c = c + matrix_weights(i,0);
        }
        matrix_particle_tmp.row(m - 1) = matrix_particle.row(i);
    }
    Eigen::MatrixXd gaussian = Eigen::MatrixXd::Zero(num_particle, vector_len);
    randomlizedGaussianColwise(gaussian, process_noise_cov);
    matrix_particle = matrix_particle_tmp + gaussian;
    matrix_weights = Eigen::MatrixXd::Ones(num_particle, 1) / float(num_particle);
    return true;
}
