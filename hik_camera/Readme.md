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

## 注意事项
	海康相机需要一定启动时间，感觉在5s左右，因此刚插线相机线要等待片刻（相机屁股后面的指示灯亮了就能使用了）。
	海康相机的各个参数设置接口均有记忆功能，如何修改没看到变化可就是修改失败，参数还是上次的保留值。部分参数设置接口例如gamma校正、色彩校正等，虽然二次开发了，但是并没有实测过，可能会出现设置失败的情况，故在此提醒。




	


