/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 *
 *
 */
#include "node.h"

/*
 * @brief  Thread function to receive queries from the scheduler.
 *         This function can be seen as one of the producers.
 * @args   Dummy parameter
 *
 */
void* thread_func_scheduler (void* argv) {

	for (char recv_data [LOT]; !panic; bzero (&recv_data, LOT)) {
		recv_msg (sock, recv_data);

		//When a new query arrive
		if (strcmp (recv_data, "QUERY") == OK) {
			Query aux;
			pthread_mutex_lock (&mutex_scheduler);

			int ret = _recv (sock, static_cast<packet*>(&aux), sizeof packet, MSG_WAITALL);
			if (ret != sizeof packet)
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
}


/*
 * @brief  Thread function to receive queries from the scheduler.
 *         This function can be seen as one of the producers.
 * @args   Dummy parameter
 *
 */
void* thread_func_neighbor (void* argv) {
	while (!panic) {
		char recv_data [LOT];
		recv_msg (sock, recv_data);

		//When a new query arrive
		if (strcmp (recv_data, "QUERY") == OK) {
			Query aux;
			pthread_mutex_lock (&mutex_neighbor);

			int ret = recv (sock, static_cast<packet*>(&aux), sizeof packet, MSG_WAITALL);

			if (ret != sizeof packet) perror ("Receiving data");

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
		Query* query = queue_scheduler.front();                          

		query->setStartDate();                                           
		cache.match (static_cast<packet*> (query), hitCount, missCount); 
		query->setFinishedDate();                                        

		queryProcessed++;                                                
		TotalExecTime += query->getExecTime();                           
		TotalWaitTime += query->getWaitTime();                           

		queue_scheduler.pop();                                           
		//-------------End of the crtical section------------------------//

		queue_empty.empty () && pthread_cond_signal (&cond_scheduler_full);

		pthread_mutex_unlock (&empty_neighbor);
		pthread_mutex_unlock (&empty_scheduler);
	}
}

/*
 * @brief
 * @param 
 * @param 
 */
bool query_send_peer (packet& p) {
	//
	// if (p < lower_boundary)
	//  _send (sock_left);
	// else 
	//  _send (sock_right);
 //
 return true;
}

/*
 * @brief
 * @param 
 * @param 
 */
void setup_server_peer (int port) {}

/*
 * @brief
 * @param 
 * @param 
 */
void setup_client_peer (const int port, const char* left, const char* right) {
 struct sockaddr_in server_addr_left;  
 struct sockaddr_in server_addr_right;  
 size_t s = sizeof (sockaddr);

 CHECK (sock_left = socket (AF_INET, SOCK_STREAM, 0), "SOCKET");

 server_addr_left.sin_family = AF_INET;
 server_addr_left.sin_port   = htons (port);
 server_addr_left.sin_addr   = inet_addr (left);
 bzero (&(server_addr_left.sin_zero), 8);

 CHECK (connect (sock_left, (sockaddr*)&server_addr_left, s), "CONNECT");

 CHECK (sock_right = socket (AF_INET, SOCK_STREAM, 0), "SOCKET");

 server_addr_right.sin_family = AF_INET;
 server_addr_right.sin_port   = htons (port);
 server_addr_right.sin_addr   = inet_addr (right);
 bzero (&(server_addr_right.sin_zero), 8);

 CHECK (connect (sock_right, (sockaddr*)&server_addr_right, s), "CONNECT");
}


/*
 * @brief
 * @param 
 * @param 
 */
void setup_client_scheduler (const char* host_str) {
 struct sockaddr_in server_addr;  

 CHECK (sock = socket (AF_INET, SOCK_STREAM, 0), "SOCKET");

 server_addr.sin_family = AF_INET;
 server_addr.sin_port   = htons (port);
 server_addr.sin_addr   = inet_addr (host_str);
 bzero (&(server_addr.sin_zero), 8);

 CHECK (connect (sock, (sockaddr*)&server_addr, sizeof(sockaddr)), "CONNECT");
}

/*
 * @brief
 * @param 
 * @param 
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
