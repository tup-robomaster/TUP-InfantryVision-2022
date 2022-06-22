#include "./serial/wt61pc.h"
#include "./thread/thread.h"
#include "./serial/serialport.h"
#include "./general/general.h"

#include "debug.h"

DaHengCamera DaHeng;
const string SERIAL_ID = "483/5740/200";
// const string SERIAL_ID = "1a86/7523/263";
// const string SERIAL_ID = "10c4/ea60/100";
// const string SERIAL_ID_IMU = "1a86/7523/264";
const int BAUD = 115200;
// const int BAUD_IMU = 460800;

int main(int argc,char* argv[])
{
#ifdef SAVE_MAIN_LOG
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = false;  //除了日志文件之外是否需要标准输出
    FLAGS_colorlogtostderr = true;  //是否启用不同颜色显示

    // 下面的函数暂时没有必要开启
    // FLAGS_logbufsecs = 0;   //设置可以缓冲日志的最大秒数，0指实时输出
    // FLAGS_max_log_size = 10;  //日志文件大小(单位：MB)
    // FLAGS_stop_logging_if_full_disk = true; //磁盘满时是否记录到磁盘

    google::SetLogDestination(google::GLOG_INFO,"../log/info/");  //设置日志级别
    google::SetLogDestination(google::GLOG_WARNING,"../log/warning/");
    google::SetLogDestination(google::GLOG_ERROR,"../log/error/");
#endif //SAVE_MAIN_LOG

    auto time_start = std::chrono::steady_clock::now();
    Factory<TaskData> task_factory(3);
    Factory<VisionData> data_transmit_factory(5);
    MessageFilter<MCUData> data_receiver(100);
    SerialPort serial(SERIAL_ID, BAUD);
    
#ifdef USING_IMU_C_BOARD
    std::thread serial_watcher(&serialWatcher, ref(serial));
    std::thread receiver(&dataReceiver, ref(serial), ref(data_receiver), time_start);
    #ifdef SAVE_MAIN_LOG
        LOG(INFO) << "[MAIN] serial_watcher(with IMU) start!"<<endl;
    #endif //SAVE_MAIN_LOG
#endif //USING_IMU_C_BOARD

// #ifdef USING_IMU_WIT
//     IMUSerial serial_imu(SERIAL_ID_IMU, BAUD_IMU);
//     std::thread serial_watcher(&serialWatcher, ref(serial), ref (serial_imu));
//     std::thread receiver(&dataReceiver, ref(serial_imu), ref(data_receiver), time_start);
// #endif //USING_IMU_WIT

    std::thread task_producer(&producer, ref(task_factory), ref(data_receiver), time_start);
#ifdef SAVE_MAIN_LOG
    LOG(INFO) << "[MAIN] task_producer start!";
#endif //SAVE_MAIN_LOG

    std::thread task_consumer(&consumer, ref(task_factory),ref(data_transmit_factory));
#ifdef SAVE_MAIN_LOG
        LOG(INFO) << "[MAIN] task_consumer start!";
#endif //SAVE_MAIN_LOG

    std::thread transmitter(&dataTransmitter, ref(serial), ref(data_transmit_factory));
#ifdef SAVE_MAIN_LOG
        LOG(INFO) << "[MAIN] transmitter start!";
#endif //SAVE_MAIN_LOG

#ifndef USING_IMU
    std::thread serial_watcher(&serialWatcher, ref(serial));
    #ifdef SAVE_MAIN_LOG
        LOG(INFO) << "[MAIN] serial_watcher(without IMU) start!";
    #endif //SAVE_MAIN_LOG
#endif //USING_IMU

    task_producer.join();
#ifdef SAVE_MAIN_LOG
    LOG(WARNING) << "[MAIN] task_producer end!";
#endif //SAVE_MAIN_LOG

    task_consumer.join();
#ifdef SAVE_MAIN_LOG
    LOG(WARNING) << "[MAIN] task_consumer end!";
#endif //SAVE_MAIN_LOG

    serial_watcher.join();
#ifdef SAVE_MAIN_LOG
    LOG(WARNING) << "[MAIN] serial_watcher end!";
#endif //SAVE_MAIN_LOG

    transmitter.join();
#ifdef SAVE_MAIN_LOG
    LOG(WARNING) << "[MAIN] transmitter end!";
#endif //SAVE_MAIN_LOG
 
#ifdef USING_IMU
    receiver.join();
    #ifdef SAVE_MAIN_LOG    
        LOG(WARNING) << "[MAIN] IMU receiver end!";
    #endif //SAVE_MAIN_LOG
#endif //USING_IMU

#ifdef SAVE_MAIN_LOG
    google::ShutdownGoogleLogging();
#endif //SAVE_MAIN_LOG

    return 0;
}