#include "./serial/wt61pc.h"
#include "./thread/thread.h"
#include "./serial/serialport.h"

DaHengCamera DaHeng;
const string SERIAL_ID = "483/5740/200";
// const string SERIAL_ID = "1a86/7523/263";
// const string SERIAL_ID = "10c4/ea60/100";
const string SERIAL_ID_IMU = "1a86/7523/264";
const int BAUD = 115200;
const int BAUD_IMU = 460800;


int main()
{
    auto time_start = std::chrono::steady_clock::now();
    Factory<Image> autoaim_factory(3);
    Factory<VisionData> data_transmit_factory(5);
    MessageFilter<IMUData> data_receiver(20);
    SerialPort serial(SERIAL_ID, BAUD);
#ifdef USING_IMU_C_BOARD
    std::thread serial_watcher(&serialWatcher, ref(serial));
    std::thread receiver(&dataReceiver, ref(serial), ref(data_receiver), time_start);
#endif //USING_IMU_C_BOARD
#ifdef USING_IMU_WIT
    IMUSerial serial_imu(SERIAL_ID_IMU, BAUD_IMU);
    std::thread serial_watcher(&serialWatcher, ref(serial), ref (serial_imu));
    std::thread receiver(&dataReceiver, ref(serial_imu), ref(data_receiver), time_start);
#endif //USING_IMU_WIT
    std::thread autoaim_producer(&producer, ref(autoaim_factory), ref(data_receiver), time_start);
    std::thread autoaim_consumer(&consumer, ref(autoaim_factory),ref(data_transmit_factory));
    std::thread transmitter(&dataTransmitter, ref(serial), ref(data_transmit_factory));

#ifndef USING_IMU
    std::thread serial_watcher(&serialWatcher, ref(serial));
#endif //USING_IMU

    autoaim_producer.join();
    autoaim_consumer.join();
    serial_watcher.join();
    transmitter.join();
#ifdef USING_IMU
    receiver.join();
#endif //USING_IMU

    return 0;
}