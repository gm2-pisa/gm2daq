#!/bin/bash

while [ 1 ]; do 
    
  date=`date +"%Y-%m-%d_%H-%M-%S"`
  echo $date

  string=(`odbedit -c "ls '/Runinfo/run number'"`)
  run=${string[2]}


  wget http://calice-cam01.fnal.gov:8080/oneshotimage.jpg
  mv oneshotimage.jpg /data/MTest2012/webcam/T_run_${run}_$date.jpg
  sleep 62
  
done

