#include <uniDQP.h>

void SETcache::match(packet* e, uint64_t** count)
{
	assert (e != NULL);

	int _max_ = (int) ceil((float)e->length/DPSIZE);
	int t 		= (int) floor((float)e->offset/DPSIZE);

	for (; t < _max_; t++){
		diskPage a (e->fid, t);
		if (cache.find(a) != cache.end()){
			*count[0]= *count[0] + 1; // hit
		}
    else{ 
			*count[1]= *count[1] + 1; // miss

      // read a block from a file
			long currentChunk = (e->fid%4429) * 12000000;  // Over 12MiB
			ifstream file ("/scratch/youngmoon01/garbage2.bin", ios::in | ios::binary);

			if (file.good() == false) {
				perror("FILE NOT FOUND"); exit(1); }

			file.seekg(currentChunk + (t * DPSIZE), ios_base::beg );
			file.read(a.chunk, DPSIZE);
			file.close();
			cache.insert(a);
		} 

		if ((int)cache.size() > max)
			cache.erase(cache.begin());
	}
}
