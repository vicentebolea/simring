/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 */

#include <uniDQP.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <queue>

#ifndef __GNUC__
#error "Required GCC"
#endif

#ifdef _DEBUG
#define WEAK __attribute__((weak))
#else
#define WEAK
#endif

using namespace std;

#define CHECK ((x), (m)) ((x) == -1) && error (EXIT_FAILURE, errno, (m));

queue<Query> queue_scheduler;
queue<Query> queue_neighbor;
LRUcache cache (CACHESIZE); 

int sock, sock_left, sock_right, port;  
bool die_thread = false, panic = false;
uint32_t queryRecieves = 0, queryProcessed = 0;
uint64_t hitCount = 0, missCount = 0;
uint64_t TotalExecTime = 0, TotalWaitTime = 0;

char host_str [32], data_file [256];
char peer_right [32], peer_left [32];

pthread_cond_t cond_scheduler_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_scheduler_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_neighbor_empty  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_scheduler     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_neighbor      = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_disk;
pthread_t thread_neighbor;
pthread_t thread_scheduler;

#ifdef _DEBUG
/* Mock functions*/
ssize_t recv_mock (int, void*, size_t, int);
ssize_t send_mock (int, const void*, size_t, int);

ssize_t (*_recv) (int, void*, size_t, int)       = recv_mock;
ssize_t (*_send) (int, const void*, size_t, int) = send_mock;

#else

ssize_t (*_recv) (int, void*, size_t, int) = recv;
ssize_t (*_send) (int, const void*, size_t, int) = send;

#endif

void* thread_func_scheduler (void*) WEAK;
void* thread_func_neighbor  (void*) WEAK;
void* thread_func_disk      (void*) WEAK;

bool query_send_peer        (packet&) WEAK;

void setup_server_peer      (int) WEAK;
void setup_client_peer      (const int, const char*, const char*) WEAK;
void setup_client_scheduler (const char*) WEAK;
void parse_args             (int, const char**) WEAK;
