#!/bin/bash

#####################################################
# Script to run                                     #
# Vicente Adolfo Bolea Sanchez                      #
#####################################################

#Includes
. bin/launcher.vars

#Dont needed to change
path_file=`readlink -f $0`
path=$(dirname $path_file)

node=$path'/../bin/node'
#node_args="-h "$host_addr" -p "$port" -d "$datafile
data=$path'/../data/trace4.ready'
#log=$path'../log/'
bin=$path'/../bin/'
date=`date +"%H/%M/%S-%d/%m/%y"`

if [ -z $nodes ] || [ -z $path ]; then 
 echo "Undefined variable in launcher.vars"
 exit
fi

function killapps {
 echo Killing another instances of this program
 killall scheduler_bdema scheduler_hash

 for i in "${nodes[@]}"; do
   ssh $i killall node
 done
 exit
}

trap killapps SIGINT SIGTERM

declare -a PIDS
declare -a TEMP_FILE

case "$1" in 
"BDEMA") scheduler=$bin'scheduler_bdema' ;;
"HASH")  scheduler=$bin'scheduler_hash'  ;; 
"KILL")  killapps                        ;; 

*)
 echo "Introduce 'launcher BDEMA|HASH|KILL' "
 exit 
 ;;

esac

#Create output tmp files for each node
for i in "${nodes[@]}"; do
 TMP_FILE[$i]=`mktemp "/tmp/simulator_"$i"_XXX"`
done

TMP_FILE["error"]="/tmp/simulator_error"

# launch programs
ii=$2
$scheduler -q $ii -p 19999 -n 7 < $data 2>> ${TMP_FILE["error"]}&
pidS=$!

idx=0
for i in "${nodes[@]}"; do
 ssh $i "$node -h $host_addr -p $port -d $datafile" > ${TMP_FILE[$i]} 2>> ${TMP_FILE["error"]}&
 PIDS[$idx]=$!
 let idx++
done

#Wait for all the process
for i in ${PIDS[@]}; do
 wait $i
done
wait $pidS

#store the output
#cat tmp/uniDQP.*.log | awk \
     #' $1 == "Processed:" { printf "%s\t",$2 }
     #END{ print "" } ' >> $distribution_log

echo Done with $ii IO operations 

for i in "${nodes[@]}"; do
 rm TMP_FILE[$i]
done
