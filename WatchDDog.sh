#!/bin/bash 

source /opt/intel/openvino_2021/bin/setupvars.sh

sec=1
cnt=0
name=TUP-InfantryVision-2022         #TODO:程序所在文件夹名称
projectname=Infantry_Vision
echo "sys666" | sudo -S chmod 777 /home/tup/Desktop/$name # -S --stdin 直接从标准输入读取密码，不需要手动添加
                                                                # TODO:password 为自己的密码
cd /home/tup/Desktop/$name/build/
# cmake .. && make clean && 
make -j6 

while [ 1 ] 
do
    count=`ps -ef | grep $name | grep -v "grep" | wc -l`    # 查看进程数
    # if [ $count -eq 0 ]
    # then
    #     echo "start $name"
    #     ./$name
    # fi
    echo "Thread count: $count" 
    echo "Expection count: $cnt" 
    # cd /home/liubiao/$name/build/

    #检查串口是否连接（未打开则等待）
    while true ; do
        if [[ -e /dev/ttyACM0 || -e /dev/ttyACM1 ]] ; then
            echo "tty is connected！"
            break
        else
            #等待串口连接
            echo "tty is not connected！"
            sleep 1
        fi
    done
    (sudo chmod 777 /dev/ttyACM0 || chmod 777 /dev/ttyACM1)  # 给串口权限
    
    if [ $count -gt 2 ]; then
        echo "The $projectname is still alive!" 
        sleep $sec 
    else  
        echo "Starting $projectname..." 
        # gnome-terminal -- bash -c "cd /home/tup/Desktop/$name/build/;
        # ./$name;exec bash;" 
        cd /home/tup/Desktop/$name/build/   #TODO:程序build文件夹路径
        make -j6 && ./$projectname
        echo "$projectname has started!"   
        sleep $sec 

        ((cnt=cnt+1)) 
        if [ $cnt -gt 9 ]; then 
            #reboot              #比赛前打开
            echo "reboot!"       #调试时打开
        fi 
    fi 
done
