#include "wt61pc.h"

bool IMUSerial::readData()
{
    int bytes;
    char *name = ttyname(fd);
    if (name == NULL)printf("tty is null\n");
    //if (name != NULL)printf("device:%s\n",name);
    int result = ioctl(fd, FIONREAD, &bytes);
    if (result == -1)return false;


    int bytes_total = 44;
    int bytes_readed = 0;
    while (bytes_readed < bytes_total)
    {
        bytes = read(fd, rdata + bytes_readed, 1);
        bytes_readed+=bytes;
    }

    // cout<<"Bytes:"<<bytes_readed<<endl;
    return processData(bytes_readed);
}

bool IMUSerial::processData(int bytes)
{
    int frame_status = 0;
    int byte_num = 0;
    unsigned char check_sum = 0;
    unsigned char acc_data[8];
    unsigned char gryo_data[8];
    unsigned char quat_data[8];

    for (int i = 0; i < bytes; i++)
    {
        if (frame_status == 0)
        {
            if (rdata[i] == 0x55 && byte_num == 0)
            {
                check_sum = rdata[i];
                byte_num = 1;
                continue;
            }
            else if (rdata[i] == 0x51 && byte_num == 1)
            {
                check_sum += rdata[i];
                frame_status = 1;
                byte_num = 2;
                continue; 
            }
            else if (rdata[i] == 0x52 && byte_num == 1)
            {
                check_sum += rdata[i];
                frame_status = 2;
                byte_num = 2;
                continue; 
            }
            else if (rdata[i] == 0x59 && byte_num == 1)
            {
                check_sum += rdata[i];
                frame_status = 3;
                byte_num = 2;
                continue; 
            }
        }
        else if (frame_status == 1)
        {
            if (byte_num < 10)
            {
                acc_data[byte_num-2] = rdata[i];
                check_sum += rdata[i];
                byte_num += 1;
            }
            else
            {
                if (rdata[i] == (check_sum&0xff))
                {
                    getAcc(acc_data);
                    check_sum = 0;
                    byte_num = 0;
                    frame_status = 0;
                }
            }
        }
        else if (frame_status == 2)
        {
            if (byte_num < 10)
            {
                gryo_data[byte_num-2] = rdata[i];
                check_sum += rdata[i];
                byte_num += 1;
            }
            else
            {
                if (rdata[i] == (check_sum&0xff))
                {
                    getGyro(gryo_data);
                    check_sum = 0;
                    byte_num = 0;
                    frame_status = 0;
                }
            }
        }
        else if (frame_status == 3)
        {
            if (byte_num < 10)
            {
                quat_data[byte_num-2] = rdata[i];
                check_sum += rdata[i];
                byte_num += 1;
            }
            else
            {
                if (rdata[i] == (check_sum&0xff))
                {
                    getQuat(quat_data);
                    check_sum = 0;
                    byte_num = 0;
                    frame_status = 0;
                }
            }
        }
    }
    return true;
}

bool IMUSerial::getAcc(unsigned char acc_data[8])
{
    auto axl = acc_data[0];                                      
    auto axh = acc_data[1];
    auto ayl = acc_data[2];                                       
    auto ayh = acc_data[3];
    auto azl = acc_data[4];                                        
    auto azh = acc_data[5];

    auto k_acc = 16 * 9.8;

    auto acc_x = (axh << 8 | axl) / 32768.0 * k_acc;
    auto acc_y = (ayh << 8 | ayl) / 32768.0 * k_acc;
    auto acc_z = (azh << 8 | azl) / 32768.0 * k_acc;

    if (acc_x >= k_acc)
        acc_x -= 2 * k_acc;
    if (acc_y >= k_acc)
        acc_y -= 2 * k_acc;
    if (acc_z >= k_acc)
        acc_z-= 2 * k_acc;

    acc << acc_x, acc_y, acc_z;
    is_acc_initialized = true;
    return true;
}

bool IMUSerial::getGyro(unsigned char gyro_data[8])
{
    auto gxl = gyro_data[0];                                      
    auto gxh = gyro_data[1];
    auto gyl = gyro_data[2];                                       
    auto gyh = gyro_data[3];
    auto gzl = gyro_data[4];                                        
    auto gzh = gyro_data[5];

    auto k_gyro = 2000;

    auto gyro_x = (gxh << 8 | gxl) / 32768.0 * k_gyro;
    auto gyro_y = (gyh << 8 | gyl) / 32768.0 * k_gyro;
    auto gyro_z = (gzh << 8 | gzl) / 32768.0 * k_gyro;

    if (gyro_x >= k_gyro)
        gyro_x -= 2 * k_gyro;
    if (gyro_y >= k_gyro)
        gyro_y -= 2 * k_gyro;
    if (gyro_z >= k_gyro)
        gyro_z-= 2 * k_gyro;
        
    gyro << gyro_x, gyro_y, gyro_z;
    is_gyro_initialized = true;
    return true;
}

bool IMUSerial::getQuat(unsigned char quat_data[8])
{
    auto q0l = quat_data[0];                                      
    auto q0h = quat_data[1];
    auto q1l = quat_data[2];                                       
    auto q1h = quat_data[3];
    auto q2l = quat_data[4];                                        
    auto q2h = quat_data[5];
    auto q3l = quat_data[6];                                        
    auto q3h = quat_data[7];


    auto q0 = (q0h << 8 | q0l) / 32768.0;
    auto q1 = (q1h << 8 | q1l) / 32768.0;
    auto q2 = (q2h << 8 | q2l) / 32768.0;
    auto q3 = (q3h << 8 | q3l) / 32768.0;
    
    if (q0 >= 1)
        q0 -= 2;
    if (q1 >= 1)
        q1 -= 2;
    if (q2 >= 1)
        q2-= 2;
    if (q3 >= 1)
        q3-= 2;

    quat = {q0, q1, q2, q3};
    quat.normalize();
    is_quat_initialized = true;
    return true;
}