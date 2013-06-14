/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 */
#pragma once
#ifndef __NODE_HH_
#define __NODE_HH_

#include <iostream>
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>

#ifndef __GNUC__
#error "Required GCC"
#endif

#ifdef _DEBUG
#define WEAK __attribute__((weak))
#else
#define WEAK
#endif

using namespace std;

#define EXIT_IF(x,m) if ((x) == -1) {error (EXIT_FAILURE, errno, (m));}

extern int sock, sock_left, sock_right, port;  
extern bool die_thread;
extern bool panic;

extern uint32_t queryRecieves;
extern uint32_t queryProcessed;
extern uint64_t hitCount;
extern uint64_t missCount;
extern uint64_t TotalExecTime;
extern uint64_t TotalWaitTime;

extern char host_str [32];
extern char data_file [256];
extern char peer_right [32];
extern char peer_left [32];

extern pthread_cond_t cond_scheduler_empty;
extern pthread_cond_t cond_scheduler_full;
extern pthread_cond_t cond_neighbor_empty;
extern pthread_mutex_t mutex_scheduler;
extern pthread_mutex_t mutex_neighbor;

extern pthread_t thread_disk;
extern pthread_t thread_neighbor;
extern pthread_t thread_scheduler;

extern ssize_t (*_recv)     (int, void*, size_t, int);
extern ssize_t (*_send)     (int, const void*, size_t, int);
extern int (*_connect)      (int, const struct sockaddr*, socklen_t);

#ifdef _DEBUG
ssize_t recv_mock           (int, void*, size_t, int);
ssize_t sendto_mock         (int, const void*, size_t, int);
ssize_t recvfrom_mock       (int, const void*, size_t, int);
ssize_t send_mock           (int, const void*, size_t, int);
int connect_mock            (int, const struct sockaddr*, socklen_t);
void parse_args             (int, char**);
#endif

void* thread_func_scheduler (void*) WEAK;
void* thread_func_neighbor  (void*) WEAK;
void* thread_func_disk      (void*) WEAK;
void* thread_func_forward   (void*) WEAK;

void setup_server_peer      (int) WEAK;
void setup_client_peer      (const int, const char*, const char*) WEAK;
void setup_client_scheduler (const char*) WEAK;
void parse_args             (int, const char**) WEAK;
void close_all              (void) WEAK;
void catch_signal           (int);

#endif
