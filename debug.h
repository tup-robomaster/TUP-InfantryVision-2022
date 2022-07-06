//---------------------------------------------------------------------------//
//                          FIXME:临场调试选项
//---------------------------------------------------------------------------//

#define DETECT_RED  //红色检测，不启用默认为蓝色检测

#define IGNORE_ENGINEER             //是否忽略工程
#define IGNORE_NPC                  //是否忽略建筑物与哨兵

//FIXME: RECORD_DATA未完成
// #define SAVE_VIDEO               // Save video
// #define RECORD_DATA
#define SAVE_LOG_ALL                // Save log


// #TODO: 开启此选项请关闭无串口调试模式
// #define USING_IMU                //是否使用陀螺仪数据
//===========================================================================//

//TODO:确保比赛时下面的宏定义均为合理的注释状态
//---------------------------------------------------------------------------//
//                              硬件设备选项
//---------------------------------------------------------------------------//

// #define USING_DAHENG                //是否使用大恒摄像头，不启用默认为USB摄像头
#define USING_HIK                   //是否使用海康摄像头
// #define USING_VIDEO              // Using video
// #define USING_USB_CAMERA
// #define USE_SAVED_DATA

#define DEBUG_WITHOUT_COM           //无串口调试
//===========================================================================//v 3     


//---------------------------------------------------------------------------//
//                              图像DEBUG相关选项
//---------------------------------------------------------------------------//

// #define SHOW_INPUT      //显示送入神经网络的图像

#define SHOW_IMG        //是否显示检测Armor后的图像
//===========================================================================//+


//---------------------------------------------------------------------------//
//                              INFO及预测相关选项
//---------------------------------------------------------------------------//

#define USING_ROI   //Using roi

// #define PRINT_LATENCY //输出运行时

#define PRINT_TARGET_INFO //是否输出目标信息

// #define USING_PREDICT           //是否启用预测

#define USING_SPIN_DETECT //是否检测陀螺

// #define DISABLE_FITTING //是否禁用曲线拟合
//===========================================================================//


//---------------------------------------------------------------------------//
//                                  默认关闭选项                               //
//---------------------------------------------------------------------------//

// #define ASSIST_LABEL //辅助数据集标注

// #define DRAW_PREDICT //是否显示预测图像(使用时需使视野内仅存在一块装甲板)
// ===========================================================================//


//---------------------------------------------------------------------------//
//              下面的选项均为默认选项，不清楚的切勿修改
//---------------------------------------------------------------------------//

#ifdef SAVE_LOG_ALL
    #define SAVE_AUTOAIM_LOG
    #define SAVE_BUFF_LOG
    #define SAVE_TRANSMIT_LOG
    #define SAVE_IMU_LOG
    #define SAVE_MAIN_LOG
#endif // SAVE_LOG_ALL

#ifdef USING_IMU
    // #define USING_IMU_WIT   //是否使用维特陀螺仪 (默认不使用)
    #define USING_IMU_C_BOARD   //使用C板陀螺仪
#endif // USING_IMU


#ifdef SHOW_IMG
    #define SHOW_ALL_ARMOR  //是否绘制装甲板
    #define SHOW_ALL_FANS   //是否绘制所有扇叶
    #define SHOW_FPS    //是否显示FPS
    #define SHOW_PREDICT //是否显示预测
    #define SHOW_AIM_CROSS//是否绘制十字瞄准线
#endif // SHOW_IMG

#ifndef DETECT_RED
    #define DETECT_BLUE
#endif  //DETECT_RED
#ifdef DETECT_RED
    #define DETECT_BUFF_BLUE
#else
    #define DETECT_BUFF_RED
#endif
//===========================================================================//
