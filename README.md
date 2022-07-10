# TUP-InfantryVision-2022
沈阳航空航天大学T-UP战队2022赛季步兵视觉识别程序。

## Author:顾昊 赵梓合
## Version: V1.1 Beta
## 0.写在前面
本赛季的视觉程序在研发之处充分考虑到实验室的实际经费和其他资源可支配情况制定了“整体框架不变，小步快跑迭代”的

## 1.简介
本程序是一套适用于步兵、英雄、哨兵的通用代码。硬件上我们使用大恒MER-139相机和海康MV-016相机和USB-CDC虚拟串口。在大恒相机+8mm焦距镜头的测试条件下可以对0-7m的目标实现较为准确的识别（若进入手动ROI模式，手动缩小送入网络的区域，可以更进一步的提高识别极限距离至10m左右）。


功能：
- 自瞄：预测、反陀螺
- 吊射模式（手动ROI模式）
- 能量机关


部署前需要自行测量的量并更改的量有：
- 相机内参与畸变参数
- 相机与IMU间的变换矩阵$T_{ci}$和$T_{ic}$
- 陀螺仪到云台转轴间的平移向量$T_{iw}$
- 相机至枪管的Yaw,Pitch偏移量
- 相机至枪管的x,y,z轴偏移量

对每个模块更加具体的说明可以参考文件夹下的README.md文件:  
>[autoaim](autoaim/README.md)   
>[buff](buff/README.md)   
>[coordsolver](coordsolver/README.md)  
>[filter](filter/README.md)

## 2.环境配置

- Ubuntu 20.04 LTS
- GCC 9.3.0
- OpenVINO 2021.4
- OpenCV 4.5.0 安装方式：OpenVINO2021.4自带
- Eigen 安装方式：`sudo apt-get install libeigen3-dev`
- fmt 安装方式：[编译安装](https://github.com/fmtlib/fmt)
- Glog 安装方式：[编译安装](https://github.com/google/glog/releases/tag/v0.5.0)
- CeresSolver 安装方式：[编译安装](https://github.com/ceres-solver/ceres-solver/releases/tag/2.1.0)
- yaml-cpp 安装方式：[编译安装](https://github.com/jbeder/yaml-cpp)
- matplotlib-cpp 安装方式：[编译安装](https://github.com/lava/matplotlib-cpp)

用于解决Cmake提示找不到Eigen3的方案

    sudo ln -s /usr/include/eigen3/Eigen /usr/include/Eigen
## 3.文件结构
    1.autoaim 自瞄
    2.buff 能量机关
    3.camera 大恒相机驱动
    4.coordsolver 坐标解算及坐标系变换
    5.data 相机内录文件
    6.filter 粒子滤波器
    7.general 常用函数
    8.hik_camera 海康相机驱动
    9.log 日志文件
    10.model 神经网络模型
    11.params 各类参数文件
    12.record imu记录文件
    13.serial 串口通信
    14.thread 线程文件
    15.CMakeLists.txt cmake文件
    16.debug.h 调试宏定义控制
    17.main.cpp 程序主文件
    18.README.md 说明文件
    19.WatchDog.sh 自启脚本
    
## 4.其他
### 更新记录
V1.1 改进预测器,优化UI 2022.6.3

V1.0 合并更新debug.h，优化输出，添加日志记录，优化相机驱动，解耦串口，增加小陀螺判断，修复USB相机调用bug	

V0.7 Beta 改善预测功能（使用拟合与粒子滤波），增添ROI 2022.1.27  

V0.6 Beta 增加预测功能 2022.1.23  

V0.5 Beta 完成主程序、串口与自瞄进程. 2021.12.12  

V0.1 Beta 完成自瞄推理部分及粒子滤波器. 2021.11.12
### 使用方法
    1.sudo cat /sys/kernel/debug/usb/devices

    2.sudo usermod -aG dialout your-username

    debug:

        ulimit -c

        ulimit -c unlimited

输入命令：sudo systemctl set-default multi-user.target 重启reboot

开启界面：sudo systemctl set-default graphical.target
