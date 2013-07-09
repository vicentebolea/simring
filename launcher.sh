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
 ssh raven09 killall node
 ssh raven10 killall node
 ssh raven11 killall node
 ssh raven12 killall node
 ssh raven13 killall node
 ssh raven14 killall node
 ssh raven15 killall node
 ssh raven16 killall node
}

trap killapps SIGINT SIGTERM
#raven01 gdbserver :29999
/home/vicente/simring/src/scheduler/scheduler -p 19999 -n 16 -q 1000000 -s 10000 < /home/vicente/simring/src/scheduler/input1.dat&

ssh raven01 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l nothing     -r 192.168.1.2 & sleep 1
ssh raven02 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.1 -r 192.168.1.3 & sleep 1
ssh raven03 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.2 -r 192.168.1.4 & sleep 1
ssh raven04 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.3 -r 192.168.1.5 & sleep 1
ssh raven05 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.4 -r 192.168.1.6 & sleep 1
ssh raven06 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.5 -r 192.168.1.7 & sleep 1
ssh raven07 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.6 -r 192.168.1.8 & sleep 1
ssh raven08 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.7 -r 192.168.1.9 & sleep 1
ssh raven09 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.8 -r 192.168.1.10 & sleep 1
ssh raven10 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.9 -r 192.168.1.11 & sleep 1
ssh raven11 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.10 -r 192.168.1.12 & sleep 1
ssh raven12 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.11 -r 192.168.1.13 & sleep 1
ssh raven13 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.12 -r 192.168.1.14& sleep 1
ssh raven14 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.13 -r 192.168.1.15& sleep 1
ssh raven15 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.14 -r 192.168.1.16& sleep 1
ssh raven16 /home/vicente/simring/src/node/node -h 10.20.12.170 -p 19999 -d /scratch/youngmoon01/garbage2.bin -l 192.168.1.15 -r nothing & 

