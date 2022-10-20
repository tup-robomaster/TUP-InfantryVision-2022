# TUP-InfantryVision-2022
沈阳航空航天大学T-UP战队2022赛季步兵视觉识别程序.
## Author:顾昊 赵梓合
## Version: V1.1 Beta
***Per Aspera Ad Astra.***    
<img src="docs/tup_badge_vision.png" width="200px">

## 0.写在前面
本赛季的视觉程序在研发之处充分考虑到实验室的实际经费和其他资源可支配情况制定了“整体框架不变，小步快跑迭代”的研发思路，并重新设定了本赛季程序的整体框架。程序整体设计目的在于实现稳定可靠的功能的基础上将各模块之间尽可能充分解耦，做到程序使用简单、易于阅读及二次开发。

## 1.简介
本程序是一套适用于步兵、英雄、哨兵的通用代码。硬件上我们使用大恒MER-139相机和海康MV-016相机和USB-CDC虚拟串口。在大恒相机+8mm焦距镜头的测试条件下可以对0-7m的目标实现较为准确的识别（若进入手动ROI模式，手动缩小送入网络的区域，可以更进一步的提高识别极限距离至10m左右）。


[打击能量机关效果](https://www.bilibili.com/video/BV1eT411377v/?spm_id_from=333.999.0.0&vd_source=b9a154f3c3bbc6442d61d52cc78af3d2)


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
- OpenCV 4.5.0 安装方式：OpenVINO2021.4自带(注意: OpenCV4.5.0修改了minAreaRect的角度计算方式,如执意使用更早的版本请自行修改程序相关部分!!!)
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
## 5.致谢(排名不分先后)
  在本套程序的开发过程中,因为经验不足,我们遇到了很多棘手的问题,在这里特别感谢来自上海交通大学云汉交龙战队,哈尔滨工业大学(深圳)南工骁鹰战队,哈尔滨工业大学(威海)HERO战队,哈尔滨工业大学I Hiter战队,南京工业大学Bluespace战队,华北理工大学Horizon战队的成员与离退休老干部的慷慨相助与分享,如果没有你们的帮助,本套程序的诞生定会变得无比困难.
## 6.私货
[神经网络训练仓库传送门](https://github.com/RangerOnMars/TUP-NN-Train) 本赛季的神经网络训练仓库(因为贪图省事,改的比较不雅观,就不放战队仓库给战队抹黑了:( )
