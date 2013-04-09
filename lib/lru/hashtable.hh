
/** ***************************************************//**
 * @class  hashTable
 * @brief  Template header for hashtable ADT
 * @author Vicente Adolfo Bolea Sanchez
 *         <vicente.bolea@gmail.com>
 *         <vicente@unist.ac.kr>
 *
 ***********************************************************
 * @section DESCRIPTION
 *
 * This is the implementation of a hash table
 * this hash table uses a linked list to prevent
 * colisions this is chained hash table.
 *
 * Also to keep a good load factor the hash table
 * will be resize whenever the load factor is over
 * 150%.
 * Nevertheless I am not rehashing to reduce the size
 *
 * For this matter I wonder to use another linkedlist to
 * keep track of all the keys stores, then I can iterate
 * over the table.
 * 
 * For now I am using a simple hash function which
 * just break the four first bytes of the key 
 * and apply mod with the bucket's table length.
 * @verbatim
 * Hash function 32 bits:
 * 
 * 00000000 00000000 00000000 XXXXXXXX
 * 00000000 00000000 XXXXXXXX 00000000
 * 00000000 XXXXXXXX 00000000 00000000
 * XXXXXXXX 00000000 00000000 00000000
 * 
 * KEY ----------------------> entry 
 *      buckets [hash (KEY)]
 *
 * Here is the model that I used to implement 
 * this hash table: 
 *
 * key | hash(KEY) |     buckets [hash (KEY)]
 * ----|-----------|------------------------------
 *
 *                 +------+--------+-------------+
 * KEY5 --+  +---->| KEY1 | VALUE1 |link to entry|
 *        |  |     +------+--------+-------------+
 * KEY1 --+--+ +-->| KEY2 | VALUE2 |link to entry|
 *        |    |   +------+--------+-------------+
 * KEY3 --+----+-->| KEY3 | VALUE3 |link to entry|
 *        |    |   +------+--------+-------------+
 * KEY2 --+----+   | KEY4 | VALUE4 |link to entry|
 *        |        +------+--------+-------------+
 *        +------->| KEY5 | VALUE5 |link to entry|
 *                 +------+--------+-------------+
 * @endverbatim
 */

#ifndef __HASHTABLE_HH_
#define __HASHTABLE_HH_

// Base class
#include <dictionary.hh>

// STL libraries
#include <list>
#include <algorithm>
#include <stdexcept>

// Types libraries
#include <inttypes.h>     
#include <stdint.h>
#include <stddef.h>
#include <assert.h> 

using std::pair;
using std::list;
using std::out_of_range;

template <class key, class value>
class hashTable: public dictionary<key, value> {
 typedef pair<key, value> entry;
 public:
  hashTable (size_t);
  ~hashTable ();

  bool insert (const key&, const value&);
  void remove (const key&);
  bool find (const key&);
  const value& lookup (const key&) throw (out_of_range);

 protected:
  // Functor to search a key in a given list
  struct match_key {
   key master_key;

   match_key (const key& k) : master_key(k) {}
   bool operator () (const entry& e) {
    return (master_key == e.first);
   }
  };

  // Attributes
  list<entry>* buckets;
  size_t buckets_no;

  const static double threshold = 1.5;
  inline bool over_threshold (void) const;

  inline uint32_t h (const key&, size_t) const;
  void rehash (void);
};

#include <hashtable.tcc>

#endif
