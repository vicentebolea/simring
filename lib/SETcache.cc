#include <simring.hh>

/*
 *
 */
SETcache::SETcache (int _size, char* p) {
	this->cache = new set<diskPage, bool (*) (const diskPage&, const diskPage&)> (diskPage::less_than);
	this->_max = _size;
	if (p != NULL) setDataFile (p);
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
  update (low, upp); //! O(logn)
  //cout << "EMA: " << ema << "LOW: " << low << "UP: " << upp << endl;

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
		cache->insert (a);

		//! In case we excede Delete the last page
		//! New policy, delete the farnest element :TRICKY:
		//! Complexity O(1)
		if ((int)cache->size () > this->_max) {
			set<diskPage>::iterator first = cache->begin (); //! 0(1)
			set<diskPage>::reverse_iterator last = cache->rbegin (); //! O(1)

			uint64_t lowest  = (*first).index;
			uint64_t highest = (*last).index;

			if (((uint64_t)ema - lowest) < ((uint64_t)highest - ema))
				cache->erase (lowest);

			else 
				cache->erase (highest);
		} 

		return false;
	}
}

/*
 *
 */
void SETcache::update (double low, double upp) {
	set<diskPage>::iterator low_i;
	set<diskPage>::iterator upp_i;
	set<diskPage>::iterator it;

	//! Set the iterators in the boundaries [ O(logn) ]
	low_i = cache->lower_bound (diskPage ((uint64_t)(low + .5)));

	if (! (low_i == cache->end ()) && !(low_i == cache->begin ())) {

		//! Fill lower queue [ O(m1) ]
		for (it = cache->begin (); it != low_i; it++) {
			queue_lower.push (*it);
		}

		cache->erase (cache->begin (), low_i);
	}  

	upp_i = cache->upper_bound (diskPage ((uint64_t)(upp + .5)));

	if (!(upp_i == cache->end ()) && !(upp_i == cache->begin ()))  {

		//! Fill upper queue [ O(m2) ]
		for (it = upp_i; it != cache->end (); it++)
			queue_upper.push (*it);

		//! Delete those elements [ O(m1 + m2) ]
		cache->erase (upp_i, cache->end());
	}
}
