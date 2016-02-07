#!/bin/bash

#run=$1

while [ 1 ]; do 
    
  date=`date +"%Y-%m-%d_%H-%M-%S"`
  echo $date
  string=(`odbedit -c "ls '/Runinfo/run number'"`)
  run=${string[2]}
  
  wget http://192.168.2.131/image.png
  mv image.png /data/MTest2012/scope/scope_run_${run}_${date}.png
  sleep 62
  
done

