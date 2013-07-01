#include <simring.hh>

/*
 *
 */
LRUcache::LRUcache (int _size, char* p) {
	this->cache_item = new set<diskPage, bool (*) (const diskPage&, const diskPage&)> (diskPage::less_than);
	this->cache_time = new set<diskPage, bool (*) (const diskPage&, const diskPage&)> (diskPage::less_than_lru);
	this->_max = _size;
	if (p != NULL) setDataFile (p);
}

void LRUcache::setDataFile (char* p) { 
	strncpy (this->path, p, 256);
}

ostream& operator<< (ostream& out, LRUcache& in) {
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
bool LRUcache::match (uint64_t idx, uint64_t time, double ema, double low, double upp) {
	diskPage a (idx, time);

	if (cache->end () != cache->find (a)) {              //! If it is found O(log n)
		return true;

	} else {

		long currentChunk = a.index * DPSIZE;              //! read a block from a file
		ifstream file (path, ios::in | ios::binary);

		if (!file.good ()) { perror ("FILE NOT FOUND"); exit (EXIT_FAILURE); } 

		file.seekg (currentChunk, ios_base::beg);
		file.read (a.chunk, DPSIZE);
		file.close (); 

		cache->insert (a);                                 //! Inserting [ O(logn) ]

		//! In case we excede Delete the last page
		if ((int)cache->size () > this->_max) {            //! Complexity O(1)
			set<diskPage>::iterator first = cache->begin (); //! 0(1)
			uint64_t oldest = (*first).index;

			if (oldest < ((uint64_t)ema)) queue_lower.push (*first);
      else                          queue_upper.push (*first);
		} 

		return false;
	}
}
