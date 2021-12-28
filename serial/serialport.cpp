#include "serialport.h"

SerialPort::SerialPort()
{
    initSerialPort();
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
                auto info_path = getParent(getParent(getParent(getParent(real_path))));
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
        if (dev.id == SERIAL_ID)
            return dev;
    }
    return Device();
}

////////////////////////////////////////////////////////////////
/**
 *@brief   获取模式命令
 */

bool SerialPort::get_Mode(int &mode, int &sentry_mode, int &base_mode)
{
    int bytes;
    char *name = ttyname(fd);
    if (name = NULL)printf("tty is null\n");
    //if (name != NULL)printf("device:%s\n",name);
    int result = ioctl(fd, FIONREAD, &bytes);
    if (result == -1)return false;


    if (bytes == 0)
    {
    //    cout << "缓冲区为空" << endl;
        return true;
    }

    bytes = read(fd, rdata, 22);

    if (rdata[0] == 0xA5 && Verify_CRC8_Check_Sum(rdata, 3))
    {
        //判断针头和CRC校验是否正确
        mode  = (int)rdata[1]; //通过此数据控制线程的开启	0关闭自瞄1开启自瞄2小能量机关3大能量机关
        // printf("接收到的指令:%d\r\n", mode);
	//----------No use---------//    
        sentry_mode  = (int)rdata[15];
        // printf("Is in sentry mode ? :%d\r\n", sentry_mode);
        base_mode  = (int)rdata[16];
        // printf("Is in base mode ? :%d\r\n", base_mode);

    }
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

    close(last_fd);
	fd = open(alias.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    speed = BAUDRATE;
    databits = 8;
    stopbits = 1;
	parity = 'N';

	if (fd == -1)
	{
        perror(alias.c_str());
        return false;
    }

	std::cout << "Opening..." << std::endl;
    set_Brate();

	if (set_Bit() == FALSE)
	{
        printf("Set Parity Error\n");
		exit(0);
    }
    printf("Open successed\n");

    last_fd = fd;
    need_init = false;
    return true;
}

/**
 *@brief   设置波特率
 */
void SerialPort::set_Brate()
{
    int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
					   B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
					  };
    int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
					  115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
					 };
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


////
//int SerialPort::set_disp_mode(int option)
//{
//   int err;
//   struct termios term;
//   if(tcgetattr(fd,&term)==-1){
//     perror("Cannot get the attribution of the terminal");
//     return 1;
//   }
//   if(option)
//        term.c_lflag|=ECHOFLAGS;   //(ECHO | ECHOE | ECHOK | ECHONL)
//   else
//        term.c_lflag &=~ECHOFLAGS;
//   err=tcsetattr(fd,TCSAFLUSH,&term);
//   if(err==-1 && err==EINTR){
//        perror("Cannot set the attribution of the terminal");
//        return 1;
//   }
//   return 0;
//}


////////////////////////////////////////////////////////////
/**
 *@brief   转换数据并发送

 *@param  p        类型  int  x坐标
 *@param  yaw      类型  int  y坐标
 *@param  dis      类型  int  dis  距离
 *@param  unsign   类型  int  空白（可添加数据）
 */
void SerialPort::TransformDataFirst(int Xpos, int Ypos, int dis)
{

    Tdata[0] = 0xA5;
    Tdata[1] = CmdID0;   //command

	Append_CRC8_Check_Sum(Tdata, 3); //CRC8 校验


	for (int i = 0; i < 4; i++)
	{
		Tdata[3 + i] = Xpos % 10;
		Xpos = (Xpos - Xpos % 10) / 10;
    }

	for (int i = 0; i < 4; i++)
	{
		Tdata[7 + i] = Ypos % 10;
		Ypos = (Ypos - Ypos % 10) / 10;
    }

	for (int i = 0; i < 4; i++)
	{
		Tdata[11 + i] = dis % 10;
		dis = (dis - dis % 10) / 10;
    }

	Tdata[15] = 0;//通过此端口向下位机发送是否识别到敌方装甲板或能量机关
    Tdata[16] = 0;
    Tdata[17] = 0;
    Tdata[18] = 0;
    Tdata[19] = 0;

	Append_CRC16_Check_Sum(Tdata, 22); //CRC16校验
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

    Tdata[15] = data.ismiddle;
	Tdata[16] = data.isFindTarget;

    Tdata[17] = data.isfindDafu;
    Tdata[18] = 0x00;
    Tdata[19] = data.nearFace;

	Append_CRC16_Check_Sum(Tdata, 22);

}

//发送数据函数
void SerialPort::send()
{
	write(fd, Tdata, 22);
}

//关闭通讯协议接口
void SerialPort::closePort()
{
    close(fd);
}
