#include <signal.h>
#include <uniDQP.h>

#define RULER()                                        \
   printf ("%-55s\n", (                                \
   "---------------------------------------------"     \
   "---------------------------------------------"     \
   "---------------------------------------------")) 
    


using namespace std;

int sock, port;
int16_t connected [NSERVERS];
uint64_t TotalCacheHit = 0, TotalCacheMiss = 0,  numQuery = 0;
uint64_t TotalExecTime = 0, TotalWaitTime = 0; 
uint64_t AveExecTime = 0, AveWaitTime = 0; 
uint64_t MaxExecTime = 0, MaxWaitTime = 0; 

inline int hash (packet* p) {
  int a = p->fid;
  a = (a+0x7ed55d16) + (a<<12);
  a = (a^0xc761c23c) ^ (a>>19);
  a = (a+0x165667b1) + (a<<5);
  a = (a+0xd3a2646c) ^ (a<<9);
  a = (a+0xfd7046c5) + (a<<3);
  a = (a^0xb55a4f09) ^ (a>>16);

  int b = p->offset;
  b = (b+0x7ed55d16) + (b<<12);
  b = (b^0xc761c23c) ^ (b>>19);
  b = (b+0x165667b1) + (b<<5);
  b = (b+0xd3a2646c) ^ (b<<9);
  b = (b+0xfd7046c5) + (b<<3);
  b = (b^0xb55a4f09) ^ (b>>16);

  return abs ((a+b)%NSERVERS);
}

void receive_all (void) {
	char recv_data[LOT];
	char INFO[] = "INFO";

	for (int i = 0; i < NSERVERS; i++)
		send_msg (connected[i], INFO, strlen(INFO));  

	cout << "Collecting the results" << endl;
	for (int i = 0; i < NSERVERS; i++) {  

		bzero (recv_data, LOT);
		recv (connected[i], recv_data, LOT, MSG_WAITALL);

		for (char *key= strtok(recv_data, "=\n"); key != NULL; key= strtok(NULL, "=\n")) { 
			char *val = strtok (NULL, "=\n");

			if (strcmp (key, "CacheHit") == 0)
				TotalCacheHit += strtoul (val, NULL, 10);

			else if (strcmp (key, "CacheMiss") == 0)
				TotalCacheMiss += strtoul (val, NULL, 10);

			else if (strcmp (key, "QueryCount") == 0)
				numQuery += strtoul (val, NULL, 10);

			else if (strcmp (key, "TotalExecTime") == 0) {
				TotalExecTime += strtoull (val, NULL, 10);

				if (strtoull (val, NULL, 0) > MaxExecTime)
					MaxExecTime = strtoull (val, NULL, 10);
			}
			else if (strcmp(key, "TotalWaitTime") == 0) {
				TotalWaitTime += strtoull (val, NULL, 10);

				if (strtoull (val, NULL, 0) > MaxWaitTime)
					MaxWaitTime = strtoull (val, NULL, 10);
			}
			else if (strcmp (key, "OK") != 0)
				cerr << "[" << recv_data << "] @ " << i << endl;
		}
	}
}

void print_header (void) {
#define HEADER ("|%10.10s|%10.10s|%10.10s|%10.10s|"                 \
                "%10.10s|%10.10s|%10.10s|%10.10s|"                  \
                "%10.10s|%10.10s|%10.10s|%10.10s|"              "\n")
 RULER ();
 printf (HEADER, "Queries", "DiskPages", "TotalHits", "TotalMiss",
                 "TotalExecTime", "MaxExecTime", "AveExecTime",
                 "TotalWaitTime", "MaxWaitTime", "AveWaitTime",
                 "TotalTime", "HitRatio");
 RULER ();
}

void print_out (void) {
#define BODY (                                                      \
  "|%010" PRIu64 "|%010" PRIu64 "|%010" PRIu64 "|%010" PRIu64       \
  "|%010" PRIu64 "|%010" PRIu64 "|%010" PRIu64 "|%010" PRIu64       \
  "|%010" PRIu64 "|%010" PRIu64 "|%010" PRIu64 "|%010.10Lf" PRIu64  \
                                                               "|\n")

 printf (BODY, numQuery, TotalCacheHit + TotalCacheMiss,
               TotalCacheHit, TotalCacheMiss, TotalExecTime,
               MaxExecTime, AveExecTime, TotalWaitTime, MaxWaitTime,
               AveWaitTime, TotalWaitTime + TotalExecTime,
               (long double) ( (double)TotalCacheHit / 
               (TotalCacheHit + TotalCacheMiss) ) );
}

void wakeUpServer (void)
{
 int one = 1;
 struct sockaddr_in server_addr;
 if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
  perror ("Socket");
  exit (EXIT_FAILURE);
 }
 if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
  perror ("Setsockopt");
  exit (EXIT_FAILURE);
 }
 server_addr.sin_family = AF_INET;
 server_addr.sin_port = htons (port);
 server_addr.sin_addr.s_addr = INADDR_ANY;
 bzero (&(server_addr.sin_zero),8);

 if (bind (sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
  perror ("Unable to bind");
  exit (EXIT_FAILURE);
 }
 if (listen (sock, NSERVERS + 1) == -1) {
  perror ("Listen");
  exit (EXIT_FAILURE);
 }
 cout << "TCPServer Waiting for client on port: " << PORT << endl;
}

void catchSignal (int Signal) {
 cerr << "Closing sockets & files.\t Signal: " <<  strsignal(Signal) << endl;
 close (sock);
 exit (EXIT_FAILURE);
}

int main (int argc, char** argv) {
 int c, nservers;
 uint32_t nqueries = 0;
 uint32_t cnt = 0, time = 0;
 struct timeval start, end;

 struct sockaddr_in* client_addr;    
 uint32_t* queryCount;
 server** ema;

 while ((c = getopt(argc, const_cast<char**> (argv), "q:n:p:")) != -1)
  switch (c) {
   case 'q': nqueries = atoi (optarg); break;
   case 'n': nservers = atoi (optarg); break;
   case 'p': port = atoi (optarg);     break;
  }

 //Dynamic memory
 client_addr = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in) * nservers);
 ema = (server**) calloc (nservers, sizeof (server*));
 queryCount  = (uint32_t*) calloc (nservers, sizeof(uint32_t));

 signal (SIGINT, catchSignal);		//catching signals to close sockets and files
 signal (SIGSEGV, catchSignal);
 signal (SIGTERM, catchSignal);

 wakeUpServer();

 for (int i = 0; i < nservers; i++) {  
  socklen_t sin_size = sizeof(struct sockaddr_in);
  connected[i] = accept (sock, (struct sockaddr *)&client_addr[i], &sin_size);
  printf ("I got a connection from %s\n", inet_ntoa(client_addr[i].sin_addr)); 
 } 
 gettimeofday(&start, NULL);

 (void)print_header ();
 //Main loop which use the given algorithm to send the queries
 for (char line[100]; fgets(line, 100, stdin) != NULL && cnt < nqueries; cnt++)
 {
  packet toSend (line);
  if (toSend.time != time && time != 0)
   usleep (toSend.time - time);

  if (toSend.getEMA() < 0) {
   cnt--;
   continue;
  }
  int selected = 0;

#ifdef HASH
  selected = hash (&toSend);

#elif defined BDEMA
  double minDist = DBL_MAX;

  for (int i = 0; i < NSERVERS; i++) {
   if ((ema + i) == NULL) {
    ema[i] = (server*) new server (toSend, ALPHA);
    selected = i;

   } else if (ema[i]->getDistance(toSend) * queryCount[i] < minDist) {
    selected = i;
    minDist = ema[i]->getDistance(toSend) * queryCount[i];
   }
  }

  ema[selected]->updateEMA(toSend);
  queryCount[selected]++;
#elif defined KDE

#endif

  time = toSend.time;

  char query[] = "QUERY";
  send_msg (connected[selected], query, strlen(query));
  send (connected[selected], &toSend, sizeof(packet), 0);

  if ((cnt+1)%10000 == 0) {
   (void)receive_all ();
   (void)print_out ();
  }
 }
 RULER ();

 //Delete dynamic objects and say bye to back-end severs
 for (int i = 0; i < NSERVERS; i++) {
  delete &ema[i];
  ema[i] = NULL;
 }

 char QUIT[] = "QUIT";
 for (int i = 0; i < NSERVERS; i++)
  send_msg (connected[i], QUIT, strlen(QUIT));  

 gettimeofday (&end, NULL);
 printf ("schedulerTime:\t %20" PRIu64 "\t S-6\n", timediff(&end, &start));

 // close sockets
 for (int i = 0; i < NSERVERS; i++)
  close (connected[i]);

 close (sock);
 free (client_addr);
 free (queryCount);
 free (ema);

 return EXIT_SUCCESS;
}