#include <uniDQP.h>

LRUcache::~LRUcache()
{
	if( length > 0 ){
		diskPage* aux;
		for (diskPage* tmp = oldest; tmp->next != NULL; tmp = aux){
			aux = tmp->next;
			delete tmp;
		}
	}
}

bool LRUcache::isEmpty() { return length == 0 ? true: false; }

void LRUcache::push(diskPage* e) 
{
	diskPage* tmp = new diskPage(*e);
	assert(tmp != NULL);

	if (length == 0)
		oldest = newest = tmp;
	else{
		newest->next = tmp;
		newest = tmp;
		newest->next = NULL;
	}

	length++;
	if (length > max)
		pop();
}

void LRUcache::pop()
{
	assert(length > 0);

	if (length == 1){
		delete oldest;
		oldest = newest = NULL;
	}
	else{
		diskPage* aux = oldest;
		oldest = oldest->next;
		delete aux;
	}
	length--;
}

bool LRUcache::get(diskPage* e) 
{
	if (length == 0){
		push(e);
		return false;
	}

	else if (*oldest == *e){
		pop();
		push(e);
		return true;
	}

	else if (length == 1){
		push(e);
		return false;	
	}

	else if (*newest == *e)
		return true;

	else{
		for (diskPage* tmp = oldest; tmp->next != NULL; tmp = tmp->next)
			if (*tmp->next == *e){
				diskPage* aux = tmp->next;
				tmp->next = tmp->next->next;
				delete aux;
				push(e);
				return true;
			}
	}
	push(e);
	return false;
}

void LRUcache::match(packet *e, uint64_t** count )
{
	assert (e != NULL);

	int _max_ = (int) ceil((float)e->length/DPSIZE);
	int t 		= (int) floor((float)e->offset/DPSIZE);

	for (; t < _max_; t++){
		diskPage a (e->fid, t);
		if (get(&a) == true){
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
			push(&a);
		} 

		if (length > max)
			pop();
	}
}
