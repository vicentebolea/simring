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

long *toArray(char*);
void toArray(char*,long*);
double toDouble(long*);
double toDouble(long, long, long);
uint64_t timediff(struct timeval*, struct timeval*);
void send_msg(int, char*, int);
void recv_msg(int, char*);
int poisson(double);

enum type {query = 001, info = 002, quit = 004};
/** @brief Class which represent an abstract packet which
 * will be send by sockets
 */
class packet {
	public:
		long fid, offset, length, last, time;

		packet();
		/** This constructed will be intesively used 
		 * @param char* line
		 * @return void
		 */
		packet(char* line);

		packet(const packet&);

		/** overload operator= for copy constructor
		 */ 
		packet& operator=(const packet&);

		/** Calculate the distant between this two packets
		 * @param packet 
		 * @return int distance
		 */
		int getDistance(const packet&);

		/** \overload
		 * @param double point
		 * @return int distance
		 */
		double getDistance(double);

		/** Calculate the exponential moving average
		 * Given by this formula:  \f$ (1 - \alpha )x_{n-1} + \alpha x_n \f$
		 * @return double exponential moving average
		 */
		double getEMA();
};

class Query: public packet {
	private:
		struct timeval scheduledDate;
		struct timeval startDate;
		struct timeval finishedDate;

	public:
		//constructor & destructor
		Query(): packet() {}
		Query(const packet&);
		Query(const Query&);

		//setter
		void setStartDate();
		void setFinishedDate();

		//getter
		uint64_t getWaitTime();
		uint64_t getExecTime();
};

class server {
	private:
		double ema, alpha;

	public:
  server () : ema(.0), alpha(.0) {}
		server(packet&, double);
		double getDistance(packet&);
		void updateEMA(packet&);
};

/* Cache Data structure classes */

class diskPage {
	public:
		long fid, offset;
		char chunk[DPSIZE];
		diskPage* next;
		diskPage(const long& f, const long& ofst): fid(f), offset(ofst) {}

		diskPage(const diskPage& that){
			fid = that.fid;
			offset = that.offset;
			memcpy (chunk, that.chunk, DPSIZE);
		}

		bool operator== (diskPage& that) {
			return fid == that.fid && offset == that.offset ? true: false;
		}
};

class LRUcache {
	private:
  lru_map<uint64_t, diskpage>& lru;

		long max;
  char path [256];
	public:
		LRUcache(int _size) : lru (_size);

  void setDataFile (char* p) { 
   strncpy (this->path, p, 256);
  }
		void match (packet&, uint64_t*, uint64_t*);
};

#endif
