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

// TODO: Check range of integer ops!

namespace MessagePack 
{
  using namespace std;

  inline Unpacker& operator>>(Unpacker &u, uint8_t &v) 
  {
    v = u.get_uint();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, uint16_t &v) 
  {
    v = u.get_uint();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, uint32_t &v) 
  {
    v = u.get_uint();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, uint64_t &v) 
  {
    v = u.get_uint();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, int8_t &v) 
  {
    v = u.get_int();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, int16_t &v) 
  {
    v = u.get_int();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, int32_t &v) 
  {
    v = u.get_int();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, int64_t &v) 
  {
    v = u.get_int();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, float &v) 
  {
    v = u.get_float();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, double &v) 
  {
    v = u.get_double();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, bool &v) 
  {
    v = u.get_bool();
    return u;
  }

  inline Unpacker& operator>>(Unpacker &u, string &v) 
  {
    uint32_t sz = u.get_raw();
    void *buf = malloc(sz);
    assert(buf);
    u.get_raw_body(buf, sz);
    v.assign((char*)buf, sz);
    free(buf);
    return u;
  }

  template <class T>
  inline Unpacker& operator>>(Unpacker &u, vector<T> &v) 
  {
    for (int sz = u.get_array(); sz > 0; --sz)
    {
      T element;
      u >> element;
      v.push_back(element);
    }
    return u;
  }

  template <class T>
  inline Unpacker& operator>>(Unpacker &u, set<T> &v) 
  {
    for (int sz = u.get_array(); sz > 0; --sz)
    {
      T element;
      u >> element;
      v.insert(element);
    }
    return u;
  }

  template <class K, class V>
  inline Unpacker& operator>>(Unpacker &u, map<K, V> &v) 
  {
    for (int sz = u.get_map(); sz > 0; --sz)
    {
      K key;
      u >> key;
      u >> v[key];
    }
    return u;
  }
};

#endif
