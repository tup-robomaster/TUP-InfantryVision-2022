#include "particle_filter.h"

template<typename T>
bool initMatrix(Eigen::MatrixXd &matrix,std::vector<T> &vector);

template<typename T>
bool initMatrix(Eigen::MatrixXd &matrix,std::vector<T> &vector)
{
    int cnt = 0;
    for(int row = 0;row < matrix.rows();row++)
    {
        for(int col = 0;col < matrix.cols();col++)
        {
            matrix(row,col) = vector[cnt];
            cnt++;
        }
    }
    return true;
}

bool randomlizedGaussianColwise(Eigen::MatrixXd &matrix,Eigen::MatrixXd &cov)
{
    default_random_engine e(time(0));
    std::vector<normal_distribution<double>> normal_distribution_list;
    normal_distribution_list.empty();

    for(int i = 0;i<cov.cols();i++)
    {
        normal_distribution<double> n(0,cov(i,i));
        normal_distribution_list.push_back(n);
    }


    for(int col = 0;col < matrix.cols();col++)
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
    // cout<<"1";
    //初始化向量长度与粒子数
    vector_len = config[param_name]["vector_len"].as<int>();
    num_particle = config[param_name]["num_particle"].as<int>();

    Eigen::MatrixXd process_noise_cov_tmp(vector_len,vector_len);
    Eigen::MatrixXd measure_noise_cov_tmp(vector_len,vector_len);
    
    //初始化过程噪声矩阵
    auto read_vector = config[param_name]["process_noise"].as<vector<float>>();
    initMatrix(process_noise_cov_tmp,read_vector);
    process_noise_cov = process_noise_cov_tmp;

    //初始化状态噪声矩阵
    read_vector = config[param_name]["measure_noise"].as<vector<float>>();
    initMatrix(measure_noise_cov_tmp,read_vector);
    measure_noise_cov = measure_noise_cov_tmp;

    //初始化粒子矩阵及粒子权重
    srand((unsigned)time(NULL));
    // matrix_particle = Eigen::MatrixXd::Random(num_particle, vector_len) * process_noise_cov;
    matrix_particle = Eigen::MatrixXd::Zero(num_particle, vector_len);
    randomlizedGaussianColwise(matrix_particle, process_noise_cov);
    matrix_weights = Eigen::MatrixXd::Ones(num_particle, 1) / float(num_particle);
    is_ready = false;
}

ParticleFilter::~ParticleFilter()
{
}

Eigen::VectorXd ParticleFilter::predict()
{
    auto particles_weighted = matrix_particle.transpose() * matrix_weights;
    return Eigen::VectorXd(particles_weighted);
}

bool ParticleFilter::estimate(Eigen::VectorXd &measure)
{
    // cout<<measure<<endl;
    Eigen::MatrixXd gaussian = Eigen::MatrixXd::Zero(num_particle, vector_len);
    randomlizedGaussianColwise(gaussian, measure_noise_cov);

    Eigen::MatrixXd mat_measure = measure.replicate(1,num_particle).transpose() + gaussian;

    if (is_ready)
    {
        //序列重要性采样
        matrix_weights = (matrix_particle - mat_measure).rowwise().squaredNorm();
        matrix_weights = Eigen::MatrixXd::Ones(num_particle, 1).array() / matrix_weights.array();
        matrix_weights /= matrix_weights.sum();
        //重采样
        resample();
        return true;
    }
    else
    {
        matrix_particle += mat_measure;
        // cout<<matrix_particle<<endl;
        is_ready = true;
        return false;
    }

}

bool ParticleFilter::resample()
{
    srand((unsigned)time(NULL));
    auto matrix_weights_cumsum = matrix_weights;
    auto matrix_particle_tmp = matrix_particle;

    double cum_sum = 0;

    for(int i = 0;i < matrix_weights.rows();i++)
    {
        cum_sum = cum_sum + matrix_weights(i,0);
        matrix_weights_cumsum(i,0) = cum_sum;
    }

    auto n_eff = 1.0 / (matrix_weights.transpose() * matrix_weights).value();
    if (n_eff < num_particle / 2.0)
    {
        for(int i = 1;i < num_particle;i++)
        {
            auto selected = rand() / float(RAND_MAX);
            for(int j = 0;j < matrix_particle.rows();j++)
            {

                if (matrix_weights_cumsum(j) >= selected)
                {
                    matrix_particle_tmp.row(i) = matrix_particle.row(j);
                    break;
                }
            }
        }
        Eigen::MatrixXd gaussian = Eigen::MatrixXd::Zero(num_particle, vector_len);
        randomlizedGaussianColwise(gaussian, process_noise_cov);
        matrix_particle = matrix_particle_tmp + gaussian;
        matrix_weights = Eigen::MatrixXd::Ones(num_particle, 1) / float(num_particle);
    }
    return true;
}

int main()
{
    YAML::Node config = YAML::LoadFile("/home/rangeronmars/RM/TUP-Vision-Infantry-2022/params/filter/filter_param.yaml");
    ParticleFilter particle_filter(config,"autoaim");
    cv::Mat pic = cv::Mat::zeros(2000, 1000, CV_8UC3);
    double dr_ms;

    for(int i = 1;i < 200;i++)
    {
        auto t1=std::chrono::steady_clock::now();
        auto x = i;
        auto y = 100 * cos (2 * CV_PI * 0.01 *i) + 100 + 0.3 * i;
        // auto y = 500 + 10 * cos (2 * CV_PI * 0.01 *i);
        cv::circle(pic,cv::Point2f(x,y),1,cv::Scalar(0,255,0),1);
        auto tmp = particle_filter.predict();
        cv::circle(pic,cv::Point2f(tmp[0],tmp[1]),1,cv::Scalar(0,0,255),1);
        // particle_filter.predict();
        // cout<<tmp<<endl;
        Eigen::VectorXd measure(2);
        measure<<x, y;
        particle_filter.estimate(measure);
        auto t2=std::chrono::steady_clock::now();
        dr_ms=std::chrono::duration<double,std::milli>(t2-t1).count();
        cout<<"Time: "<<dr_ms<<"ms"<<endl;
    }
    // cv::imshow("result",pic);
    // cv::waitKey(0);
}