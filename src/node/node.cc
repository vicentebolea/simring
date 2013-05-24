/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 *
 *
 */
#include "node.hh"

queue<Query> queue_scheduler;
queue<Query> queue_neighbor;
LRUcache cache (CACHESIZE); 

int sock, sock_left, sock_right, port, sock_server;  
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

pthread_cond_t  cond_scheduler_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_scheduler_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_neighbor_empty  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_scheduler      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_neighbor       = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_disk;
pthread_t thread_neighbor;
pthread_t thread_scheduler;

char host_str [32];
char data_file [256];
char peer_right [32];
char peer_left [32];

struct sockaddr_in addr_left_peer; 
struct sockaddr_in addr_right_peer;  
struct sockaddr_in sa_server_peer;

#ifdef _DEBUG

ssize_t (*_recv) (int, void*, size_t, int)       = recv_mock;
ssize_t (*_send) (int, const void*, size_t, int) = send_mock;
ssize_t (*_recvfrom) (int, void*, size_t, sockaddr*, socklen_t*) = recvfrom_mock;
ssize_t (*_sendto) (int, const void*, size_t, sockaddr*, socklen_t*) = sendto_mock;

#else

ssize_t (*_recv) (int, void*, size_t, int) = recv;
ssize_t (*_send) (int, const void*, size_t, int) = send;

#endif

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

		//When a new query arrive
		if (strcmp (recv_data, "QUERY") == OK) {
			Query aux;
			pthread_mutex_lock (&mutex_scheduler);

			int ret = _recv (sock, static_cast<packet*>(&aux), 
					sizeof(packet), MSG_WAITALL);
			if (ret != sizeof (packet))
				perror ("Receiving data");

			queue_scheduler.push (aux);
			queryRecieves++;

			pthread_cond_signal (&cond_scheduler_empty);
			pthread_mutex_unlock (&mutex_scheduler);

			//When it ask for information
		} else if (strcmp (recv_data, "INFO") == OK) {
			char send_data [LOT] = "", tmp [256];

			pthread_mutex_lock (&mutex_scheduler);

			while (!queue_scheduler.empty ())
				pthread_cond_wait (&cond_scheduler_full, &mutex_scheduler);

			pthread_mutex_unlock (&mutex_scheduler);

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

			_send (sock, send_data, LOT, 0);

			//In case that we need to finish the execution 
		} else if (strcmp (recv_data, "QUIT") == OK) {
			die_thread = true;
			pthread_mutex_unlock (&mutex_scheduler);
			panic = true;

		} else {
			cerr << "Unknown message received." << endl;
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
		char recv_data [LOT];
		recv_msg (sock, recv_data);

		//When a new query arrive
		if (strcmp (recv_data, "QUERY") == OK) {
			Query aux;
			pthread_mutex_lock (&mutex_neighbor);

			int ret = recvfrom (sock_server, static_cast<packet*>(&aux),
					sizeof (packet), MSG_WAITALL,
					(sockaddr*)&sa_server_peer, &s);

			if (ret != sizeof (packet)) perror ("Receiving data");

			queue_neighbor.push (aux);
			queryRecieves++;

			pthread_cond_signal (&cond_neighbor_empty);
			pthread_mutex_unlock (&mutex_neighbor);

			//When it ask for information
		} else {
			cerr << "Unknown message received from peer server" << endl;
			panic = true;
		}
	}
	pthread_exit (EXIT_SUCCESS);
}


/*
 * @brief  Function thread to compute the query.
 *         It can be seen as the 2 producters and one consumer
 *         problem. 
 *
 * @args   Dummy parameter
 */
void* thread_func_disk (void* argv) {
	while (!panic) {

		pthread_mutex_lock (&mutex_scheduler);      //Waiting for producer 1
		while (queue_scheduler.empty()) 
			pthread_cond_wait (&cond_scheduler_empty, &mutex_scheduler);

		pthread_mutex_lock (&mutex_neighbor);       //Waiting for producer 2
		while (queue_neighbor.empty()) 
			pthread_cond_wait (&cond_neighbor_empty, &mutex_neighbor);

		if (panic) break;

		//--------------Start of the critical section--------------------//
		Query query = queue_scheduler.front();                          

		query.setStartDate();                                           
		cache.match (static_cast<packet&> (query), &hitCount, &missCount); 
		query.setFinishedDate();                                        

		queryProcessed++;                                                
		TotalExecTime += query.getExecTime();                           
		TotalWaitTime += query.getWaitTime();                           

		queue_scheduler.pop();                                           
		//-------------End of the crtical section------------------------//

		queue_scheduler.empty () && pthread_cond_signal (&cond_scheduler_full);

		pthread_mutex_unlock (&mutex_neighbor);
		pthread_mutex_unlock (&mutex_scheduler);
	}
	pthread_exit (EXIT_SUCCESS);
}

/*
 * @brief
 * @param 
 * @param 
 */
bool query_send_peer (packet& p) {
	socklen_t s = sizeof (struct sockaddr);	

	if (true) //! :TODO:
		sendto (sock_left, &p, sizeof (packet), 0, (sockaddr*)&addr_left_peer, s);
	else 
		sendto (sock_right, &p, sizeof (packet), 0, (sockaddr*)&addr_right_peer, s);

	return true;
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
	sa_server_peer.sin_addr.s_addr = inet_addr (INADDR_ANY);
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

	EXIT_IF (sock = socket (AF_INET, SOCK_STREAM, 0), "SOCKET");

	server_addr.sin_family      = AF_INET;
	server_addr.sin_port        = htons (port);
	server_addr.sin_addr.s_addr = inet_addr (host_str);
	bzero (&(server_addr.sin_zero), 8);

	EXIT_IF (connect (sock, (sockaddr*)&server_addr, s), "CONNECT");
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
}
