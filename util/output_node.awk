#!/bin/awk -f

BEGIN {
	print "[#queries] [hits] [Miss] [hitRatio] [Exec Time] [Avg QWET]"
}

{
	if ($1 == "Recieved:") { 
		printf "%s\t",$2 
	}
	if ($1 == "Processed:") { 
		printf "%s\t",$2 
	}
	if ($1 == "CacheHits:")	{ 
		printf "%s\t",$2 
	}
	if ($1 == "TotalExecTime:")	{ 
		printf "%s\t",$2 
	}
	if ($1 == "TotalWaitTime:" )	{ 
		printf "%s\t",$2 
	}
	if ($1 == "TotalTime:") 	{ 
		printf "%s\n",$2 
	}
}

END { 
}
