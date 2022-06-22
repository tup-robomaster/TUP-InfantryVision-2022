#pragma once

#include <atomic>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>

#include <iostream>
#include <vector>

#include "../serial/serialport.h"

using namespace std;

class IMUSerial : public SerialPort
{
public:
    using SerialPort::SerialPort;
    bool is_acc_initialized = false;
    bool is_gyro_initialized = false;
    bool is_quat_initialized = false;
    Eigen::Vector3d acc;
    Eigen::Vector3d gyro;
    Eigen::Quaterniond quat;
    bool readData();
    bool processData(int bytes);
    bool getAcc(unsigned char acc_data[8]);
    bool getGyro(unsigned char gyro_data[8]);
    bool getQuat(unsigned char quat_data[8]);
};