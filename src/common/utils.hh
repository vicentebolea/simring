#ifndef __UTILS_HH_
#define __UTILS_HH_

#include <macros.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

/*
 * EWMA ema (.03);
 * current = ema (45);
 */
class EWMA {
 double weight;
 double current_ewma;

 public:
  EWMA (double w) : weight (w), current_ewma (0) {}
  double operator() (double sample) {
   current_ewma = (weight * current_ewma) + ((1.0 - weight) * sample);
   return current_ewma; 
  }
};

enum m_error { M_ERR   = 0, M_WARN  = 1, M_DEBUG = 2, M_INFO  = 3};

void     log           (int, const char* _ip, const char* in, ...);
int64_t  hilbert       (int64_t n, int64_t x, int64_t y);
uint64_t prepare_input (char* in);
char*    get_ip        (const char*);
char*    byte_units    (const double);

/*
 *
 */
inline bool
fd_is_ready (int fd, int _time = 1000000) 
{
 struct timeval timeout = {0, _time};
 fd_set set;

 FD_ZERO(&set);
 FD_SET(fd, &set);
 return (select(fd+1, &set, NULL, NULL, &timeout) >= 0) && FD_ISSET(fd, &set);
}

/* * */
inline uint64_t
timediff (struct timeval *end_time, struct timeval *start_time)
{
 return  (end_time->tv_usec + (1000000 * end_time->tv_sec)) 
  - (start_time->tv_usec + (1000000 * start_time->tv_sec));
}

/* * */
inline void
send_msg (int socket, const char* send_data)
{
 int msg_len = strlen (send_data);
 send (socket, &msg_len, sizeof(int), 0); 
 send (socket, send_data, msg_len, 0); 
}

/*
 *
 */
inline void
recv_msg (int sock, char* recv_data)
{
 int nbytes, bytes_received = 0, r = 0;
 while (r != 4)
  r += recv (sock, (char*)&nbytes+r, sizeof(int) - r, MSG_WAITALL);

 // read nbytes;
 while (bytes_received < nbytes)
  bytes_received += recv (sock, recv_data+bytes_received, nbytes-bytes_received, MSG_WAITALL);

 recv_data [bytes_received] = 0;
}

//inline int
//poisson (double c)
//{ 
 //int x = 0;
 //srand (time(NULL));
//
 //for (double t = .0; t <= 1.0; x++) {
  //t -= ::log ((double)(rand()%1000) / 1000.0) / c;
 //}
 //return x;
//}

#endif
