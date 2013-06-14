#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

char help [] =
 "|====================================================|\n"
 "| RANDOM NUMBER GENERATOR                            |\n"
 "|====================================================|\n"
 "|  -h           help                                 |\n" 
 "|  -n           Number of numbers                    |\n"
 "|  -f ,from     Lower boundary                       |\n"
 "|  -t ,to       Upper boundary                       |\n"
 "|  -l ,lambda   lambda parameter (exponential dist)  |\n"
 "|  -d ,dist     Distribution (exponential|uniform)   |\n"
 "|                                                    |\n"
 "|====================================================|\n"
 "| AUTHOR                vicente.bolea@gmail.com      |\n"
 "| VERSION               0.1.0                        |\n"
 "|====================================================|\n";

inline int uniform (const int low, const int up) {
	return (rand () % up) + low;
}

inline int exponential (const double lambda, const int low, const int up) {
 return  ((int)ceil (((double)-1.0 / lambda) * log (1 - ((double) rand () / RAND_MAX)))) % up + low;
}

int main (int argc, const char ** argv) {
	int i, limit, low, up, c = 0;
  double lambda = 1;  
  char dist [32];

	do {
		switch (c) {
			case 'h': printf ("%s\n",help);           break;
			case 'n': limit = atoi (optarg);          break;
			case 'f': low = atoi (optarg);            break;
			case 't': up = atoi (optarg);             break;
			case 'l': lambda = strtod (optarg, NULL); break;
			case 'd': strncpy (dist, optarg, 32);     break;
		}
		c = getopt (argc, (char**)argv, "hn:f:t:d:l:");
	} while (c != -1);

	srand (time(NULL));
	if (strcmp (dist, "uniform") == 0) {

		for (i = 0; i < limit; i++)
			printf ("%i %i\n", uniform (low, up), uniform (low, up));

	} else if (strcmp (dist, "exponential") == 0) {
		for (i = 0; i < limit; i++)
			printf ("%i %i\n", exponential (lambda, low, up), exponential (lambda, low, up));
	}

	return EXIT_SUCCESS;
}
