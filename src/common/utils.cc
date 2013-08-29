#include <utils.hh>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <cfloat>
#include <string.h>
#include <stdlib.h>

const char *error_str [20] = {
 "[\e[31mERROR\e[0m]",   //! RED COLOR
 "[\e[35mWARN\e[0m]",    //! MAGENTA COLOR 
 "[\e[32mDEBUG\e[0m]",   //! GREEN COLOR 
 "[\e[34mINFO\e[0m]"     //! BLUE COLOR
};

const char *error_str_nocolor [20] = {"[ERROR]", "[WARN]", "[DEBUG]", "[INFO]"};

void 
log (int type, const char* _ip, const char* in, ...) 
{
 va_list args;
 char output [256];
 sprintf (output, "%s\e[33m::\e[0m[\e[36m%s\e[0m]\e[1m \e[33m", error_str [type],  _ip);

 if (isatty (fileno (stdout)) || true)
   fprintf (stderr, "%s\e[33m::\e[0m[\e[36m%s\e[0m]\e[1m \e[33m", error_str [type],  _ip);
 else 
   fprintf (stderr, "%s::[%s] ", error_str_nocolor [type],  _ip);

 va_start (args, in);
 vfprintf (stderr, in, args);
 va_end (args);

 if (isatty (fileno (stdout)) || true)
   fprintf (stderr, "\e[0m\n");
 else 
   fprintf (stderr, "\n");

 if (type == M_ERR) exit (EXIT_SUCCESS);

}

//rotate/flip a quadrant appropriately
inline void
rot (int64_t n, int64_t  *x, int64_t *y, int64_t  rx, int64_t ry) 
{
 if (ry == 0) {
  if (rx == 1) {
   *x = n - 1 - *x;
   *y = n - 1 - *y;
  }

  //Swap x and y
  int64_t t  = *x;
  *x = *y;
  *y = t;
 }
}

int64_t
hilbert (int64_t n, int64_t x, int64_t y) 
{
 int64_t rx, ry, s, d = 0;
 for (s = n / 2; s > 0; s /= 2) {
  rx = (x & s) > 0;
  ry = (y & s) > 0;
  d += s * s * ((3 * rx) ^ ry);
  rot (s, &x, &y, rx, ry);
 }
 return d;
}

uint64_t
prepare_input (char* in) 
{
 int64_t a, b, ret;
 sscanf (in, "%" SCNi64 " %" SCNi64 , &a , &b );
 a /= 2000;
 b /= 2000;
 ret = hilbert (1024, a, b);
 return ret;
}

char*
get_ip (const char* interface) 
{
 static char if_ip [INET_ADDRSTRLEN];
 struct ifaddrs *ifAddrStruct = NULL, *ifa = NULL;

 getifaddrs (&ifAddrStruct);

 for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
  if (ifa->ifa_addr->sa_family == AF_INET && strcmp (ifa->ifa_name, interface) == 0)
   inet_ntop (AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, 
     if_ip, INET_ADDRSTRLEN);

 if (ifAddrStruct != NULL) freeifaddrs (ifAddrStruct);

 return if_ip;
}

char*
byte_units (const double bytes)
{
 static char result[0x100];

 if (bytes >= 1 << 30)
  sprintf(result, "%0.2lf GiB", bytes / (1 << 30));
 else if (bytes >= 1 << 20)
  sprintf(result, "%0.2lf MiB", bytes / (1 << 20));
 else if (bytes >= 1 << 10)
  sprintf(result, "%0.2lf KiB", bytes / (1 << 10));
 else
  sprintf(result, "%.0lf B", bytes);

 return result;
}
