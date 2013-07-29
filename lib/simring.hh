/**
 * @file uniDQP.h
 * @mainpage 
 * @brief "Library for uniDQP project"
 * @author Vicente Adolfo Bolea Sanchez
 */

#ifndef _SIMRING_HH_
#define _SIMRING_HH_

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#define OK 0
#define FAIL -1

#ifndef DPSIZE
#define DPSIZE (2 << 13)
#endif

#ifndef DATAFILE
#define DATAFILE "/scratch/youngmoon01/garbage2.bin"
#endif

#ifndef PORT 
#define PORT 19999
#endif

#ifndef NSERVERS
#define NSERVERS 39
#endif

#ifndef HOST
#define HOST "10.20.12.170"
#endif

#ifndef ALPHA
#define ALPHA 0.03f
#endif

#ifndef LOT
#define LOT 1024
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdint.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <cfloat>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <set>
#include <queue>
#include <boost/lambda/lambda.hpp>
#include "lru/lru_map.hh"

using namespace std;
using namespace boost::lambda;

uint64_t timediff      (struct timeval*, struct timeval*);
void     send_msg      (int, const char*);
void     recv_msg      (int, char*) __attribute__((weak));
int      poisson       (double);
int64_t  hilbert       (int64_t n, int64_t x, int64_t y);
uint64_t prepare_input (char* in);

/** @brief Class which represent an abstract packet which
 * will be send by sockets
 */
class packet {
	public:
		uint64_t point, time; 
		double EMA, low_b, upp_b;

		packet ();
		packet (uint64_t);
		packet (uint64_t, double, double, double);
		packet (const packet&);
		packet& operator= (const packet&);

		uint64_t get_point ();
};

class Query: public packet {
	protected:
		struct timeval scheduledDate;
		struct timeval startDate;
		struct timeval finishedDate;

	public:
		//constructor & destructor
		Query (): packet() {}
		Query (const packet&);
		Query (const Query&);

		//setter
		void setScheduledDate ();
		void setStartDate ();
		void setFinishedDate ();

		//getter
		uint64_t getWaitTime ();
		uint64_t getExecTime ();
};

class Node { 
	protected:
		double EMA, low_b, upp_b, alpha;
		int fd;
		uint64_t time;

	public:
		Node () : EMA (.0), low_b (.0), upp_b (.0), alpha (.0) {} 
		Node (double a) : EMA (.0), low_b (.0), upp_b (.0), alpha (a) {} 
		Node (double a, double e) : EMA (e), low_b (.0), upp_b (.0) , alpha (a) {} 

		Node& set_fd (int f) { fd = f; return *this;}
		Node& set_EMA (double a) { EMA= a; return *this;}
		Node& set_alpha (double a) { alpha = a; return *this;}
		Node& set_low (double l) { low_b = l; return *this;}
		Node& set_upp (double u) { upp_b = u; return *this;} 
		Node& set_time (uint64_t u) { time = u; return *this;} 

		int get_fd () { return fd; }
		double get_low () { return low_b; }
		double get_upp () { return upp_b; } 
		double get_EMA () const { return EMA; }

		double get_distance (packet& p) { 
			return fabs (EMA - p.get_point ());
		}

		double get_distance (uint64_t p) {
			return fabs (EMA - p); 
		}

		Node& update_EMA (double point)  { EMA += alpha * (point - EMA); return *this; } 

		Node& accept (int sock) {
			socklen_t sin_size = sizeof(struct sockaddr_in);
			sockaddr_in addr;
			fd = ::accept (sock, (struct sockaddr *)&addr, &sin_size);
			printf ("[SCHEDULER] Backend server linked (addr = %s).\n", inet_ntoa(addr.sin_addr)); 
			return *this;
		}

		Node& send (uint64_t point) {
			packet toSend (point, EMA, low_b, upp_b);
			toSend.time = time;
			::send_msg (fd, "QUERY");
			::send (fd, &toSend, sizeof (packet), 0);
			return *this;
		}

		Node& send_msg (const char * in) { ::send_msg (fd, in); return *this; }
		Node& close () { ::close (fd); return *this; }
};

//-----------------------------------------------------------------//
//---------------CACHE CLASSES INTERFACE---------------------------//
//-----------------------------------------------------------------//

class diskPage {
	public:
		uint64_t index, time;
		char chunk [DPSIZE];

		diskPage () : index (0) {}
		diskPage (const uint64_t i) : index (i) {}

		diskPage (const diskPage& that) {
			index = that.index;
			time = that.time;
			memcpy (chunk, that.chunk, DPSIZE);
		}

		diskPage& operator= (const diskPage& that) {
			index = that.index;
			time = that.time;
			memcpy (chunk, that.chunk, DPSIZE);
			return *this;
		}

		bool operator== (const diskPage& that) {
			return index == that.index ? true: false;
		}

		static bool less_than (const diskPage& a, const diskPage& b) {
			return (a.index < b.index);
		}

		static bool less_than_lru (const diskPage& a, const diskPage& b) {
			return (a.time < b.time);
		}

};

enum policy {
	NOTHING  = 0x0,
	UPDATE   = 0x1,
	LRU      = 0x2,
	BOUNDARY = 0x4,
	JOIN     = 0x8
};

class SETcache {
	protected:
		set<diskPage, bool (*) (const diskPage&, const diskPage&)>* cache;
		set<diskPage, bool (*) (const diskPage&, const diskPage&)>*
			cache_time;
		char path [256];
		int _max, policy;
		uint64_t count;
		double boundary_low, boundary_upp, ema;

		pthread_mutex_t mutex_match     ;
		pthread_mutex_t mutex_queue_low ;
		pthread_mutex_t mutex_queue_upp ;

		void pop_farthest ();

	public:
		SETcache (int, char * p = NULL);
		~SETcache () { delete cache; }

		void set_policy (int);
		void setDataFile (char*);
		bool match (Query&);
		bool is_valid (diskPage&);
		void update (double low, double upp);
  diskPage get_diskPage (uint64_t);

		queue<diskPage> queue_lower;
		queue<diskPage> queue_upper;

		diskPage get_low ();
		diskPage get_upp ();

		friend ostream& operator<< (ostream&, SETcache&);
};

#endif
