#include <iostream>

#include <random>
#include <vector>
#include <ctime>

#include <yaml-cpp/yaml.h>
#include <Eigen/Core>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace Eigen;

class ParticleFilter
{
public:
    ParticleFilter(YAML::Node &config,const string param_name);
    ~ParticleFilter();

    Eigen::VectorXd predict();
    bool estimate(Eigen::VectorXd &measure);
    bool is_ready;
private:

    bool resample();

    int vector_len;
    int num_particle;

    Eigen::MatrixXd process_noise_cov;
    Eigen::MatrixXd measure_noise_cov;
    Eigen::MatrixXd weights;

    Eigen::MatrixXd matrix_estimate;
    Eigen::MatrixXd matrix_particle;
    Eigen::MatrixXd matrix_weights;

};