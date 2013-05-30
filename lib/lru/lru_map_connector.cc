#include <lru_map.h>

extern "C" int LRU_create (void* l, size_t size) {
  l = static_cast <void*> (new lru_map<int, void []> (size));
	return 0;
}

extern "C" int LRU_destroy (void* l) {
  delete static_cast <lru_map*> (l);
	return 0;
}

extern "C" int LRU_push (void* l, int key, void* data, size_t _size)) {
  (static_cast <lru_map*> (l)) ->push (key, data, _size);
	return 0;
}

extern "C" int LRU_pop (void* l) {
  (static_cast <lru_map*> (l)) -> pop ();
	return 0;
}
