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
ssh raven01 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l nothing     -r 192.168.1.2 & 
ssh raven02 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.1 -r 192.168.1.3 & 
ssh raven03 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.2 -r 192.168.1.4 & 
ssh raven04 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.3 -r 192.168.1.5 & 
ssh raven05 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.4 -r 192.168.1.6 & 
ssh raven06 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.5 -r 192.168.1.7 & 
ssh raven07 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.6 -r 192.168.1.8 & 
ssh raven08 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.7 -r nothing & 

/home/vicente/simring/src/scheduler/scheduler -p 19999 -n 8 -q 200000 -s 10000 < /home/vicente/simring/src/scheduler/input1.dat
