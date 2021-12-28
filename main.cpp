
#include "./thread/thread.h"
#include "./serial/serialport.h"



DaHengCamera DaHeng;

int main()
{
    Factory<cv::Mat> autoaim_factory(3);
    Factory<VisionData> data_transmit_factory(5);
    SerialPort serial;

    std::thread serial_watcher(&serialWatcher, ref(serial));
    std::thread autoaim_producer(&producer, ref(autoaim_factory));
    std::thread autoaim_consumer(&consumer, ref(autoaim_factory),ref(data_transmit_factory));
    std::thread transmitter(&dataTransmitter, ref(serial),ref(data_transmit_factory));
    

    autoaim_producer.join();
    autoaim_consumer.join();
    serial_watcher.join();
    transmitter.join();

    return 0;
}