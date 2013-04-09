#include <uniDQP.h>
#include <pthread.h>
#include <queue>

using namespace std;

queue<Query> queue_scheduler;
queue<Query> queue_neighbor;
LRUcache cache(CACHESIZE); 

int sock, port;  
bool die_thread = false;
uint32_t queryRecieves = 0, queryProcessed = 0;
uint64_t hitCount = 0, missCount = 0;
uint64_t TotalExecTime = 0, TotalWaitTime = 0;

pthread_mutex_t mutex_scheduler = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_neighbor  = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_disk;
pthread_t thread_neighbor;

/*
 *
 */
void thread_func_main (void) {
 while (true) {
  char recv_data[LOT];
  recv_msg (sock, recv_data);

  //When a new query arrive
  if (strcmp(recv_data, "QUERY") == OK) {
   Query* aux = new Query();
   if (sizeof(packet) != recv(sock, static_cast<packet*>(aux), sizeof(packet), MSG_WAITALL))
    perror ("Receiving data");

   queue_scheduler.push (*aux);
   queryRecieves++;
   delete aux;

   pthread_mutex_unlock (&mutex_scheduler);

  //When it ask for information
  } else if (strcmp (recv_data , "INFO") == OK) {
   char send_data[LOT] = "", tmp[256];
   while (queue_scheduler.empty () != true)
    sleep (1);

   sprintf (tmp, "CacheHit=%"      PRIu64 "\n", hitCount);
   strcat (send_data, tmp);
   sprintf (tmp, "CacheMiss=%"     PRIu64 "\n", missCount);
   strcat (send_data, tmp);
   sprintf (tmp, "QueryCount=%"    PRIu32 "\n", queryProcessed);
   strcat (send_data, tmp);
   sprintf (tmp, "TotalExecTime=%" PRIu64 "\n", TotalExecTime);
   strcat (send_data, tmp);
   sprintf (tmp, "TotalWaitTime=%" PRIu64 "\n", TotalWaitTime);
   strcat (send_data, tmp);

   send (sock, send_data, LOT, 0);

  //In case that we need to finish the execution 
  } else if (strcmp (recv_data , "QUIT") == OK) {
   die_thread = true;
   pthread_mutex_unlock (&mutex_scheduler);
   pthread_join (worker, NULL);
   break;

  } else {
   cerr << "Unknown message received." << endl;
   exit (EXIT_FAILURE);
  }
 }
}

/*
 *
 */
void* thread_func_disk (void* argv) {
 while (die_thread != true || queue_scheduler.empty() != true) {
  while (queue_scheduler.empty() && die_thread != true) 
   pthread_mutex_lock (&empty);
  if (die_thread == true)
   continue;

  Query* query = queue_scheduler.front();

  query->setStartDate();
  cache.match (static_cast<packet*> (query), hitCount, missCount);
  query->setFinishedDate();

  queryProcessed++;
  TotalExecTime += query->getExecTime();
  TotalWaitTime += query->getWaitTime();

  queue_scheduler.pop();
 }
 pthread_exit (EXIT_SUCCESS);
}

/*
 *
 */
void* thread_func_neighbor (void* argv) {

 pthread_exit (EXIT_SUCCESS);   
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
//------------------------------------------------------------------//
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

 pthread_create (&thread_disk, NULL, thread_func_disk, NULL);
 pthread_create (&thread_neighbor, NULL, thread_func_neighbor, NULL);

 thread_func_main ();

 gettimeofday (&end, NULL);
 cout.width (20);
 cout.fill ();
 cout.flags (ios::scientific);

 cout << "-------------------------------" << endl;
 cout << "Recieved: "  << queryRecieves << " queries" << endl;
 cout << "Processed: " << queryProcessed << " queries" << endl;
 cout << "CacheHits: " << hitCount << " diskPages" << endl;
 cout << "TotalExecTime: " << TotalExecTime << " 10E-6 s" << endl;
 cout << "TotalWaitTime: " << TotalWaitTime << " 10E-6 s" << endl;
 cout << "TotalTime: " << timediff(&end, &start) << " 10E-6 s" << endl;

 close (sock);

 return EXIT_SUCCESS;
}
