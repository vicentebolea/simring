#include <uniDQP.h>
#include <pthread.h>
#include <queue>

using namespace std;

queue<Query> queue_scheduler;
queue<Query> queue_neighbor;
LRUcache cache (CACHESIZE); 

int sock, port;  
bool die_thread = false;
uint32_t queryRecieves = 0, queryProcessed = 0;
uint64_t hitCount = 0, missCount = 0;
uint64_t TotalExecTime = 0, TotalWaitTime = 0;

pthread_cond_t cond_scheduler_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_neighbor_empty  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_scheduler = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_neighbor  = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_disk;
pthread_t thread_neighbor;
pthread_t thread_scheduler;

/*
 *
 */
void thread_func_scheduler (void* argv) {
 while (true) {
  char recv_data[LOT];
  recv_msg (sock, recv_data);

  //When a new query arrive
  if (strcmp (recv_data, "QUERY") == OK) {
   Query aux;
   int ret = recv (sock, static_cast<packet*>(&aux), sizeof packet, MSG_WAITALL);
   if (ret != sizeof packet)
    perror ("Receiving data");

   queue_scheduler.push (aux);
   queryRecieves++;

   pthread_mutex_unlock (&mutex_scheduler);

  //When it ask for information
  } else if (strcmp (recv_data, "INFO") == OK) {
   char send_data [LOT] = "", tmp [256];
   while (!queue_scheduler.empty ())
    sleep (1);

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
   pthread_exit (EXIT_SUCCESS);

  } else {
   cerr << "Unknown message received." << endl;
   pthread_exit (EXIT_SUCCESS);
  }
 }
}

/*
 *
 */
void* thread_func_disk (void* argv) {
 while (!die_thread) 
 {
  pthread_mutex_lock (&mutex_scheduler);      //Waiting for producer 1
  while (queue_scheduler.empty()) 
   pthread_cond_wait (&cond_scheduler_empty, &mutex_scheduler);

  pthread_mutex_lock (&mutex_neighbor);       //Waiting for producer 2
  while (queue_neighbor.empty()) 
   pthread_cond_wait (&cond_neighbor_empty, &mutex_neighbor);

  if (die_thread) break;

  //--------------Start of the critical section--------------------//
  Query* query = queue_scheduler.front();                          //
                                                                   //
  query->setStartDate();                                           //
  cache.match (static_cast<packet*> (query), hitCount, missCount); //
  query->setFinishedDate();                                        //
                                                                   //
  queryProcessed++;                                                //
  TotalExecTime += query->getExecTime();                           //
  TotalWaitTime += query->getWaitTime();                           //
                                                                   //
  queue_scheduler.pop();                                           //
  //-------------End of the crtical section------------------------//

  pthread_mutex_unlock (&empty_neighbor);
  pthread_mutex_unlock (&empty_scheduler);
 }
}

/*
 *
 */
void* thread_func_neighbor (void* argv) {

}

/*
 *
 */
void setup_network (char* host_str) {
 struct sockaddr_in server_addr;  
 struct hostent *host;

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

int main (int argc, char** argv) {
 int c;  
 struct timeval start, end;
 char host_str [32], data_file [256];

 while ((c = getopt (argc, argv, "h:d:p:")) != -1)
  switch (c) {
   case 'h': strncpy (host_str, optarg, 32);   break;
   case 'd': strncpy (data_file, optarg, 256); break;
   case 'p': port = atoi (optarg);             break;
  }

 cache.setDataFile (data_file);
 setup_network (host_str);
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
