#ifndef __MESSAGEPACK_SERIALIZE__HEADER__
#define __MESSAGEPACK_SERIALIZE__HEADER__

#include <stdint.h>   /* uint32_t ... */
#include <assert.h>   /* assert() */
#include <stdlib.h>   /* malloc, free */
#include <vector>
#include <set>
#include <map>
#include <string>
#include <string.h>

#if (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L)
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#endif


namespace MessagePack 
{
  using namespace std;

  //
  // Encode
  //

  inline Encoder& operator<<(Encoder& p, const uint8_t &v) { p.emit_uint(v); return p; }
  inline Encoder& operator<<(Encoder& p, const uint16_t &v) { p.emit_uint(v); return p; }
  inline Encoder& operator<<(Encoder& p, const uint32_t &v) { p.emit_uint(v); return p; }
  inline Encoder& operator<<(Encoder& p, const uint64_t &v) { p.emit_uint(v); return p; }
  inline Encoder& operator<<(Encoder& p, const int8_t &v) { p.emit_int(v); return p; }
  inline Encoder& operator<<(Encoder& p, const int16_t &v) { p.emit_int(v); return p; }
  inline Encoder& operator<<(Encoder& p, const int32_t &v) { p.emit_int(v); return p; }
  inline Encoder& operator<<(Encoder& p, const int64_t &v) { p.emit_int(v); return p; }
  inline Encoder& operator<<(Encoder& p, const float &v) { p.emit_float(v); return p; }
  inline Encoder& operator<<(Encoder& p, const double &v) { p.emit_double(v); return p; }
  inline Encoder& operator<<(Encoder& p, const bool &v) { p.emit_bool(v); return p; }

  inline Encoder& operator<<(Encoder& p, const string &v)
  {
    p.emit_raw(v.c_str(), v.size());
    return p;
  }

  inline Encoder& operator<<(Encoder& p, const char *v)
  {
    p.emit_raw(v, strlen(v));
    return p;
  }

  template <class T>
  Encoder& operator<<(Encoder& p, const vector<T> &v)
  {
    typedef typename vector<T>::const_iterator CI;
    p.emit_array(v.size());
    for (CI it=v.begin(); it != v.end(); ++it)
    {
      p << (*it);
    }
    return p;
  }

  template <class T>
  Encoder& operator<<(Encoder& p, const set<T> &v)
  {
    typedef typename set<T>::const_iterator CI;
    p.emit_array(v.size());
    for (CI it=v.begin(); it != v.end(); ++it)
    {
      p << (*it);
    }
    return p;
  }

  template <class K, class V>
  Encoder& operator<<(Encoder& p, const map<K, V> &v)
  {
    typedef typename map<K, V>::const_iterator CI;
    p.emit_map(v.size());
    for (CI it=v.begin(); it != v.end(); ++it)
    {
      p << it->first;
      p << it->second;
    }
    return p;
  }

  #if (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L)

  /*
   * encode_interleaved:
   *
   * Encodes an array of tuples, e.g. 
   *
   *   [ (1,"a"), (2,"b"), (3,"c") ]
   *
   * in this way:
   *
   *   [1, 2, 3], ["a", "b", "c"]
   */

  template <int N, int S, typename ...Types> 
  struct _InterleavedEncoder {
    static void encode(Encoder &enc, const vector<tuple<Types...>> &array) {
      enc.emit_array(array.size());
      for (const auto &e : array) enc << get<N>(e);
      _InterleavedEncoder<N+1, S, Types...>::encode(enc, array);
    }
  };

  template <int S, typename ...Types> 
  struct _InterleavedEncoder<S, S, Types...> {
    static void encode(Encoder &enc, const vector<tuple<Types...>> &array) { }
  };

  template <typename ...Types>
  inline void encode_interleaved(Encoder &enc, const vector<tuple<Types...>> &array) {
    _InterleavedEncoder<0, sizeof...(Types), Types...>::encode(enc, array);
  };

  /* 
   * Encoder for tuples
   */
 
  template <int N, int S, typename ...Types>
  struct _TupleEncoder {
    static void encode(Encoder &enc, const tuple<Types...> &tuple) {
      enc << get<N>(tuple);
      _TupleEncoder<N+1, S, Types...>::encode(tuple);
    }
  };

  template <int S, typename ...Types>
  struct _TupleEncoder<S, S, Types...> {
    static void encode(Encoder& enc, const tuple<Types...> &tuple) { }
  };

  template <typename ...Types>
  Encoder& operator<<(Encoder& enc, const tuple<Types...> &v)
  {
    enc.emit_array(sizeof...(Types));
    _TupleEncoder<0, sizeof...(Types), Types...>::encode(enc, v);
    return enc;
  }

  template <class T>
  Encoder& operator<<(Encoder& p, const unordered_set<T> &v)
  {
    p.emit_array(v.size());
    for (const auto &elem : v)
    {
      p << v;
    }
    return p;
  }

  template <class K, class V>
  Encoder& operator<<(Encoder& p, const unordered_map<K, V> &v)
  {
    p.emit_map(v.size());
    for (const auto &elem : v)
    {
      p << elem.first << elem.second;
    }
    return p;
  }


  #endif


  //
  // Decode
  //

  inline Decoder& operator>>(Decoder &dec, uint8_t &v) 
  {
    v = dec.read_unsigned<uint8_t>(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, uint16_t &v) 
  {
    v = dec.read_unsigned<uint16_t>(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, uint32_t &v) 
  {
    v = dec.read_unsigned<uint32_t>(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, uint64_t &v) 
  {
    v = dec.read_unsigned<uint64_t>(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, int8_t &v) 
  {
    v = dec.read_signed<int8_t>(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, int16_t &v) 
  {
    v = dec.read_signed<int16_t>(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, int32_t &v) 
  {
    v = dec.read_signed<int32_t>(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, int64_t &v) 
  {
    v = dec.read_signed<int64_t>(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, float &v) 
  {
    v = dec.read_float(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, double &v) 
  {
    v = dec.read_double(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, bool &v) 
  {
    v = dec.read_bool(); return dec;
  }

  inline Decoder& operator>>(Decoder &dec, string &v) 
  {
    uint32_t sz = dec.read_raw();
#if 0
    // This is the safe way of doing it. But it needs two allocations!
    void *buf = malloc(sz);
    if (!buf) throw OutOfMemoryException();
    dec.read_raw_body(buf, sz);
    v.assign((char*)buf, sz);
    free(buf);
#else
    // XXX: This should work, however it's more of a hack
    v.resize(sz);
    dec.read_raw_body((char*)v.data(), sz);
#endif
    return dec;
  }

  inline Decoder& operator>>(Decoder &dec, char* &v) 
  {
    uint32_t sz = dec.read_raw();
    char *str = (char*)malloc(sz+1);
    if (!str) throw OutOfMemoryException();
    dec.read_raw_body(str, sz);
    str[sz] = '\0';
    v = str;
    return dec;
  }

  template <class T>
  inline Decoder& operator>>(Decoder &dec, vector<T> &v) 
  {
    size_t sz = dec.read_array();
    v.clear();
    v.reserve(sz);

    for (; sz > 0; --sz)
    {
      T element;
      dec >> element;
      v.push_back(element);
    }
    return dec;
  }

  template <class T>
  inline Decoder& operator>>(Decoder &dec, set<T> &v) 
  {
    for (int sz = dec.read_array(); sz > 0; --sz)
    {
      T element;
      dec >> element;
      v.insert(element);
    }
    return dec;
  }

  template <class K, class V>
  inline Decoder& operator>>(Decoder &dec, map<K, V> &v) 
  {
    for (int sz = dec.read_map(); sz > 0; --sz)
    {
      K key;
      dec >> key;
      dec >> v[key];
    }
    return dec;
  }

  #if (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L)
  template <int N, int S, typename ...Types>
  struct _TupleDecoder {
    static void decode(Decoder &dec, tuple<Types...> &tuple) {
      dec >> get<N>(tuple);
      _TupleDecoder<N+1, S, Types...>::decode(dec, tuple);
    }
  };

  template <int S, typename ...Types>
  struct _TupleDecoder<S, S, Types...> {
    static void decode(Decoder& dec, tuple<Types...> &tuple) { }
  };

  template <typename ...Types>
  Decoder& operator>>(Decoder& dec, tuple<Types...> &v)
  {
    if (dec.read_array() != sizeof...(Types)) {
      throw InvalidDecodeException("decode tuple");
    }
 
    _TupleDecoder<0, sizeof...(Types), Types...>::decode(dec, v);
    return dec;
  }

  template <class K>
  inline Decoder& operator>>(Decoder &dec, unordered_set<K> &v) 
  {
    for (auto sz = dec.read_array(); sz > 0; --sz)
    {
      K key;
      dec >> key;
      v.insert(key);
    }
    return dec;
  }

  template <class K, class V>
  inline Decoder& operator>>(Decoder &dec, unordered_map<K, V> &v) 
  {
    for (auto sz = dec.read_map(); sz > 0; --sz)
    {
      K key;
      dec >> key;
      dec >> v[key];
    }
    return dec;
  }

  #endif

}; /* namespace MessagePack */

#endif
