#ifndef __NODE_HH_
#define __NODE_HH_

#include <dht.hh>
#include <simring.hh>
#include <SETcache.hh>

#include <iostream>
#include <inttypes.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <error.h>
#include <signal.h>

#ifndef CACHESIZE
#define CACHESIZE 1000
#endif 

using namespace std;

class Node {
 protected:
  SETcache* cache;
  DHT dht;
  int sock_scheduler, sock_left, sock_right, sock_server;  
  int sch_port, peer_port, dht_port, local_no, nservers;
  char *local_ip, peer_left [32], peer_right[32], host_str [128], data_file [128];
  bool panic, color;

  uint32_t queryRecieves;
  uint32_t queryProcessed;
  uint64_t hitCount;
  uint64_t missCount;
  uint64_t TotalExecTime;
  uint64_t TotalWaitTime;
  uint64_t ReceivedData;
  uint64_t RequestedData;
  uint64_t shiftedQuery;
  uint64_t SentShiftedQuery;

  pthread_t thread_neighbor;
  pthread_t thread_scheduler;
  pthread_t thread_forward;
  pthread_t thread_dht;

  struct sockaddr_in addr_left, addr_right, addr_server;

  void* func_dht       () WEAK;
  void* func_scheduler () WEAK;
  void* func_neighbor  () WEAK;
  void* func_forward   () WEAK;

  void setup_server_peer      (int, int*, struct sockaddr_in*) WEAK;
  void setup_client_peer      (const int, const char*, int*, struct sockaddr_in*) WEAK;
  void setup_client_scheduler (int, const char*, int*) WEAK;
  void parse_args             (int, const char**) WEAK;
  void close_all              (void) WEAK;
  Node () {}                  //! For singleton

 public:
  void setup (int, const char**, const char*);
  ~Node ();

  bool run ();
  bool join ();

  /*******************LOW LEVEL FUNCTIONS FOR MULTITHREADING************************************/
	static Node* instance;
	static Node& get_instance (int argc, const char ** argv, const char * ifa) {
    if (Node::instance == NULL) 
     Node::instance = new Node ();

		Node& node = *Node::instance;
		node.setup (argc, argv, ifa);
		return node;
	}

	static Node& get_instance () {
    if (Node::instance == NULL) Node::instance = new Node ();
		return *Node::instance;
	}

	void         catch_signal          (int);
	static void  signal_handler        (int arg)       { (*Node::instance).catch_signal (arg); }
	static void* thread_func_dht       (void* context) { ((Node*)context)->func_dht(); return NULL;}
	static void* thread_func_scheduler (void* context) { ((Node*)context)->func_scheduler(); return NULL;}
	static void* thread_func_neighbor  (void* context) { ((Node*)context)->func_neighbor(); return NULL;}
	static void* thread_func_forward   (void* context) { ((Node*)context)->func_forward(); return NULL;}

};

#endif
