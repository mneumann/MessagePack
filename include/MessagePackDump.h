#ifndef __MESSAGEPACK_DUMP__HEADER__
#define __MESSAGEPACK_DUMP__HEADER__

#include <stdint.h>   /* uint32_t ... */
#include <vector>
#include <set>
#include <map>
#include <string>
#include <string.h>
#include "MessagePack.h"

namespace MessagePack 
{
  using namespace std;

  inline Packer& operator<<(Packer& p, const uint8_t &v) { p.pack_uint(v); return p; }
  inline Packer& operator<<(Packer& p, const uint16_t &v) { p.pack_uint(v); return p; }
  inline Packer& operator<<(Packer& p, const uint32_t &v) { p.pack_uint(v); return p; }
  inline Packer& operator<<(Packer& p, const uint64_t &v) { p.pack_uint(v); return p; }
  inline Packer& operator<<(Packer& p, const int8_t &v) { p.pack_int(v); return p; }
  inline Packer& operator<<(Packer& p, const int16_t &v) { p.pack_int(v); return p; }
  inline Packer& operator<<(Packer& p, const int32_t &v) { p.pack_int(v); return p; }
  inline Packer& operator<<(Packer& p, const int64_t &v) { p.pack_int(v); return p; }
  inline Packer& operator<<(Packer& p, const float &v) { p.pack_float(v); return p; }
  inline Packer& operator<<(Packer& p, const double &v) { p.pack_double(v); return p; }
  inline Packer& operator<<(Packer& p, const bool &v) { p.pack_bool(v); return p; }

  inline Packer& operator<<(Packer& p, const string &v)
  {
    p.pack_raw(v.c_str(), v.size());
    return p;
  }

  inline Packer& operator<<(Packer& p, const char *v)
  {
    p.pack_raw(v, strlen(v));
    return p;
  }


  template <class T>
  Packer& operator<<(Packer& p, const vector<T> &v)
  {
    typedef typename vector<T>::const_iterator CI;
    p.pack_array(v.size());
    for (CI it=v.begin(); it != v.end(); ++it)
    {
      p << (*it);
    }
    return p;
  }

  template <class T>
  Packer& operator<<(Packer& p, const set<T> &v)
  {
    typedef typename set<T>::const_iterator CI;
    p.pack_array(v.size());
    for (CI it=v.begin(); it != v.end(); ++it)
    {
      p << (*it);
    }
    return p;
  }

  template <class K, class V>
  Packer& operator<<(Packer& p, const map<K, V> &v)
  {
    typedef typename map<K, V>::const_iterator CI;
    p.pack_map(v.size());
    for (CI it=v.begin(); it != v.end(); ++it)
    {
      p << it->first;
      p << it->second;
    }
    return p;
  }
};

#endif
