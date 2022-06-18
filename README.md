# TUP-InfantryVision-2022
沈阳航空航天大学T-UP战队2022赛季步兵视觉识别程序
## Author:顾昊 赵梓合 
## Version: V1.1 Beta
## Env
http://www.fftw.org/download.html

https://github.com/fmtlib/fmt

https://github.com/google/glog/releases/tag/v0.5.0

https://github.com/ceres-solver/ceres-solver/releases/tag/2.1.0

https://github.com/lava/matplotlib-cpp

https://www.hikrobotics.com/cn/machinevision/service/download?module=0    #海康工业相机客户端MVS下载（linux）

sudo apt-get install libeigen3-dev

sudo ln -s /usr/include/eigen3/Eigen /usr/include/Eigen

### 更新内容
v1.5 修改roi裁减方式，裁减策略为随着丢帧次数逐渐扩大ROI，改变之前裁减固定大小的策略

v1.4 增加海康相机驱动

v1.3 修改WatchDDog.sh自启脚本

v1.2 添加异常机制，相机型号选择通过debug.h文件修改，大恒相机曝光时间等参数设置也通过debug.h文件修

V1.1 改进预测器,优化UI 2022.6.3

V1.0 合并更新debug.h，优化输出，添加日志记录，优化相机驱动，解耦串口，增加小陀螺判断，修复USB相机调用bug	

V0.7 Beta 改善预测功能（使用拟合与粒子滤波），增添ROI 2022.1.27  

V0.6 Beta 增加预测功能 2022.1.23  

V0.5 Beta 完成主程序、串口与自瞄进程. 2021.12.12  

V0.1 Beta 完成自瞄推理部分及粒子滤波器. 2021.11.12  


## 1.结构
    1.autoaim 自瞄
    2.camera 相机
    3.coordsolver 坐标解算及坐标系变换
    4.general 常用函数
    5.model 神经网络模型
    6.params 各类参数文件
    7.main.cpp 程序主文件
    8.README.md 说明文件

    
## 2.Use
    1.sudo cat /sys/kernel/debug/usb/devices

    2.sudo usermod -aG dialout your-username

    debug:

        ulimit -c

        ulimit -c unlimited

输入命令：sudo systemctl set-default multi-user.target 重启reboot

开启界面：sudo systemctl set-default graphical.target
