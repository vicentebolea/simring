/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 *
 *
 */
#include <node.hh>
#include <simring.hh>
#include <queue>

queue<Query> queue_scheduler;
queue<Query> queue_neighbor;
SETcache cache (CACHESIZE); 

int sock, sock_left, sock_right, port = 0, sock_server;  
int lower_boundary;
int upper_boundary;
bool die_thread = false;
bool panic = false;

uint32_t queryRecieves = 0;
uint32_t queryProcessed = 0;
uint64_t hitCount = 0;
uint64_t missCount = 0;
uint64_t TotalExecTime = 0;
uint64_t TotalWaitTime = 0;
uint64_t shiftedQuery = 0;
uint64_t SentShiftedQuery = 0;

pthread_cond_t  cond_scheduler_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_scheduler_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_neighbor_empty  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_scheduler      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_neighbor       = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_disk;
pthread_t thread_neighbor;
pthread_t thread_scheduler;
pthread_t thread_forward;

char host_str [32]   = {0};
char data_file [256] = {0};
char peer_right [32] = {0};
char peer_left [32]  = {0};

struct sockaddr_in addr_left_peer; 
struct sockaddr_in addr_right_peer;  
struct sockaddr_in sa_server_peer;

ssize_t (*_recv) (int, void*, size_t, int) = recv;
ssize_t (*_send) (int, const void*, size_t, int) = send;
ssize_t (*_sendto) (int, const void*, size_t, int) = send;
int (*_connect) (int, const struct sockaddr*, socklen_t) = connect;

//---------------------------------------------------------------------//
//-----------END OF VARIABLES, FUNTIONS DEFINITIONS--------------------//
//---------------------------------------------------------------------//


/*
 * @brief  Thread function to receive queries from the scheduler.
 *         This function can be seen as one of the producers.
 * @args   Dummy parameter
 *
 */
void* thread_func_scheduler (void* argv) {
	cache.setDataFile (data_file);

	for (char recv_data [LOT]; !panic; bzero (&recv_data, LOT)) {
		recv_msg (sock, recv_data);

		//! When a new query arrive
		if (strcmp (recv_data, "QUERY") == OK) {
			Query query;

      query.setScheduledDate();

			int ret = recv (sock, &query, sizeof(packet), MSG_WAITALL);
			if (ret != sizeof (packet))
				perror ("Receiving data");

			pthread_mutex_lock (&mutex_scheduler);

		  query.setStartDate ();                                           
  	  bool rt = cache.match (query.get_point (), query.EMA, query.low_b, query.upp_b);
		  query.setFinishedDate();                                        
      
		  if (rt == true) hitCount++; else missCount++;

		  queryProcessed++;                                                
			queryRecieves++;
      
		  TotalExecTime += query.getExecTime();                           
		  TotalWaitTime += query.getWaitTime();                           

			pthread_mutex_unlock (&mutex_scheduler);

			//When it ask for information
		} else if (strcmp (recv_data, "INFO") == OK) {
			char send_data [LOT] = "", tmp [256];

			sprintf (tmp, "CacheHit=%"      PRIu64 "\n", hitCount);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "CacheMiss=%"     PRIu64 "\n", missCount);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "QueryCount=%"    PRIu32 "\n", queryProcessed);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "TotalExecTime=%" PRIu64 "\n", TotalExecTime);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "TotalWaitTime=%" PRIu64 "\n", TotalWaitTime);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "TotalWaitTime=%" PRIu64 "\n", TotalWaitTime);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "shiftedQuery=%"  PRIu64 "\n", shiftedQuery);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "SentShiftedQuery=%"  PRIu64 "\n", SentShiftedQuery);
			strncat (send_data, tmp, 256);

			_send (sock, send_data, LOT, 0);

			//In case that we need to finish the execution 
		} else if (strcmp (recv_data, "QUIT") == OK) {
			panic = true;
		  sleep (1);

		} else {
			fprintf (stderr, "Unknown message received\n");
			panic = true;
		}
	}
	pthread_exit (EXIT_SUCCESS);
}


/*
 * @brief  Thread function to receive queries from the scheduler.
 *         This function can be seen as one of the producers.
 * @args   Dummy parameter
 *
 */
void* thread_func_neighbor (void* argv) {
	socklen_t s = sizeof (sockaddr);

	while (!panic) {
		Query query;

		int ret = recvfrom (sock_server, &query, sizeof (packet), MSG_DONTWAIT,
				(sockaddr*)&sa_server_peer, &s);

		if (ret != sizeof (packet) && ret != -1) perror ("Receiving data");
  	if (ret == -1) { continue; }

		pthread_mutex_lock (&mutex_scheduler);

    //--------------Start measuring the time------------------//
  	query.setStartDate ();                                           
  	cache.match (query.get_point (), query.EMA, query.low_b, query.upp_b);
  	query.setFinishedDate();                                        
  	//--------------Stop  measuring the time------------------//

    shiftedQuery++;

		pthread_mutex_unlock (&mutex_scheduler);
	}
	pthread_exit (EXIT_SUCCESS);
}

/*
 * @brief
 * @param 
 * @param 
 */
void * thread_func_forward (void * arg) {
	socklen_t s = sizeof (struct sockaddr);	
  
  while (!panic) {
    while (!cache.queue_lower.empty ()) {

      diskPage& DP = cache.queue_lower.front ();
		  if (sizeof (diskPage) == sendto (sock_left, &DP, sizeof (diskPage), 0, (sockaddr*)&addr_left_peer, s))
        SentShiftedQuery++;
      cache.queue_lower.pop ();
    }

    while (!cache.queue_upper.empty ()) {

      diskPage& DP = cache.queue_upper.front ();
		  if (sizeof (diskPage) == sendto (sock_right, &DP, sizeof (diskPage), 0, (sockaddr*)&addr_right_peer, s));
        SentShiftedQuery++;
      cache.queue_upper.pop ();
    }
  }
	pthread_exit (EXIT_SUCCESS);
}

//---------------------------------------------------------------------//
//-----------SETTING UP FUNCTIONS--------------------------------------//
//---------------------------------------------------------------------//

/*
 * @brief
 * @param 
 * @param 
 */
void setup_server_peer (int port) {
	EXIT_IF (sock_server = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP), "SOCKET");

	sa_server_peer.sin_family      = AF_INET;
	sa_server_peer.sin_port        = htons (port);
	sa_server_peer.sin_addr.s_addr = htonl (INADDR_ANY);
	bzero (&(sa_server_peer.sin_zero), 8);
}

/*
 * @brief
 * @param 
 * @param 
 */
void setup_client_peer (const int port, const char* left, const char* right) {

	EXIT_IF (sock_left = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP), "SOCKET");
	EXIT_IF (sock_right = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP), "SOCKET");

	addr_left_peer.sin_family      = AF_INET;
	addr_left_peer.sin_port        = htons (port);
	addr_left_peer.sin_addr.s_addr = inet_addr (left);
	bzero (&(addr_left_peer.sin_zero), 8);

	addr_right_peer.sin_family      = AF_INET;
	addr_right_peer.sin_port        = htons (port);
	addr_right_peer.sin_addr.s_addr = inet_addr (right);
	bzero (&(addr_right_peer.sin_zero), 8);
}


/*
 * @brief
 * @param 
 * @param 
 */
void setup_client_scheduler (const char* host_str) {
	struct sockaddr_in server_addr;  
	socklen_t s = sizeof (sockaddr);

	EXIT_IF (sock = socket (AF_INET, SOCK_STREAM, 0), "SOCKET SCHEDULER");

	server_addr.sin_family      = AF_INET;
	server_addr.sin_port        = htons (port);
	server_addr.sin_addr.s_addr = inet_addr (host_str);
	bzero (&(server_addr.sin_zero), 8);

	EXIT_IF (connect (sock, (sockaddr*)&server_addr, s), "CONNECT SCHEDULER");
  char hostname [128];
  gethostname (hostname, 128);
  cout << "[HN: " << hostname << "] Linked with the scheduler"<<endl;
}

/*
 * @brief parse the command line options
 * @param number or args
 * @param array of args 
 */
void parse_args (int argc, const char** argv) {
	int c = 0;  
	do {
		switch (c) {
			case 'h': strncpy (host_str, optarg, 32);   break;
			case 'r': strncpy (peer_right, optarg, 32); break;
			case 'l': strncpy (peer_left, optarg, 32);  break;
			case 'd': strncpy (data_file, optarg, 256); break;
			case 'p': port = atoi (optarg);             break;
		}
		c = getopt (argc, const_cast<char**> (argv), "h:d:p:r:l:");
	} while (c != -1);

	// Check if everything was set
	if (!host_str || !peer_right || !peer_left || !data_file || !port)
		error (EXIT_FAILURE, errno, "PARSER: Arguments needs to be setted");
}

void catch_signal (int arg) {
 close_all();
 fprintf (stderr, "Sockets closed for security\n");
 exit (EXIT_SUCCESS);
}

void close_all () {
	close (sock);
	close (sock_left);
	close (sock_right);
	close (sock_server);
}
