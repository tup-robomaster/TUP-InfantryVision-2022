#include <iostream>
#include <algorithm>
#include <exception>
#include <typeinfo>
using namespace std;

#pragma once
class BaseException
{
public:
    virtual void what() = 0;
    virtual ~BaseException(){}
};

class divideZeroException : public BaseException
{
public:
    virtual void what()
    {
        std::cerr << "除数为零！" << endl;
    }

    ~divideZeroException(){}
};

class typeError : public BaseException
{
public:
    virtual void what()
    {
        std::cerr << "数据类型错误！" << endl;
    }

    ~typeError(){}
};

class pathError : public BaseException
{
public:
    virtual void what()
    {
        std::cerr << "未检测到串口路径！" << endl;
    }

    ~pathError(){}
};

class setBrateDefault : public BaseException
{
public:
    virtual void what()
    {
        std::cerr << "波特率设置失败！" << endl;
    }
};

class serailSendingError : public BaseException
{
public:
    virtual void what()
    {
        std::cerr << "数据发送失败！" << endl;
    }
};

class openFileDefault : public BaseException
{
public:
    virtual void what()
    {
        std::cerr << "文件打开失败！" << endl;
    }
};




