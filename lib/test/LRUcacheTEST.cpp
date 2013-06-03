#include <iostream>
#include <stdlib.h>
#include <uniDQP.h>

int main(){
	// Test A
	{
		char a1[] = "1.5000.100000";
		char a2[] = "1.5000.100000";
		packet p_a (a1);
		LRUcache cache (150);
		cache.match (&p_a);
		packet p_b (a2);
		//count match
		printf("###%i", cache.match (&p_b));
	}

/*	
	//Test B
	{
		char a1[] = "1.0.000030";
		char a2[] = "2.0.102030";
		char a3[] = "6.0.202000";
		char a4[] = "1.0.100000";

	}
*/
}
