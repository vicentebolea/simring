#ifndef __HASHTABLE_TCC_
#define __HASHTABLE_TCC_

/////////////////////////PUBLIC////////////////////////////


/** ***************************************************//**
 * @pre _size > 0
 */
template <class key, class value>
hashTable<key, value>::hashTable (size_t _size = 512) {
 assert (_size != 0);
 buckets_no = _size;
 buckets = new list<entry> [buckets_no];
}


/** ***************************************************//**
 *
 */
template <class key, class value>
hashTable<key, value>::~hashTable () {
 delete[] buckets;
}

// ================== OPERATIONS ==========================


/** ***************************************************//**
 * @pre key != NULL and value != NULL
 * @post buckets[h(key)] == value
 */
template <class key, class value>
bool
hashTable<key, value>::insert (const key& k, const value& v) {
 uint32_t key_n = h(k);

 if (find (k))
  return false;

 else {
  buckets[key_n].push_back (entry(k, v));
  this->size++;

  if (over_threshold()) rehash();

  return true;
 }
}


/** ***************************************************//**
 *
 */
template <class key, class value>
void
hashTable<key, value>::remove (const key& k) {
	if (find (k)) this->size--;
 buckets [h(k)].remove_if (match_key(k));
}


/** ***************************************************//**
 * 
 * @throw  STL Exception "out_of_range"
 * @param  The key of the object that you are looking for
 * @return An instance of the object paired with the given
 *         key
 */
template <class key, class value>
const value&
hashTable<key, value>::lookup (const key& k)
throw (out_of_range)
{
 list<entry>& l = buckets[h(k)];
 typename list<entry>::iterator it;

 it = find_if (l.begin(), l.end(), match_key(k));
 if (it == l.end())
  throw out_of_range ("Doesnt belong");
 else
  return (*it).second;
}


/** ***************************************************//**
 * @param  The key of the object that you are looking for
 * @return A boolean variable if it is found
 */
template <class key, class value>
bool 
hashTable<key, value>::find (const key& k) {
 list<entry>& t = buckets [h(k)];
 return find_if(t.begin(), t.end(), match_key(k)) != t.end();
}

/////////////////////////PROTECTED/////////////////////////


/** ***************************************************//**
 * @return  An boolean indicating if the load_factor is 
 *          over the upper threshold e.g 1.5
 */
template <class key, class value>
inline bool hashTable<key, value>::over_threshold () const {
 return (double)(this->size/buckets_no) >= threshold;
 return true;
}


/** ***************************************************//**
 * This method will resize the hash table to the double 
 * of the current size.
 *
 * First it will create a new table and it will be filled 
 * with the old elements and keys. After that the old table 
 * will be removed.
 *
 * @param  void
 * @return void
 */
template <class key, class value>
void hashTable<key, value>::rehash () {

 list<entry>* old_buckets    = buckets;
 size_t       old_buckets_no = buckets_no;

 buckets_no *= 2;
 buckets = new list<entry> [buckets_no];

 //For each list of entries
 for (size_t i = 0; i < old_buckets_no; i++) {

  if (!old_buckets[i].empty()) {
   typename list<entry>::iterator it;
   list<entry>& b = old_buckets[i];

   //Copy all the entries of the list
   for (it = b.begin(); it != b.end(); it++) {
    size_t new_key = h ((*it).first);
    buckets [new_key].push_back (*it);
   }
  }
 }
 delete [] old_buckets;
}


/** ***************************************************//**
 * @brief    Simple hash function which can represent
 *           less than 32 bits number, It uses modulo.
 *
 * @param    A key object
 * @param    An integer with the domain of the function
 * @return   An unique integer of 32b for indexing the table
 * @pre      length should be greater than 0
 */
template <class key, class value>
inline uint32_t
hashTable<key, value>::h (const key& k, size_t length = 0) const
{
 uint8_t* seed = (uint8_t*) &k;
 uint32_t _key = 0;

 if (!length) length = buckets_no;

 for (size_t i = 0; i < sizeof(key) % 5; i++)
  _key += (uint32_t) (seed[i] << (0x8 * i));

 return _key % length;
}

#endif

