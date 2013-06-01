#!/usr/bin/gdb -x

#file scheduler_bdema

define ARprint
	set $i = 0

	while ( $i < $arg1)
		print $arg0[$i]
		set $i = $i +1
	end
end

define llprint
	set $currentte = $arg0
	set $ii = $arg1

	while ($ii > 0)
		set $ii = $ii - 1
		set $currentte = $currentte->next
		print *$currentte
	end
end

define print_report 
  print "QUEUE size:" 
  print "QUEUE size:" 
  print "QUEUE size:" 
  print "QUEUE size:" 
  print "QUEUE size:" 
  print "QUEUE size:" 
  print "QUEUE size:" 
  print "QUEUE size:" 
end
