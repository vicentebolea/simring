/*
	* @file This file contains the source code of the application 
	*       which will run in each server 
	*
	*
	*/
#include <node.hh>
#include <simring.hh>
#include <queue>
#include <err.h>
#define CACHESIZE 1000

queue<Query> queue_scheduler;
queue<Query> queue_neighbor;
SETcache cache (CACHESIZE); 
int sock_scheduler, sock_left, sock_right, sock_server;  

bool panic = false;

uint32_t queryRecieves = 0;
uint32_t queryProcessed = 0;
uint64_t hitCount = 0;
uint64_t missCount = 0;
uint64_t TotalExecTime = 0;
uint64_t TotalWaitTime = 0;
uint64_t shiftedQuery = 0;
uint64_t SentShiftedQuery = 0;

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
void * thread_func_scheduler (void * argv) {
	for (char recv_data [LOT]; !panic; bzero (&recv_data, LOT)) {
		recv_msg (sock_scheduler, recv_data);

		//! When a new query arrive
		if (strcmp (recv_data, "QUERY") == OK) {
			Query query;

			query.setScheduledDate ();

			int ret = recv (sock_scheduler, &query, sizeof(packet), 0);
			if (ret != sizeof (packet)) warn ("[NODE] Receiving data size=%i != %i", ret, sizeof (packet));

			query.setStartDate ();                                           
			bool rt = cache.match (query);
			query.setFinishedDate ();                                        

			if (rt == true) hitCount++; else missCount++;

			queryProcessed++;                                                
			queryRecieves++;

			TotalExecTime += query.getExecTime ();                            
			TotalWaitTime += query.getWaitTime ();                           

			//! When it ask for information
		} else if (strcmp (recv_data, "INFO") == OK) {
			char send_data [LOT] = "", tmp [256];
			struct timeval timeout = {1, 0};

			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(sock_server, &readSet);

			//while ((select(sock_server+1, &readSet, NULL, NULL, &timeout) >= 0) && FD_ISSET(sock_server, &readSet));

			sprintf (tmp, "CacheHit=%"         PRIu64 "\n", hitCount);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "CacheMiss=%"        PRIu64 "\n", missCount);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "QueryCount=%"       PRIu32 "\n", queryProcessed);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "TotalExecTime=%"    PRIu64 "\n", TotalExecTime);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "TotalWaitTime=%"    PRIu64 "\n", TotalWaitTime);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "TotalWaitTime=%"    PRIu64 "\n", TotalWaitTime);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "shiftedQuery=%"     PRIu64 "\n", shiftedQuery);
			strncat (send_data, tmp, 256);
			sprintf (tmp, "SentShiftedQuery=%" PRIu64 "\n", SentShiftedQuery);
			strncat (send_data, tmp, 256);

			_send (sock_scheduler, send_data, LOT, 0);

			//! In case that we need to finish the execution 
		} else if (strcmp (recv_data, "QUIT") == OK) {
			panic = true;
			sleep (1);

		} else {
			char host_name [256];
			gethostname (host_name, 256);
			warn ("Unknown message received [ID: %s] [MSG: %s]", host_name, recv_data);
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
void * thread_func_neighbor (void* argv) {
	socklen_t s = sizeof (sockaddr);
	struct sockaddr_in* addr = (struct sockaddr_in*)argv;
	assert (addr->sin_family == AF_INET);

	while (!panic) {
		diskPage dp;
		struct timeval timeout = {1, 0};

		fd_set readSet;
		FD_ZERO (&readSet);
		FD_SET (sock_server, &readSet);

		if ((select(sock_server+1, &readSet, NULL, NULL, &timeout) >= 0) && FD_ISSET(sock_server, &readSet)) {

			int ret = recvfrom (sock_server, &dp, sizeof (diskPage), 0, (sockaddr*)addr, &s);

			if (ret != sizeof (diskPage) && ret != -1) err (EXIT_FAILURE, "[NODE] Receiving data");
			if (ret == -1) { continue; }

			if (cache.is_valid (dp)) shiftedQuery++;
		}
	}
	pthread_exit (EXIT_SUCCESS);
}

/*
	* @brief
	* @param 
	* @param 
	*/
void * thread_func_forward (void * argv) {
	socklen_t s = sizeof (struct sockaddr);	
	struct sockaddr_in* addr_left = *((struct sockaddr_in**)argv + 0);
	struct sockaddr_in* addr_right = *((struct sockaddr_in**)argv + 1);

	while (!panic) {
		if (!cache.queue_lower.empty ()) {

			diskPage DP = cache.get_low ();
			sendto (sock_left, &DP, sizeof (diskPage), 0, (sockaddr*)addr_left, s);
			SentShiftedQuery++;
		}

		if (!cache.queue_upper.empty ()) {

			diskPage DP = cache.get_upp ();
			sendto (sock_right, &DP, sizeof (diskPage), 0, (sockaddr*)addr_right, s); 
			SentShiftedQuery++;
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
void setup_server_peer (int port, int* sock, sockaddr_in* addr) {
	socklen_t s = sizeof (sockaddr);
	EXIT_IF (*sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP), "SOCKET");

	addr->sin_family      = AF_INET;
	addr->sin_port        = htons (port + 1);
	addr->sin_addr.s_addr = htonl (INADDR_ANY);
	bzero (&(addr->sin_zero), 8);

	EXIT_IF (bind (*sock, (sockaddr*)addr, s), "BIND PEER");
}

/*
	* @brief
	* @param 
	* @param 
	*/
void setup_client_peer (const int port, const char* host, int* sock, sockaddr_in* addr) {

	EXIT_IF (*sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP), "SOCKET");

	addr->sin_family      = AF_INET;
	addr->sin_port        = htons (port + 1);
	addr->sin_addr.s_addr = inet_addr (host);
	bzero (&(addr->sin_zero), 8);
}


/*
	* @brief
	* @param 
	* @param 
	*/
void setup_client_scheduler (int port, const char* host, int* sock) {
	struct sockaddr_in server_addr;  
	socklen_t s = sizeof (sockaddr);

	EXIT_IF (*sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP), "SOCKET SCHEDULER");

	server_addr.sin_family      = AF_INET;
	server_addr.sin_port        = htons (port);
	server_addr.sin_addr.s_addr = inet_addr (host);
	bzero (&(server_addr.sin_zero), 8);

	EXIT_IF (connect (*sock, (sockaddr*)&server_addr, s), "CONNECT SCHEDULER");
}

/*
	* @brief parse the command line options
	* @param number or args
	* @param array of args 
	*/
void parse_args (int argc, const char** argv, Arguments* args) {
	int c = 0;  
	do {
		switch (c) {
			case 'h': strncpy (args->host_str, optarg, 32);   break;
			case 'r': strncpy (args->peer_right, optarg, 32); break;
			case 'l': strncpy (args->peer_left, optarg, 32);  break;
			case 'd': strncpy (args->data_file, optarg, 256); break;
			case 'p': args->port = atoi (optarg);             break;
		}
		c = getopt (argc, const_cast<char**> (argv), "h:d:p:r:l:");
	} while (c != -1);

	// Check if everything was set
	if (!args->host_str || !args->data_file || !args->port)
		err (EXIT_FAILURE, "PARSER: Arguments needs to be setted");
}

void catch_signal (int arg) {
	close_all ();
	err (EXIT_SUCCESS, "Sockets closed for security");
}

void close_all () {
	close (sock_scheduler);
	close (sock_left);
	close (sock_right);
	close (sock_server);
}
