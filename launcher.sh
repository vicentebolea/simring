#!/bin/bash

function killapps {
 echo Killing another instances of this program
 killall scheduler 
 ssh raven01 killall node
 ssh raven02 killall node
 ssh raven03 killall node
 ssh raven04 killall node
 ssh raven05 killall node
 ssh raven06 killall node
 ssh raven07 killall node
 ssh raven08 killall node
}

trap killapps SIGINT SIGTERM
#raven01 gdbserver :29999 
> /home/vicente/simring/outnod
ssh raven01 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l nothing -r raven02 &  #>> /home/vicente/simring/outnode &
ssh raven02 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l raven01 -r raven03 &  #>> /home/vicente/simring/outnode &
ssh raven03 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l raven02 -r raven04 &  #>> /home/vicente/simring/outnode &
ssh raven04 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l raven03 -r raven05 &  #>> /home/vicente/simring/outnode &
ssh raven05 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l raven04 -r raven06 &  #>> /home/vicente/simring/outnode &
ssh raven06 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l raven05 -r raven07 &  #>> /home/vicente/simring/outnode &
ssh raven07 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l raven06 -r raven08 &  #>> /home/vicente/simring/outnode &
ssh raven08 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l raven07 -r nothing &  #>> /home/vicente/simring/outnode &

#/home/vicente/simring/src/scheduler/scheduler -p 19999 -n 8 -q 100 -s 10 < /home/vicente/simring/src/scheduler/input1.dat
