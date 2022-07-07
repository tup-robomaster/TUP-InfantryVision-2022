# Copyright(C),2018-2022,沈阳航空航天大学T-UP战队 All Rights Reserved
海康相机驱动程序  
## Author:  刘彪
  
## Version: 4.1.1

## Env
https://www.hikrobotics.com/cn/machinevision/service/download?module=0    #相机客户端MVS下载（linux）

https://github.com/fmtlib/fmt					

## 更新内容

## 结构
1.include 海康相机驱动头文件夹

2.HaiKangCamera.cpp 海康相机驱动源文件

## 注意事项
	海康相机启动需要一定时间，感觉在5s左右，因此刚插线相机线要等待片刻（相机屁股后面的指示灯亮了就能正常使用）。
	海康相机的各个参数设置接口均有记忆功能，如何修改没看到变化可就是修改失败，参数还是上次的保留值。部分参数设置接口例如gamma校正、色彩校正等，虽然二次开发了，但是并没有实测过，可能会出现设置失败的情况，故在此提醒。
	实测海康相机发热情况较为严重，时间过久温度过高可能会触发保护机制造成相机离线。

## 使用步骤
	1.前往HIKROBOT官网找到linux版本的相机开发库（USB3.0）,下载后进行安装;
	2.由于使用到了fmt库，也需要安装，链接在上方；
	3.使用时直接创建海康相机对象，然后调用对象成员函数进行图像抓取和参数设置，设置成功即可正常使用(样例可见thread/thrad.cpp文件)。
	




	


