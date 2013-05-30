#include <simring.hh>

LRUcache::LRUcache (int _size) {
 cache = new lru_map<uint64_t, diskPage> (_size);
 setDataFile (DATAFILE);
}

void LRUcache::setDataFile (char* p) { 
 strncpy (this->path, p, 256);
}

void LRUcache::match (packet& p, uint64_t* hit, uint64_t* miss) {
 int _max_ = (int) ceil((float)p.length/DPSIZE);
 int t     = (int) floor((float)p.offset/DPSIZE);

 for (; t < _max_; t++) {
  diskPage a (p.fid, t);

  try {

   if (a == cache->lookup (a.key))
    hit++; // hit

  } catch (out_of_range& e) {

   miss++; // miss

   // read a block from a file
   long currentChunk = (p.fid%4429) * 12000000;  // Over 12MiB
   ifstream file ("/scratch/youngmoon01/garbage2.bin", ios::in | ios::binary);

   if (file.good() == false) {
    perror ("FILE NOT FOUND"); exit (1); }

   file.seekg (currentChunk + (t * DPSIZE), ios_base::beg );
   file.read (a.chunk, DPSIZE);
   file.close (); 
   cache->insert (a.key, a);
  } 
 }
}
