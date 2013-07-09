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

#ifndef CACHESIZE
#define CACHESIZE 100000
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
void     send_msg      (int, char*);
void     recv_msg      (int, char*) __attribute__((weak));
int      poisson       (double);
int64_t  hilbert       (int64_t n, int64_t x, int64_t y);
uint64_t prepare_input (char* in);

/** @brief Class which represent an abstract packet which
 * will be send by sockets
 */
class packet {
	public:
    uint64_t point; 
    double low_b, upp_b, EMA;

		packet () : point (0) {}
		packet (uint64_t);
		packet (uint64_t, uint64_t, uint64_t);
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

 public:
  Node (double a) : EMA (.0), low_b (.0), upp_b (.0), alpha (a) {} 
  Node (double a, double e) : EMA (e), low_b (.0), upp_b (.0) , alpha (a) {} 
  void set_low (double l) { low_b = l; }
  void set_upp (double u) { upp_b = u; } 
  double get_low () { return low_b; }
  double get_upp () { return upp_b; } 

	double get_distance (packet& p) { 
   return fabs (EMA - p.get_point ());
  }
 
  double get_distance (uint64_t p) {
   return fabs (EMA - p); 
  }

  void update_EMA (double point)  { EMA += alpha * (point - EMA); } 
  double get_EMA () const { return EMA; }
};

//-----------------------------------------------------------------//
//---------------CACHE CLASSES INTERFACE---------------------------//
//-----------------------------------------------------------------//

class diskPage {
	public:
		uint64_t index;
		char chunk [DPSIZE];

		diskPage () : index (0) {}
		diskPage (const uint64_t i) : index (i) {}

		diskPage (const diskPage& that) {
			index = that.index;
			memcpy (chunk, that.chunk, DPSIZE);
		}

		diskPage& operator= (const diskPage& that) {
			index = that.index;
			memcpy (chunk, that.chunk, DPSIZE);
      return *this;
		}

		bool operator== (const diskPage& that) {
			return index == that.index ? true: false;
		}

    static bool less_than (const diskPage& a, const diskPage& b) {
     return (a.index < b.index);
    }
};

class SETcache {
	protected:
		set<diskPage, bool (*) (const diskPage&, const diskPage&)>* cache;
		char path [256];
    int _max;
    uint64_t count;
    double boundary_low, boundary_upp, ema;
		pthread_mutex_t mutex_match     ;
		pthread_mutex_t mutex_queue_low ;
		pthread_mutex_t mutex_queue_upp ;

    void pop_farthest ();

	public:
    SETcache (int, char * p = NULL);
    ~SETcache () { delete cache; }

		void setDataFile (char*);
		bool match (uint64_t, double, double, double);
    bool is_valid (diskPage&);
    void update (double low, double upp);

		queue<diskPage> queue_lower;
		queue<diskPage> queue_upper;
 
    diskPage get_low ();
    diskPage get_upp ();

    friend ostream& operator<< (ostream&, SETcache&);
};

class LRUcache {
	protected:
		lru_map<uint64_t, diskPage>* cache;
		set<uint64_t> bst;
		char path [256];

		void insert (uint64_t);

	public:
		LRUcache (int);

		void setDataFile (char*);
		void match (uint64_t , uint64_t*, uint64_t*);
		void update (double, double);

		queue<diskPage> queue_lower;
		queue<diskPage> queue_upper;
};

#endif
