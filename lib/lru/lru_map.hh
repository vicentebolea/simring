/**
 * @class  lru_map 
 * @brief  Template header for lru_map ADT
 * @see    hashTable <hashtable.hh>
 * @author Vicente Adolfo Bolea Sanchez
 *         <vicente.bolea@gmail.com>,
 *         <vicente@unist.ac.kr>
 **********************************************************
 *
 * @section DESCRIPTION
 * This is just a sketch for the LRU map class.
 *
 * @subsection TIME COMPLEXITY
 * Here is described the time complexities of each
 * functions: 
 *  - insert:   O(1) (amortized time)
 *  - lookup:   O(1)
 *  - oldest:   O(1) 
 *  - newest:   O(1) 
 *  - pop:      O(1) 
 *
 * These complexities will be in the best case since 
 * the hash table will may rehash sometimes and in each 
 * rehash it will take O(n + buckets).
 *
 * @subsection METHODOLOGY
 * This Lru map is using a hashtable+doublylinkedlist where:
 *  - The hash table will store a pointer of each element in
 *    the list.
 *  - The linkedlist will be use as lru linkedlist with a
 *    pair of key and value.
 * 
 * @subsection TODO
 * Here is the way that I am implementing this LRU:
 *  - The LL will just store the diskpage.
 *
 *  - The hash table given a key (wish in this experiment
 *    will be a set of number) will store pointers to each 
 *    node of the LL. 
 *
 */

#ifndef __LRU_MAP_HH_
#define __LRU_MAP_HH_

#include "dictionary.hh"
#include "hashtable.hh"

#include <stdexcept>
#include <algorithm>
#include <list>

using std::pair;
using std::list;
using std::out_of_range;

template <class key, class value>
class lru_map: public dictionary<key, value> {

 public:
  lru_map (size_t _max) : max (_max) {}
	~lru_map () {}

  bool insert (const key&, const value&);
  void pop (void) throw (out_of_range);
  const value& lookup (const key&) throw (out_of_range);
  const value& oldest (void) throw (out_of_range);
  const value& newest (void) throw (out_of_range);

 protected:
  void update (const key&, const value&) throw (out_of_range);

 protected:
  size_t max;

  list<pair<key, value> > ll;
  hashTable<key, typename list<pair<key, value> >::iterator> ht;
};

#include "lru_map.tcc"


#endif
