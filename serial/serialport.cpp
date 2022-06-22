#include "serialport.h"

SerialPort::SerialPort(const string ID, const int BAUD)
{
    serial_id = ID;
    baud = BAUD;
    // if(not initSerialPort()){
    //     exit(0);
    // };
    // system(std::string("echo root@233|sudo chmod 777 /dev/ttyUSB0").c_str());
    // cout << system(std::string("pwd").c_str()) << endl;
    // system(std::string("root@233").c_str());
#ifdef DEBUG_WITHOUT_COM
    withoutSerialPort();
#else
    initSerialPort();
#endif //DEBUG_WITHOUT_COM
}

/**
 * @brief 返回所有可用串口
 * **/
std::vector<Device> SerialPort::listPorts()
{
    vector<Device> devices;
    for(auto port_dir : DEFAULT_PORT)
    {
        std::vector<string> availible_path;
        auto general_path = "/sys/class/tty/"+ port_dir;
        for(int i = 0; i < MAX_ITER;i++)
        {
            auto tty_dir_path = general_path + to_string(i);

            if (access(tty_dir_path.c_str(),F_OK) != -1)
            {
                Device dev;
                availible_path.push_back(tty_dir_path);
                auto real_path = symbolicToReal(tty_dir_path);
                string info_path;
                //需注意ttyACM与ttyUSB的uevent文件实际深度不同
                if (port_dir == "ttyACM")
                    info_path = getParent(getParent(getParent(real_path)));
                else if (port_dir == "ttyUSB")
                    info_path = getParent(getParent(getParent(getParent(real_path))));
                dev = getDeviceInfo(info_path);
                dev.alias = port_dir + to_string(i);
                dev.path = tty_dir_path;
                devices.push_back(dev);
            }
        } 
    }
    return devices;
}

/**
 * @brief 获取串口有关信息
 * @return 串口类
 * **/
Device SerialPort::getDeviceInfo(string path)
{
    Device dev;
    auto uevent_path = path + "/uevent";
    auto texts = readLines(uevent_path);
    for(auto text : texts)
    {
        int equal_idx = text.find("=");
        string config_type = text.substr(0,equal_idx);
        string config_info = text.substr(equal_idx + 1);

        if(config_type == "PRODUCT")
        {
            dev.id = config_info;
        }
    }
    return dev;
}

/**
 * @brief 通过ID设置选取串口
 * @return 串口
 * **/
Device SerialPort::setDeviceByID(std::vector<Device> devices)
{
    for (auto dev : devices)
    {
        if (dev.id == serial_id)
            return dev;
    }
    return Device();
}

////////////////////////////////////////////////////////////////
/**
 *@brief   获取模式命令
 */
bool SerialPort::get_Mode()
{
    int bytes;
    char *name = ttyname(fd);
    if (name == NULL) printf("tty is null\n");
    int result = ioctl(fd, FIONREAD, &bytes);
    if (result == -1)
        return false;

    if (bytes == 0)
    {
    //    cout << "缓冲区为空" << endl;
        return false;
    }
    // bytes = read(fd, rdata, 49);
    bytes = read(fd, rdata, 50);
    // bytes = read(fd, rdata, 45);
    // cout<<bytes<<endl;

    if (rdata[0] == 0xA5 && Verify_CRC8_Check_Sum(rdata, 3))
    {
        mode = rdata[1];
        getQuat(&rdata[3]);
        getGyro(&rdata[19]);
        getAcc(&rdata[31]);
        getSpeed(&rdata[43]);
        Verify_CRC16_Check_Sum(rdata,50);
        //TODO:接收下位机发送的弹速
    }
    // mode = rdata[0];
    // getQuat(&rdata[1]);
    // getGyro(&rdata[17]);
    // getAcc(&rdata[29]);
    // getSpeed(&rdata[41]);
    return true;
}

/**
 *@brief   初始化数据
 *@param  fd       类型  int  打开的串口文件句柄
 *@param  speed    类型  int  波特率

 *@param  databits 类型  int  数据位   取值 为 7 或者8

 *@param  stopbits 类型  int  停止位   取值为 1 或者2
 *@param  parity   类型  int  效验类型 取值为N,E,O,S
 *@param  portchar 类型  char* 串口路径
 */
bool SerialPort::initSerialPort()
{
    device = setDeviceByID(listPorts());
    const string alias = "/dev/" + device.alias;

    if(alias.length() - 4 == 0)
        return false;

    close(last_fd);
	fd = open(alias.c_str(), O_RDWR | O_NOCTTY);

    speed = baud;
    databits = 8;
    stopbits = 1;
	parity = 'N';

	if (fd == -1)
	{
        perror(alias.c_str());
        return false;
    }

	std::cout << "Opening " <<alias<<"..."<< std::endl;
    set_Brate();

	if (set_Bit() == FALSE)
	{
        printf("Set Parity Error\n");
		exit(0);
    }
    printf("Open successed\n");

#ifdef SAVE_LOG_ALL
    LOG(INFO) << "[SERIAL] Open "<< alias << " successed"<<endl;
#endif //SAVE_LOG_ALL

    last_fd = fd;
    need_init = false;
    return true;
}

// TODO: finish visual com port
/**
 *@brief   初始化数据
 *@param  fd       类型  int  打开的串口文件句柄
 *@param  speed    类型  int  波特率

 *@param  databits 类型  int  数据位   取值 为 7 或者8

 *@param  stopbits 类型  int  停止位   取值为 1 或者2
 *@param  parity   类型  int  效验类型 取值为N,E,O,S
 *@param  portchar 类型  char* 串口路径
 */
bool SerialPort::withoutSerialPort()
{
    return true;
}

/**
 *@brief   设置波特率
 */
void SerialPort::set_Brate()
{
    // int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	// 				   B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	// 				  };
    // int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
	// 				  115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
	// 				 };
    int speed_arr[] = {B921600, B460800, B230400, B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {921600, 460800, 230400,115200, 38400, 19200, 9600, 4800, 2400, 1200,  300};
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);

	for (i = 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if (speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);//清空缓冲区的内容
			cfsetispeed(&Opt, speed_arr[i]);//设置接受和发送的波特率
			cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt); //使设置立即生效

			if (status != 0)
			{
                perror("tcsetattr fd1");
                return;
            }

			tcflush(fd, TCIOFLUSH);

        }
    }
}

/**
 *@brief   设置串口数据位，停止位和效验位
 */
int SerialPort::set_Bit()
{
    struct termios termios_p;

	if (tcgetattr(fd, &termios_p)  !=  0)
	{
        perror("SetupSerial 1");
		return (FALSE);
    }

    termios_p.c_cflag |= (CLOCAL | CREAD);  //接受数据
	termios_p.c_cflag &= ~CSIZE;//设置数据位数

    switch (databits)
    {
	case 7:
		termios_p.c_cflag |= CS7;
		break;

	case 8:
		termios_p.c_cflag |= CS8;
		break;

	default:
		fprintf(stderr, "Unsupported data size\n");
		return (FALSE);
    }

	//设置奇偶校验位double
    switch (parity)
    {
	case 'n':
	case 'N':
		termios_p.c_cflag &= ~PARENB;   /* Clear parity enable */
		termios_p.c_iflag &= ~INPCK;     /* Enable parity checking */
		break;

	case 'o':
	case 'O':
		termios_p.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
		termios_p.c_iflag |= INPCK;             /* Disnable parity checking */
		break;

	case 'e':
	case 'E':
		termios_p.c_cflag |= PARENB;     /* Enable parity */
		termios_p.c_cflag &= ~PARODD;   /* 转换为偶效验*/
		termios_p.c_iflag |= INPCK;       /* Disnable parity checking */
		break;

	case 'S':
	case 's':  /*as no parity*/
		termios_p.c_cflag &= ~PARENB;
		termios_p.c_cflag &= ~CSTOPB;
		break;

	default:
		fprintf(stderr, "Unsupported parity\n");
		return (FALSE);

    }

    /* 设置停止位*/
    switch (stopbits)
    {
	case 1:
		termios_p.c_cflag &= ~CSTOPB;
		break;

	case 2:
		termios_p.c_cflag |= CSTOPB;
		break;

	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return (FALSE);

    }

    /* Set input parity option */
    if (parity != 'n')
        termios_p.c_iflag |= INPCK;

	tcflush(fd, TCIFLUSH); //清除输入缓存区
    termios_p.c_cc[VTIME] = 150; /* 设置超时15 seconds*/
    termios_p.c_cc[VMIN] = 0;  //最小接收字符
    termios_p.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input原始输入*/
	termios_p.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    termios_p.c_iflag &= ~(ICRNL | IGNCR);
    termios_p.c_oflag &= ~OPOST;   /*Output禁用输出处理*/

	if (tcsetattr(fd, TCSANOW, &termios_p) != 0) /* Update the options and do it NOW */
    {
        perror("SetupSerial 3");
        return (FALSE);
    }

    return (TRUE);
}

/**
 *@brief   转换数据并发送
 *@param   data   类型  VisionData(union)  包含pitch,yaw,distance
 *@param   flag   类型  char   用于判断是否瞄准目标，0代表没有，1代表已经瞄准
 */
void SerialPort::TransformData(const VisionData &data)
{

    Tdata[0] = 0xA5;

    Tdata[1] = CmdID1;
	Append_CRC8_Check_Sum(Tdata, 3);

    Tdata[3] = data.pitch_angle.c[0];
    Tdata[4] = data.pitch_angle.c[1];
    Tdata[5] = data.pitch_angle.c[2];
    Tdata[6] = data.pitch_angle.c[3];

    Tdata[7] = data.yaw_angle.c[0];
    Tdata[8] = data.yaw_angle.c[1];
    Tdata[9] = data.yaw_angle.c[2];
    Tdata[10] = data.yaw_angle.c[3];

    Tdata[11] = data.dis.c[0];
    Tdata[12] = data.dis.c[1];
    Tdata[13] = data.dis.c[2];
    Tdata[14] = data.dis.c[3];

    Tdata[15] = data.isSwitched;
	Tdata[16] = data.isFindTarget;

    Tdata[17] = data.isSpinning;
    Tdata[18] = data.ismiddle;
    Tdata[19] = 0x00;

	Append_CRC16_Check_Sum(Tdata, 22);

}

/////////////////////////////////////////////
/**
 * @brief 将4个uchar转换为float
 * 
 * @param data data首地址指针
 * @return
 */
float SerialPort::exchange_data(unsigned char *data)
{
    float float_data;
    float_data = *((float*)data);
    return float_data;
};

/**
 * @brief 解算四元数数据
 * 
 * @param data 四元数首地址指针
 * @return
 */
bool SerialPort::getQuat(unsigned char *data)
{
    unsigned char* f1 = &data[0];
    unsigned char* f2 = &data[4];
    unsigned char* f3 = &data[8];
    unsigned char* f4 = &data[12];

    quat[0] = exchange_data(f1);
    quat[1] = exchange_data(f2);
    quat[2] = exchange_data(f3);
    quat[3] = exchange_data(f4);
    // fmt::print(fmt::fg(fmt::color::white), "quat: {} {} {} {} \n", quat[0], quat[1], quat[2], quat[3]);
    return true;
}

/**
 * @brief 解算角速度数据
 * 
 * @param data 角速度首地址指针
 * @return
 */
bool SerialPort::getGyro(unsigned char *data)
{    
    unsigned char* f1 = &data[0];
    unsigned char* f2 = &data[4];
    unsigned char* f3 = &data[8];

    gyro[0] = exchange_data(f1);
    gyro[1] = exchange_data(f2);
    gyro[2] = exchange_data(f3);

    // fmt::print(fmt::fg(fmt::color::white), "gyro: {} {} {} \n", gyro[0], gyro[1], gyro[2]);
    return true;
}

/**
 * @brief 解算加速度数据
 * 
 * @param data 加速度首地址指针
 * @return
 */
bool SerialPort::getAcc(unsigned char *data)
{
    unsigned char* f1 = &data[0];
    unsigned char* f2 = &data[4];
    unsigned char* f3 = &data[8];

    acc[0] = exchange_data(f1);
    acc[1] = exchange_data(f2);
    acc[2] = exchange_data(f3);
    // fmt::print(fmt::fg(fmt::color::white), "acc: {} {} {} \n", acc[0], acc[1], acc[2]);
    return true;
}
/**
 * @brief 解算速度数据
 * 
 * @param data 速度首地址指针
 * @return
 */
bool SerialPort::getSpeed(unsigned char *data)
{
    unsigned char* f1 = &data[0];

    bullet_speed = exchange_data(f1);

    // fmt::print(fmt::fg(fmt::color::white), "speed: {} \n", bullet_speed);
    return true;
}
//////////////////////////////////////////////

//发送数据函数
void SerialPort::send()
{
	auto write_stauts = write(fd, Tdata, 22);
}

//关闭通讯协议接口
void SerialPort::closePort()
{
    close(fd);
}

