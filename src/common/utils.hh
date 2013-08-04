#ifndef __UTILS_HH_
#define __UTILS_HH_

#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

uint64_t timediff      (struct timeval*, struct timeval*);
void     send_msg      (int, const char*);
void     recv_msg      (int, char*) __attribute__((weak));
int      poisson       (double);
int64_t  hilbert       (int64_t n, int64_t x, int64_t y);
uint64_t prepare_input (char* in);

#endif
