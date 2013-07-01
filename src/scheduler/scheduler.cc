#include <signal.h>
#include <simring.hh>
#include <setjmp.h>

#define RULER()                                        \
   printf ("%-80.145s\n", (                            \
   "---------------------------------------------"     \
   "---------------------------------------------"     \
   "---------------------------------------------"     \
   "---------------------------------------------"     \
   "---------------------------------------------")) 
    

using namespace std;

int sock, port = 0, nservers = 0;
int16_t* connected;
uint64_t TotalCacheHit = 0, TotalCacheMiss = 0,  numQuery = 0;
uint64_t TotalExecTime = 0, TotalWaitTime = 0, shiftedQuery = 0, SentShiftedQuery = 0; 
uint64_t AveExecTime = 0, AveWaitTime = 0; 
uint64_t MaxExecTime = 0, MaxWaitTime = 0; 
static jmp_buf finish;

void receive_all (void) {
	char recv_data [LOT];

	TotalCacheHit = 0, TotalCacheMiss = 0,  numQuery = 0;
  TotalExecTime = 0, TotalWaitTime = 0; 
  AveExecTime = 0, AveWaitTime = 0; 
  MaxExecTime = 0, MaxWaitTime = 0; 
  SentShiftedQuery = 0, shiftedQuery = 0;

	for (int i = 0; i < nservers; i++)
		send_msg (connected[i], "INFO");  

	for (int i = 0; i < nservers; i++) {  

		bzero (recv_data, LOT);
		recv (connected[i], recv_data, LOT, MSG_WAITALL);

		for (char *key= strtok(recv_data, "=\n"); key != NULL; key= strtok(NULL, "=\n")) { 
			char* val = strtok (NULL, "=\n");

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

			else if (strcmp(key, "shiftedQuery") == 0) {
				shiftedQuery += strtoull (val, NULL, 10);
      }
			else if (strcmp(key, "SentShiftedQuery") == 0) {
				SentShiftedQuery += strtoull (val, NULL, 10);
      }
			else if (strcmp (key, "OK") != 0)
				cerr << "[" << recv_data << "] @ " << i << endl;
		}
    AveExecTime = TotalExecTime / nservers;
    AveWaitTime = TotalWaitTime / nservers;
	}
}

/*
 * 
 */
void print_header (void) {
 RULER ();
 printf (
   "|%15.15s|%15.15s|%15.15s|%15.15s|%15.15s|%15.15s|" 
   "%15.15s|%15.15s|%15.15s|\n", 

   "Queries", "Hits", "Miss", "ShiftedQuery", "SentShiftedQuery",
   "TotalExecTime", "MaxExecTime", "AveExecTime", "AveWaitTime"
 );
 RULER ();
}

/*
 * 
 */
void print_out (void) {
 printf (
   "|%15" PRIu64 "|%15" PRIu64 "|%15" PRIu64 "|%15" PRIu64 "|%15" PRIu64
   "|%15.5LE|%15.5LE|%15.5LE|%15.5LE|\n",

    numQuery, TotalCacheHit, TotalCacheMiss, shiftedQuery, SentShiftedQuery,
   ((long double)TotalExecTime) / 1000000.0, 
   ((long double)MaxExecTime)   / 1000000.0,
   ((long double)AveExecTime)   / 1000000.0,
   ((long double)AveWaitTime)   / 1000000.0
 );
}

/*
 * 
 */
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

 if (listen (sock, nservers + 1) == -1) {
  perror ("Listen");
  exit (EXIT_FAILURE);
 }

 cout << "TCPServer Waiting for client on port: " << PORT << endl;
}

/*
 * 
 */
void catchSignal (int Signal) {
 cerr << "Closing sockets & files.\t Signal: " <<  strsignal(Signal) << endl;
 longjmp (finish, 1);
 close (sock);
 exit (EXIT_FAILURE);
}


int main (int argc, char** argv) {
 int c;
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
 connected = new int16_t [nservers];
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
 if (setjmp (finish)) goto end; 
 { 
   int j = 0;
   for (uint64_t i = 0; i < data_interval; i += (data_interval/nservers)) {
     backend [j++] = new Node (ALPHA, i + (data_interval/nservers)/2);
   }
 }  

 //! Main loop which use the given algorithm to send the queries
 for (char line[100]; fgets(line, 100, stdin) != NULL && cnt < nqueries; cnt++)
 {
  int selected = 0;
  uint64_t point = prepare_input (line);
  double minDist = DBL_MAX;
  
  //usleep (50);

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

  //cout << "NODE: " << selected;
  //cout << " EMA: " <<  backend[selected]->get_EMA ();
  //cout << " SENDING: " << point << " LOW: " << backend[selected]->get_low() << " UPP: " << backend[selected]->get_upp() << endl;

  packet toSend (point);
  toSend.low_b = backend [selected]->get_low();
  toSend.upp_b = backend [selected]->get_upp();
  toSend.EMA   = backend [selected]->get_EMA();

  send_msg (connected[selected], "QUERY");
  send (connected[selected], &toSend, sizeof (packet), 0);

  if ((cnt + 1) % step == 0) {
   sleep (1);
   receive_all ();
   print_out ();
  }
 }
 RULER ();

 //! Delete dynamic objects and say bye to back-end severs
 for (int i = 0; i < nservers; i++)
  send_msg (connected[i], "QUIT");  

 gettimeofday (&end, NULL);
 printf ("SchedulerTime:\t %20LE S\n", (long double) timediff (&end, &start) / 1000000.0);


end:

 //! Close sockets
 close (sock);
 for (int i = 0; i < nservers; i++) close (connected[i]);

 for (int i = 0; i < nservers; i++) delete backend[i];

 free (client_addr);
 delete[] queryCount;
 delete[] backend;

 return EXIT_SUCCESS;
}
