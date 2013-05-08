/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 *
 *
 */

#include <uniDQP.h>
#include <pthread.h>
#include <queue>

using namespace std;

queue<Query> queue_scheduler;
queue<Query> queue_neighbor;
LRUcache cache (CACHESIZE); 

int sock, sock_left, sock_right, port;  
bool die_thread = false, panic = false;
uint32_t queryRecieves = 0, queryProcessed = 0;
uint64_t hitCount = 0, missCount = 0;
uint64_t TotalExecTime = 0, TotalWaitTime = 0;

pthread_cond_t cond_scheduler_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_scheduler_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_neighbor_empty  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_scheduler = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_neighbor  = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_disk;
pthread_t thread_neighbor;
pthread_t thread_scheduler;

/*
 * @brief  Thread function to receive queries from the scheduler.
 *         This function can be seen as one of the producers.
 * @args   Dummy parameter
 *
 */
void thread_func_scheduler (void* argv) {
 
 for (char recv_data [LOT]; !panic; bzero (&recv_data, LOT)) {
  recv_msg (sock, recv_data);

  //When a new query arrive
  if (strcmp (recv_data, "QUERY") == OK) {
   Query aux;
   pthread_mutex_lock (&mutex_scheduler);

   int ret = recv (sock, static_cast<packet*>(&aux), sizeof packet, MSG_WAITALL);
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

   send (sock, send_data, LOT, 0);

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
   if (ret != sizeof packet)
    perror ("Receiving data");

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

  if (queue_empty.empty())
   pthread_cond_signal (&cond_scheduler_full);

  pthread_mutex_unlock (&empty_neighbor);
  pthread_mutex_unlock (&empty_scheduler);
 }
}


/*
 *
 */
bool query_send_peer (packet& p) {
 //
 // if (p < lower_boundary)
 //  send (sock_left);
 // else 
 //  send (sock_right);
 //
 return true;
}


/*
 *
 */
void setup_server_peer (int port) {

}


/*
 *
 */
void setup_client_peer (int port, char* left, char* right) {
 struct sockaddr_in server_addr_left;  
 struct sockaddr_in server_addr_right;  
 struct hostent* host_left, host_right;

 host_left = gethostbyname (left);
 if ((sock_left = socket (AF_INET, SOCK_STREAM, 0)) == FAIL) {
  perror ("Socket");
  exit (EXIT_FAILURE);
 }

 server_addr_left.sin_family = AF_INET;
 server_addr_left.sin_port = htons (port);
 server_addr_left.sin_addr = *((struct in_addr *)host_left->h_addr);
 bzero (&(server_addr_left.sin_zero), 8);

 if (connect(sock_left, (sockaddr*)&server_addr_left, sizeof(sockaddr)) == -1) {
  perror ("Connect error");
  exit (EXIT_FAILURE); 
 }

 host_right = gethostbyname (right);
 if ((sock_right = socket (AF_INET, SOCK_STREAM, 0)) == FAIL) {
  perror ("Socket");
  exit (EXIT_FAILURE);
 }

 server_addr_right.sin_family = AF_INET;
 server_addr_right.sin_port = htons (port);
 server_addr_right.sin_addr = *((struct in_addr *)host_right->h_addr);
 bzero (&(server_addr_right.sin_zero), 8);

 if (connect(sock_right, (sockaddr*)&server_addr_right, sizeof(sockaddr)) == -1) {
  perror ("Connect error");
  exit (EXIT_FAILURE); 
 }
}


/*
 *
 */
void setup_client_scheduler (char* host_str) {
 struct sockaddr_in server_addr;  
 struct hostent* host;

 host = gethostbyname (host_str);
 if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == FAIL) {
  perror ("Socket");
  exit (EXIT_FAILURE);
 }

 server_addr.sin_family = AF_INET;
 server_addr.sin_port = htons (port);
 server_addr.sin_addr = *((struct in_addr *)host->h_addr);
 bzero (&(server_addr.sin_zero), 8);

 if (connect(sock, (sockaddr*)&server_addr, sizeof(sockaddr)) == -1) {
  perror ("Connect error");
  exit (EXIT_FAILURE); 
 }
}

//------------------------------------------------------------------//
//-------------MAIN FUNCTION, MAIN THREAD---------------------------//
//------------------------------------------------------------------//

int main (int argc, const char** argv) {
 int c = 0;  
 struct timeval start, end;
 char host_str [32], data_file [256];
 char peer_right [32], peer_left [32];

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

 cache.setDataFile (data_file);
 setup_client_scheduler (host_str);
 gettimeofday (&start, NULL);

 pthread_mutex_lock (&mutex_scheduler); /* Initialize the lock to 0 */
 pthread_mutex_lock (&mutex_neighbor);  /* Initialize the lock to 0 */

 pthread_create (&thread_disk,      NULL, thread_func_disk,      NULL);
 pthread_create (&thread_neighbor,  NULL, thread_func_neighbor,  NULL);
 pthread_create (&thread_scheduler, NULL, thread_func_scheduler, NULL);

 pthread_join (thread_scheduler, NULL);
 pthread_join (thread_disk,      NULL);
 pthread_join (thread_neighbor,  NULL);

 gettimeofday (&end, NULL);
 cout.width (20);
 cout.fill ();
 cout.flags (ios::scientific);

 cout << "------------------------------------------" << endl;
 cout << "Recieved: "  << queryRecieves << " queries" << endl;
 cout << "Processed: " << queryProcessed << " queries" << endl;
 cout << "CacheHits: " << hitCount << " diskPages" << endl;
 cout << "TotalExecTime: " << TotalExecTime << " 10E-6 s" << endl;
 cout << "TotalWaitTime: " << TotalWaitTime << " 10E-6 s" << endl;
 cout << "TotalTime: " << timediff(&end, &start) << " 10E-6 s" << endl;

 close (sock);

 return EXIT_SUCCESS;
}
