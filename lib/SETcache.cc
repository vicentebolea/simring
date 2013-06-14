#include <simring.hh>

SETcache::SETcache (int _size, char* p) {
	this->cache = new set<diskPage, bool (*) (const diskPage&, const diskPage&)> (diskPage::less_than);
	this->_max = _size;
	if (p != NULL) setDataFile (p);
}

void SETcache::setDataFile (char* p) { 
	strncpy (this->path, p, 256);
}

bool SETcache::match (uint64_t idx, double low, double upp) {
	diskPage a (idx);

	if (cache->end () != cache->find (a)) {  //!If it is found 
		return true;

	} else {

		long currentChunk = a.index * DPSIZE; //! read a block from a file
		ifstream file (path, ios::in | ios::binary);

		if (!file.good ()) { perror ("FILE NOT FOUND"); exit (EXIT_FAILURE); } 

		file.seekg (currentChunk, ios_base::beg);
		file.read (a.chunk, DPSIZE);
		file.close (); 

		//Inserting 
		cache->insert (a);

		//! In case we excede Delete the last page
		//! New policy, delete the farnest element
		if ((int)cache->size () >= this->_max) {
			set<diskPage>::iterator first = cache->begin ();
			set<diskPage>::reverse_iterator last = cache->rbegin ();

      uint64_t lowest  = (*first).index;
      uint64_t highest = (*last).index;

      if (((uint64_t)lowest - low) > ((uint64_t)upp - lowest))
			  cache->erase (lowest);

      else 
        cache->erase (highest);
		} 

		return false;
	}
}

void SETcache::update (double low, double upp) {
	set<diskPage>::iterator low_i;
	set<diskPage>::iterator upp_i;
	set<diskPage>::iterator it;

	//! Set the iterators in the boundaries
	low_i = cache->lower_bound (diskPage ((uint64_t)low));
	upp_i = cache->upper_bound (diskPage ((uint64_t)upp));

	//! Fill lower queue
	for (it = cache->begin(); it != low_i; it++)
		queue_lower.push (*it);

	//! Fill upper queue
	for (it = upp_i; it != cache->begin (); it++)
		queue_upper.push (*it);

	//! Delete those elements
	cache->erase (cache->begin(), low_i);
	cache->erase (upp_i, cache->end());
}
