#include <simring.hh>

LRUcache::LRUcache (int _size) {
	cache = new lru_map<uint64_t, diskPage> (_size);
	setDataFile (DATAFILE);
}

void LRUcache::setDataFile (char* p) { 
	strncpy (this->path, p, 256);
}

void LRUcache::match (uint64_t idx, uint64_t* hit, uint64_t* miss) {
	diskPage a (idx);

	try {
		if (a == cache->lookup (a.index))
			*hit = *hit + 1; // hit

	} catch (out_of_range& e) {

		*miss = *miss + 1; // miss
		long currentChunk = a.index * DPSIZE; //! read a block from a file
		ifstream file (path, ios::in | ios::binary);

		if (!file.good ()) { perror ("FILE NOT FOUND"); exit (EXIT_FAILURE); } 

		file.seekg (currentChunk, ios_base::beg );
		file.read (a.chunk, DPSIZE);
		file.close (); 
		cache->insert (a.index, a);
	} 
}

void LRUcache::insert (uint64_t i) {
	bst.insert (i);
}

void LRUcache::update (double low, double upp) {
	set<uint64_t>::iterator low_i;
	set<uint64_t>::iterator upp_i;
	set<uint64_t>::iterator it;

  //! Set the iterators in the boundaries
	low_i = bst.lower_bound ((uint64_t)low);
	upp_i = bst.upper_bound ((uint64_t)low);

  //! Fill lower queue
	for (it = bst.begin(); it != low_i; it++)
		queue_lower.push (cache->peak (*it));

  //! Fill upper queue
	for (it = upp_i; it != bst.begin (); it++)
		queue_upper.push (cache->peak (*it));

  //! Delete those elements
  bst.erase (bst.begin(), low_i);
  bst.erase (upp_i, bst.end());
}
