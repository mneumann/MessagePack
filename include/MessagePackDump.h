#ifndef __MESSAGEPACK_DUMP__HEADER__
#define __MESSAGEPACK_DUMP__HEADER__

#include <stdint.h>   /* uint32_t ... */
#include <vector>
#include <set>
#include <map>
#include <string>
#include "MessagePack.h"

namespace MessagePack 
{
  using namespace std;

  inline void dump(uint64_t value, Packer &pk)
  {
    pk.pack_uint(value);
  }

  inline void dump(uint32_t value, Packer &pk)
  {
    pk.pack_uint(value);
  }

  inline void dump(uint16_t value, Packer &pk)
  {
    pk.pack_uint(value);
  }

  inline void dump(uint8_t value, Packer &pk)
  {
    pk.pack_uint(value);
  }

  inline void dump(int64_t value, Packer &pk)
  {
    pk.pack_int(value);
  }

  inline void dump(int32_t value, Packer &pk)
  {
    pk.pack_int(value);
  }

  inline void dump(int16_t value, Packer &pk)
  {
    pk.pack_int(value);
  }

  inline void dump(int8_t value, Packer &pk)
  {
    pk.pack_int(value);
  }

  inline void dump(double value, Packer &pk)
  {
    pk.pack_double(value);
  }

  inline void dump(const string &value, Packer &pk)
  {
    pk.pack_raw(value.c_str(), value.size());
  }

  template <class T>
  void dump(const vector<T> &value, Packer &pk)
  {
    typedef typename vector<T>::const_iterator CI;
    pk.pack_array(value.size());
    for (CI it=value.begin(); it != value.end(); ++it)
    {
      dump(*it, pk);
    }
  }

  template <class T>
  void dump(const set<T> &value, Packer &pk)
  {
    typedef typename set<T>::const_iterator CI;
    pk.pack_array(value.size());
    for (CI it=value.begin(); it != value.end(); ++it)
    {
      dump(*it, pk);
    }
  }

  template <class K, class V>
  void dump(const map<K, V> &value, Packer &pk)
  {
    typedef typename map<K, V>::const_iterator CI;
    pk.pack_map(value.size());
    for (CI it=value.begin(); it != value.end(); ++it)
    {
      dump(it->first, pk);
      dump(it->second, pk);
    }
  }
};

#endif
