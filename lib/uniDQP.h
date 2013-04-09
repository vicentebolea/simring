/**
 * @file uniDQP.h
 * @mainpage 
 * @brief "Library for uniDQP project"
 * @author Vicente Adolfo Bolea Sanchez
 */

#ifndef _UNIDQP_
#define _UNIDQP_

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#define OK 0
#define FAIL -1

#ifndef DPSIZE
#define DPSIZE 4096
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
#include <cfloat>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <set>
#include "lru/lru_map.hh"

using namespace std;

long *toArray (char*);
void toArray (char*,long*);
double toDouble (long*);
double toDouble (long, long, long);
uint64_t timediff (struct timeval*, struct timeval*);
void send_msg (int, char*, int);
void recv_msg (int, char*);
int poisson (double);

enum type {query = 001, info = 002, quit = 004};

/** @brief Class which represent an abstract packet which
 * will be send by sockets
 */
class packet {
	public:
		long fid, offset, length, last, time;

		packet ();
		packet (char* line);
		packet (const packet&);
		packet& operator= (const packet&);
		int getDistance (const packet&);
		double getDistance (double);
		double getEMA ();
};

class Query: public packet {
	private:
		struct timeval scheduledDate;
		struct timeval startDate;
		struct timeval finishedDate;
  uint64_t key;

	public:
		//constructor & destructor
		Query (): packet() {}
		Query (const packet&);
		Query (const Query&);

		//setter
		void setStartDate();
		void setFinishedDate();

		//getter
		uint64_t getWaitTime();
		uint64_t getExecTime();
  uint64_t getKey();
};

class server {
	private:
		double ema, alpha;

	public:
  server () : ema(.0), alpha(.0) {}
		server (packet&, double);
		double getDistance (packet&);
		void updateEMA (packet&);
};

/* Cache Data structure classes */

class diskPage {
	public:
		uint64_t fid, offset, key;
		char chunk [DPSIZE];

		diskPage (const long& f, const long& ofst): fid(f), offset(ofst) {
   key = f + ofst;
  }

		diskPage (const diskPage& that) {
			fid = that.fid;
			offset = that.offset;
   key = that.key;
			memcpy (chunk, that.chunk, DPSIZE);
		}

		bool operator== (const diskPage& that) {
			return fid == that.fid && offset == that.offset ? true: false;
		}
};

class LRUcache {
	protected:
  lru_map<uint64_t, diskPage>* cache;
  char path [256];

	public:
		LRUcache (int);

  void setDataFile (char*);
		void match (packet&, uint64_t*, uint64_t*);
};

#endif
