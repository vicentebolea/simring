#include <signal.h>
#include <simring.hh>

#define RULER()                                        \
   printf ("%-54s\n", (                                \
   "---------------------------------------------"     \
   "---------------------------------------------"     \
   "---------------------------------------------")) 
    

using namespace std;

int sock, port = 0;
int16_t connected [NSERVERS];
uint64_t TotalCacheHit = 0, TotalCacheMiss = 0,  numQuery = 0;
uint64_t TotalExecTime = 0, TotalWaitTime = 0; 
uint64_t AveExecTime = 0, AveWaitTime = 0; 
uint64_t MaxExecTime = 0, MaxWaitTime = 0; 

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

void wakeUpServer (void) {

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
  perror ("SCHEDULER: Unable to bind");
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
 int c, nservers = 0;
 uint32_t nqueries = 0, step = 10000;
 uint32_t cnt = 0;
 struct timeval start, end;

 struct sockaddr_in* client_addr;    
 uint32_t* queryCount;
 Node** backend;

 while ((c = getopt(argc, const_cast<char**> (argv), "q:n:p:s:")) != -1)
  switch (c) {
   case 'q': nqueries = atoi (optarg); break;
   case 'n': nservers = atoi (optarg); break;
   case 'p': port = atoi (optarg);     break;
   case 's': step = atoi (optarg);     break;
  }

 if (!nqueries || !nservers || !port) {
   fprintf (stderr, "PARSER: all the options needs to be setted\n");
   exit (EXIT_FAILURE);
 }

 //Dynamic memory
 client_addr = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in) * nservers);
 backend = new Node* [nservers];
 queryCount  = new uint32_t [nservers];

 signal (SIGINT, catchSignal);		//catching signals to close sockets and files
 signal (SIGSEGV, catchSignal);
 signal (SIGTERM, catchSignal);

 wakeUpServer();

 for (int i = 0; i < nservers; i++) {  
  socklen_t sin_size = sizeof(struct sockaddr_in);
  connected[i] = accept (sock, (struct sockaddr *)&client_addr[i], &sin_size);
  printf ("I got a connection from %s\n", inet_ntoa(client_addr[i].sin_addr)); 
 } 

 gettimeofday (&start, NULL);
 print_header ();
 
 //! Initiliaze the node equally
 const uint64_t data_interval = 1000000;
 { 
 int j = 0;
 for (uint64_t i = 0; i < data_interval; i += (data_interval/nservers)) {
   backend [j++] = new Node (ALPHA, i + (data_interval/nservers)/2);
   cout << "FIRST: i: " << i << " j: " << j << " b: " << backend[j-1]->get_EMA () << endl;
 }
 }  

 //! Main loop which use the given algorithm to send the queries
 for (char line[100]; fgets(line, 100, stdin) != NULL && cnt < nqueries; cnt++)
 {
  int selected = 0;
  uint64_t point = prepare_input (line);
  cout << "POINT: " << point << " ";
  double minDist = DBL_MAX;

  for (int i = 0; i < nservers; i++) {
   if (backend[i]->get_distance (point) < minDist) {
    selected = i;
    minDist = backend[i]->get_distance (point);
   }
  }

  //! :TODO: Circular boundaries
  if (selected == 0) {                     //! For now lets fix the first node low boundary in 0
    backend[selected]->set_low (.0);

  } else {
    backend[selected]->set_low ((backend [selected]->get_EMA () - backend [(selected - 1)]->get_EMA ()) / 2.0);
  } 

  if (selected == nservers - 1 || backend[selected + 1] == NULL) {
    backend[selected]->set_upp (DBL_MAX);  //! For now lets fix the last node upp boundary in the maximum num

  } else {
    backend[selected]->set_upp ((backend [selected + 1]->get_EMA () - backend [selected]->get_EMA ()) / 2.0);
  }

  backend[selected]->update_EMA (point);
  //queryCount[selected]++;

  cout << "NODE: " << selected;
  cout << " EMA: " <<  backend[selected]->get_EMA ();
  cout << " SENDING: " << point << " LOW: " << backend[selected]->get_low() << " UPP: " << backend[selected]->get_upp() << endl;
  char query [] = "QUERY";
  send_msg (connected[selected], query, strlen (query));
  packet toSend (point);
  send (connected[selected], &toSend, sizeof (packet), 0);

  if ((cnt + 1) % step == 0) {
   receive_all ();
   print_out ();
  }
 }
 RULER ();

 //! Delete dynamic objects and say bye to back-end severs
 for (int i = 0; i < NSERVERS; i++) {
  delete &backend[i];
  backend[i] = NULL;
 }

 char QUIT[] = "QUIT";
 for (int i = 0; i < NSERVERS; i++)
  send_msg (connected[i], QUIT, strlen(QUIT));  

 gettimeofday (&end, NULL);
 printf ("schedulerTime:\t %20" PRIu64 "\t S-6\n", timediff (&end, &start));

 //! Close sockets
 for (int i = 0; i < NSERVERS; i++)
  close (connected[i]);

 close (sock);
 free (client_addr);
 delete[] queryCount;
 delete[] backend;

 return EXIT_SUCCESS;
}
