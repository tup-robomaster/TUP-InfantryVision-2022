#include "exception.h"

template<class T>
void test(T& num)
{
    if(typeid(T) != typeid(float))
    {
        throw typeError();
    }

    if(num == 0)
    {
        throw divideZeroException();
    }
}

int main()
{
    float a = 0;
    try
    {
        test<float>(a);
    }
    catch(BaseException& e)
    {
        e.what();    
    }
    
}