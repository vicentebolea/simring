#!/bin/bash
SERVERS=(cherry birch dove elm gingko hawthorn fever)

trap killapps SIGINT SIGTERM
function killapps {
 echo Killing another instances of this program
 killall scheduler 

 for i in ${SERVERS[@]}; do	
   ssh $i killall node
 done
 exit
}
path=~vicente/simulator_simple/
appserver=$path'bin/node'
declare -a PIDS

idx=0
for i in ${SERVERS[@]}; do	
  ssh $i gdbserver localhost:1212 $appserver&
  pids[idx]=$!
done

echo finish
for i in ${PIDS[@]}; do
	wait $i
done



#
