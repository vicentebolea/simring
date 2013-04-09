#ifndef __LRU_MAP_TCC_	
#define __LRU_MAP_TCC_	

////////////////////////  PUBLIC   /////////////////////////


/**
	* @brief  Push back in the linked list the given value and
	*         store the key and the address of that value in
	*         the hash table. In case that the pair is already
	*         in the list it will be moved to the newest
	*         position.
 * @param[in]  k  value
 * @param[in]  v  paired with the previous key
 */
template <class key, class value>
bool lru_map<key, value>::insert (const key& k, const value& v) {

 if (!ht.find(k)) {
  ll.push_back (pair<key, value>(k,v));

  typename list<pair<key, value> >::iterator it;
  it = --ll.end();
  ht.insert (k, it); // :TRICKY: end() returns the last?

  this->size++;

 } else {
  update (k, v);
 }

 if (this->size > max) pop();
	return true;
}

/** ***************************************************//**
 * @brief  Remove the older element of the list and its 
	*         reference in the hash table.
 */
template <class key, class value>
void lru_map<key, value>::pop () throw (out_of_range) {

	if (!this->size)
		return;
 key k ((*ll.begin()).first);
 ht.remove (k);

 ll.pop_front();
 this->size--;
}

/** ***************************************************//**
	* @brief     This method will search the value of the given
	*            key. If it is found, It will remove the
	*            element and push into the last position of the
	*            list, e.g. the newest element.
 * 
	* @param[in] k    key 
 * @return    tmp  value paired with the given key
 */
template <class key, class value>
const value&
lru_map<key, value>::lookup (const key& k) throw (out_of_range) {

 typename list<pair<key, value> >::iterator it = ht.lookup (k);
 value& tmp ((*it).second);
 update (k, tmp);

 return tmp;
}

/** ***************************************************//**
 * @return  The oldest element of the LRU_map
 */
template <class key, class value>
const value& lru_map<key, value>::oldest () throw (out_of_range) {
 return ll.front().second;
}

/** ***************************************************//**
 * @return  The oldest element of the LRU_map
 */
template <class key, class value>
const value& lru_map<key, value>::newest () throw (out_of_range) {
 return ll.back().second;
}

////////////////////////  PRIVATE   /////////////////////////

/** ***************************************************//**
 * @param[in]  k  value
 * @param[in]  v  paired with the previous key
 */
template <class key, class value>
void
lru_map<key, value>::update (const key& k, const value& v)
 throw (out_of_range) 
{
 // :TRICKY:
 typename list<pair<key, value> >::iterator it = ht.lookup (k);

 ht.remove (k);         // Delete to the hash table
 ll.erase (it);         // Delete that node in the list
 this->size--;
 insert (k, v);         // Rearrange the list
}

#endif
