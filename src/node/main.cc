/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 *
 *
 */
#include <node.hh>
#include <simring.hh>

#define DATA_MIGRATION

int main (int argc, const char** argv) {
 struct sockaddr_in addr_left, addr_right, addr_server;
 pthread_t thread_neighbor, thread_scheduler, thread_forward;
 struct timeval start, end;
 struct Arguments args;
 
 parse_args (argc, argv, &args);
 setup_client_scheduler (args.port, args.host_str, &sock_scheduler);
 setup_server_peer (args.port, &sock_server, &addr_server);

 if (args.peer_left)  setup_client_peer (args.port, args.peer_left, &sock_left, &addr_left);
 if (args.peer_right) setup_client_peer (args.port, args.peer_right, &sock_right, &addr_right);

 cache.setDataFile (args.data_file);
 struct sockaddr_in* addr_vec [2] = {&addr_left, &addr_right}; 

 gettimeofday (&start, NULL);
 pthread_create (&thread_scheduler, NULL, thread_func_scheduler, NULL);
#ifdef DATA_MIGRATION
 pthread_create (&thread_neighbor,  NULL, thread_func_neighbor, &addr_server);
 pthread_create (&thread_forward,   NULL, thread_func_forward, addr_vec);
#endif

 pthread_join (thread_scheduler, NULL);
#ifdef DATA_MIGRATION
 pthread_join (thread_forward,  NULL);
 pthread_join (thread_neighbor,  NULL);
#endif

 gettimeofday (&end, NULL);
 //cout.width (20);
 //cout.fill ();
 //cout.flags (ios::scientific);

 //cout << "------------------------------------------" << endl;
 //cout << "Recieved: "  << queryRecieves << " queries" << endl;
 //cout << "Processed: " << queryProcessed << " queries" << endl;
 //cout << "CacheHits: " << hitCount << " diskPages" << endl;
 //cout << "TotalExecTime: " << TotalExecTime << " 10E-6 s" << endl;
 //cout << "TotalWaitTime: " << (long double) TotalWaitTime << " 10E-6 s" << endl;
 //cout << "TotalTime: " << timediff (&end, &start) << " 10E-6 s" << endl;

 close_all ();

 return EXIT_SUCCESS;
}
