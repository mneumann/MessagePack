#ifndef __MESSAGEPACK_LOAD__HEADER__
#define __MESSAGEPACK_LOAD__HEADER__

#include <stdint.h>   /* uint32_t ... */
#include <assert.h>   /* assert() */
#include <stdlib.h>   /* malloc, free */
#include <vector>
#include <set>
#include <map>
#include <string>
#include "MessagePack.h"

namespace MessagePack 
{
  using namespace std;

  // TODO: Check range!

  inline void load(uint64_t &value, Unpacker &uk)
  {
    value = uk.get_uint();
  }

  inline void load(uint32_t &value, Unpacker &uk)
  {
    value = uk.get_uint();
  }

  inline void load(uint16_t &value, Unpacker &uk)
  {
    value = uk.get_uint();
  }

  inline void load(uint8_t &value, Unpacker &uk)
  {
    value = uk.get_uint();
  }

  inline void load(int64_t &value, Unpacker &uk)
  {
    value = uk.get_int();
  }

  inline void load(int32_t &value, Unpacker &uk)
  {
    value = uk.get_int();
  }

  inline void load(int16_t &value, Unpacker &uk)
  {
    value = uk.get_int();
  }

  inline void load(int8_t &value, Unpacker &uk)
  {
    value = uk.get_int();
  }

  inline void load(double &value, Unpacker &uk)
  {
    value = uk.get_double();
  }

  inline void load(string &value, Unpacker &uk)
  {
    uint32_t sz = uk.get_raw();
    void *buf = malloc(sz);
    assert(buf);
    uk.get_raw_body(buf, sz);
    value.assign((char*)buf, sz);
    free(buf);
  }

  template <class T>
  void load(vector<T> &value, Unpacker &uk)
  {
    for (int sz = uk.get_array(); sz > 0; --sz)
    {
      T element;
      load(element, uk);
      value.push_back(element);
    }
  }

  template <class T>
  void load(set<T> &value, Unpacker &uk)
  {
    for (int sz = uk.get_array(); sz > 0; --sz)
    {
      T element;
      load(element, uk);
      value.insert(element);
    }
  }

  template <class K, class V>
  void load(map<K, V> &value, Unpacker &uk)
  {
    for (int sz = uk.get_map(); sz > 0; --sz)
    {
      K key;
      load(key, uk);
      load(value[key], uk);
    }
  }
};

#endif
