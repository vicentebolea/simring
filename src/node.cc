#include <uniDQP.h>
#include <pthread.h>
#include <queue>

using namespace std;

queue<Query*> Queue;
SETcache setCache(CACHESIZE); //since a BST has a O(log(n)) for searching

bool die_thread = false;
uint32_t queryRecieves = 0, queryProcessed = 0;
uint64_t hitCount = 0, missCount = 0;
uint64_t TotalExecTime = 0, TotalWaitTime = 0;
pthread_mutex_t empty = PTHREAD_MUTEX_INITIALIZER;

void* worker_run (void *ptr) {
	uint64_t* hitmiss_count[2] = {&hitCount, &missCount};

	//Condition variable
	while(die_thread != true || Queue.empty() != true) {
		while (Queue.empty() && die_thread != true) 
			pthread_mutex_lock (&empty);
		if (die_thread == true)
			continue;

		Query* query = Queue.front();

		query->setStartDate();
		setCache.match (static_cast<packet*> (query), hitmiss_count);
		query->setFinishedDate();

		queryProcessed++;
		TotalExecTime += query->getExecTime();
		TotalWaitTime += query->getWaitTime();
				
		Queue.pop();
		delete query;
	}
	pthread_exit(EXIT_SUCCESS);
}
int main (int argc, char** argv) {
	int sock, c, port;  
	struct hostent *host;
	struct sockaddr_in server_addr;  
	struct timeval start, end;;
	pthread_t worker;
 char host_str [32], data_file [256];

 while ((c = getopt (argc, argv, "h:d:p:")) != -1)
  switch (c) {
   case 'h': strncpy (host_str, optarg, 32); break;
   case 'd': strncpy (data_file, optarg, 256); break;
   case 'p': port = atoi (optarg);      break;
  }

 setCache.setDataFile (data_file);
	//Start Network setting
	host = gethostbyname (host_str);
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == FAIL) {
		perror ("Socket");
		exit (EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;     
	server_addr.sin_port = htons(port);   
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero (&(server_addr.sin_zero),8); 

	if (connect(sock, (struct sockaddr *)&server_addr,
				sizeof(struct sockaddr)) == FAIL) {
		perror ("Connect error"); exit (EXIT_FAILURE); } 	//End Network settings

	gettimeofday(&start, NULL);

	pthread_mutex_lock (&empty); /* Initialize the lock to 0 */
	pthread_create (&worker, NULL, worker_run, NULL);

	while (true) {
		char recv_data[LOT];
		recv_msg (sock, recv_data);

		if (strcmp(recv_data, "QUERY") == OK) {
			Query* aux = new Query();
			if (sizeof(packet) != recv(sock, static_cast<packet*>(aux), sizeof(packet), MSG_WAITALL))
				perror("Receiving data");
		
			Queue.push (aux);
			queryRecieves++;
			pthread_mutex_unlock (&empty);

		} else if (strcmp (recv_data , "INFO") == OK){
			char send_data[LOT] = "", tmp[256];
			while (Queue.empty () != true)
				sleep(1);

			//make variables such as hit Count...
			sprintf (tmp, "CacheHit=%"				PRIu64 "\n", hitCount);
			strcat (send_data, tmp);
			sprintf (tmp, "CacheMiss=%" 			PRIu64 "\n", missCount);
			strcat (send_data, tmp);
			sprintf (tmp, "QueryCount=%" 		PRIu32 "\n", queryProcessed);
			strcat (send_data, tmp);
			sprintf (tmp, "TotalExecTime=%" 	PRIu64 "\n", TotalExecTime);
			strcat (send_data, tmp);
			sprintf (tmp, "TotalWaitTime=%" 	PRIu64 "\n", TotalWaitTime);
			strcat (send_data, tmp);

			send(sock, send_data, LOT, 0);

		} else if (strcmp(recv_data , "QUIT") == OK) {
			die_thread = true;
			pthread_mutex_unlock (&empty);
			pthread_join (worker, NULL);
			break;

		} else {
			cerr << "Unknown message received." << endl;
			exit (EXIT_FAILURE);
		}
	}

	gettimeofday (&end, NULL);
	cout.width (20);
	cout.fill ();
 cout.flags (ios::scientific);

	cout << "-------------------------------" << endl;
	cout << "Recieved: "  << queryRecieves << endl;
	cout << "Processed: " << queryProcessed << endl;
	cout << "CacheHits: " << hitCount << endl;
	cout << "TotalExecTime: " << TotalExecTime << " nanoSeconds" << endl;
	cout << "TotalWaitTime: " << TotalWaitTime << " nanoSeconds" << endl;
	cout << "TotalTime: " << timediff(&end, &start) << " nanoSeconds" << endl;
	close(sock);
}
