/** ***************************************************//**
 * @author Vicente Adolfo Bolea Sanchez
 *         <vicente.bolea@gmail.com>,
 *         <vicente@unist.ac.kr>
 *
 * @class  dictionary
 * @interface dictionary
 * @brief  abstract class for dictionary ADT (INTERFACE)
 ** ***************************************************//**
 *
 */

#ifndef __DICTIONARY_HH_
#define __DICTIONARY_HH_

#include <algorithm>
#include <stdexcept>
#include <stddef.h>

using std::out_of_range;

template <class key, class value>
class dictionary {
 protected:
  size_t size;

 public:
  dictionary () : size (0) {}
  virtual ~dictionary () {}

  virtual bool insert (const key& k, const value& v) = 0;
  //virtual void remove (const key& k) = 0; :TODO:
  virtual const value& lookup (const key& k)
   throw (out_of_range) = 0;

  inline size_t getSize () const { return size; }
  inline bool empty () const { return size == 0 ? true: false; }
};

#endif
