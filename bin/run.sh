path=~vicente/uniDQP/
appserver=$path'bin/appserver'
scheduler=$path'bin/scheduler'
declare -a PIDS

idx=0
for i in {1..39}; do	
 if ((i < 10)); then
	ssh raven0$i $appserver&
	pids[idx]=$!
 else
	ssh raven$i $appserver&
	pids[idx]=$!
 fi

echo finish
for i in ${PIDS[@]}; do
	wait $i
done


