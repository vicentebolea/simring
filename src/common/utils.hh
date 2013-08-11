#ifndef __UTILS_HH_
#define __UTILS_HH_

#include <macros.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

#define M_ERR     "[\e[31mERROR\e[0m]"   //! RED COLOR
#define M_WARN    "[\e[35mWARN\e[0m]"    //! MAGENTA COLOR 
#define M_DEBUG   "[\e[32mDEBUG\e[0m]"   //! GREEN COLOR 
#define M_INFO    "[\e[34mINFO\e[0m]"    //! BLUE COLOR

void     log           (const char*, const char* _ip, const char* in, ...);
uint64_t timediff      (struct timeval*, struct timeval*);
void     send_msg      (int, const char*);
void     recv_msg      (int, char*) __attribute__((weak));
int      poisson       (double);
int64_t  hilbert       (int64_t n, int64_t x, int64_t y);
uint64_t prepare_input (char* in);
char*    get_ip        (const char*);

#endif
