#include <simring.hh>

/*
 *
 */
SETcache::SETcache (int _size, char* p) {
	this->cache = new set<diskPage, bool (*) (const diskPage&, const diskPage&)> (diskPage::less_than);
	this->_max = _size;
	if (p != NULL) setDataFile (p);
  pthread_mutex_init (&mutex_match, NULL );
  pthread_mutex_init (&mutex_queue_low, NULL );
  pthread_mutex_init (&mutex_queue_upp, NULL );
}

void SETcache::setDataFile (char* p) { 
	strncpy (this->path, p, 256);
}

ostream& operator<< (ostream& out, SETcache& in) {
	set<diskPage>::iterator it; 
  out << "Size: [" << in.cache->size() << "]" << endl;
  out << "Current elements in the cache" << endl;
  out << "---------------------------" << endl;
  out << "SET: ";
	for (it = in.cache->begin (); it != in.cache->end(); it++)
		out << "Item: [" << (*it).index << "] , ";
  out << endl;
  if (!in.queue_lower.empty()) {
    out << "QUEUE_LOW: [FRONT=" << in.queue_lower.front().index;
    out << "] [BACK=" << in.queue_lower.back().index << "]" << endl; 
  } 

  if (!in.queue_upper.empty()) {
    out << "QUEUE_LOW: [FRONT=" << in.queue_upper.front().index;
    out << "] [BACK=" << in.queue_upper.back().index << "]" << endl; 
  }
  out << "---------------------------" << endl;
	return out;
}

/*
 *
 */
bool SETcache::match (uint64_t idx, double ema, double low, double upp) {
	diskPage a (idx);
  //cout << "EMA: " << ema << "LOW: " << low << "UP: " << upp << endl;

  this->boundary_low = low;
  this->boundary_upp = upp;
  this->ema = ema;

	if (cache->end () != cache->find (a)) {  //! If it is found O(log n)
		return true;

	} else {

		long currentChunk = a.index * DPSIZE; //! read a block from a file
		ifstream file (path, ios::in | ios::binary);

		if (!file.good ()) { perror ("FILE NOT FOUND"); exit (EXIT_FAILURE); } 

		file.seekg (currentChunk, ios_base::beg);
		file.read (a.chunk, DPSIZE);
		file.close (); 

		//! Inserting [ O(logn) ]
    pthread_mutex_lock (&mutex_match);
		cache->insert (a);
    pthread_mutex_unlock (&mutex_match);

		pop_farthest ();
		return false;
  }
}

diskPage SETcache::get_low () {

   diskPage out = queue_lower.front ();
   pthread_mutex_lock (&mutex_queue_low);
   queue_lower.pop ();
   pthread_mutex_unlock (&mutex_queue_low);
   return out;
}

diskPage SETcache::get_upp () {

   diskPage out = queue_upper.front ();
   pthread_mutex_lock (&mutex_queue_upp);
   queue_upper.pop ();
   pthread_mutex_unlock (&mutex_queue_upp);
   return out;
}

bool SETcache::is_valid (diskPage& dp) {
  uint64_t item (dp.index);

  //! 1st test: Its inside the boundaries 
  //if ((boundary_low < item) && (item < boundary_upp)) {

    //! 2st test: Is not the farthest one 
		set<diskPage>::iterator first = cache->begin (); //! 0(1)
		set<diskPage>::reverse_iterator last = cache->rbegin (); //! O(1)

		uint64_t lowest  = (*first).index;
		uint64_t highest = (*last).index;

    //if (lowest < item && item < highest) { 
      diskPage in = dp;
      pthread_mutex_lock (&mutex_match);
      cache->insert(in);
      pthread_mutex_unlock (&mutex_match);
      pop_farthest ();
      return true;
    //}
  //}
  //return false;
}
void SETcache::pop_farthest () {
	//! In case we excede Delete the last page
	//! New policy, delete the farnest element :TRICKY:
	//! Complexity O(1)
	if ((int)cache->size () > this->_max) {
		set<diskPage>::iterator first = cache->begin (); //! 0(1)
		set<diskPage>::reverse_iterator last = cache->rbegin (); //! O(1)

		uint64_t lowest  = (*first).index;
		uint64_t highest = (*last).index;

		if (((uint64_t)ema - lowest) > ((uint64_t)highest - ema)) {

			pthread_mutex_lock (&mutex_queue_low);
			queue_lower.push (*first);
			pthread_mutex_unlock (&mutex_queue_low);

			pthread_mutex_lock (&mutex_match);
			cache->erase (lowest);
			pthread_mutex_unlock (&mutex_match);

		} else { 

			pthread_mutex_lock (&mutex_queue_upp);
			queue_upper.push (*last);
			pthread_mutex_unlock (&mutex_queue_upp);

			pthread_mutex_lock (&mutex_match);
			cache->erase (highest);
			pthread_mutex_unlock (&mutex_match);

		}
	}
}
