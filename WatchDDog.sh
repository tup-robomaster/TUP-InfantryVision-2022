#!/bin/bash 

source /opt/intel/openvino_2021/bin/setupvars.sh
sec=1 
cnt=0 
name=TUP-InfantryVision-2022-main
program_name=Infantry_Vision
cd /home/tup/$name/build/
#make clean && 
make -j6
./$program_name
while [ 1 ] 
do 
    count=`ps -ef | grep $name | grep -v "grep" | wc -l`
    echo "Thread count: $count" 
    echo "Expection count: $cnt" 
    if [ $count -gt 1 ]; then 
        echo "The $name is still alive!" 
        sleep $sec 
    else  
        echo "Starting $name..." 
        gnome-terminal -- bash -c "cd /home/tup/$name/build/;
        ./$program_name;exec bash;" 
        echo "$program_name has started!"   
        sleep $sec 
        ((cnt=cnt+1)) 
        if [ $cnt -gt 9 ]; then 
            echo "reboot!"
        fi 
    fi 
done
