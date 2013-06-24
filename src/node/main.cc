/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 *
 *
 */
#include <node.hh>
#include <simring.hh>

int main (int argc, const char** argv) {
 struct timeval start;
 struct timeval end;

 parse_args (argc, argv);
 setup_client_scheduler (host_str);
 setup_server_peer (port);
 setup_client_peer (port, peer_right, peer_left);
 gettimeofday (&start, NULL);

// pthread_mutex_lock (&mutex_scheduler); /* Initialize the lock to 0 */
 //pthread_mutex_lock (&mutex_neighbor);  /* Initialize the lock to 0 */

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
 cout << "TotalTime: " << timediff (&end, &start) << " 10E-6 s" << endl;

 close (sock);

 return EXIT_SUCCESS;
}
