#ifndef __MESSAGEPACK_DECODER__HEADER__
#define __MESSAGEPACK_DECODER__HEADER__

namespace MessagePack
{

  enum DataType
  {
    MSGPACK_T_UINT,
    MSGPACK_T_INT,
    MSGPACK_T_FLOAT,
    MSGPACK_T_DOUBLE,
    MSGPACK_T_NIL,
    MSGPACK_T_BOOL,
    MSGPACK_T_ARRAY,
    MSGPACK_T_MAP,
    MSGPACK_T_RAW,
    MSGPACK_T_RESERVED,
    MSGPACK_T_INVALID
  };

  union DataValue
  {
    bool     b;
    uint64_t u;
    int64_t  i;
    float    f;
    double   d;
    uint32_t len;
  };

  class Decoder
  {
    private:

    Reader *buffer;

    public:

    Reader *get_reader() const { return buffer; }

    Decoder(Reader *buf) : buffer(buf) { }

    /*
     * Returns the next data item in data.
     */
    inline DataType read_next(DataValue &data)
    {
      uint8_t c = buffer->read_byte();
      if (c <= 0x7f) {
        data.u = c;
        return MSGPACK_T_UINT;
      } else if (c <= 0x8f) {
        data.len = c & 0x0F;
        return MSGPACK_T_MAP;
      } else if (c <= 0x9f) {
        data.len = c & 0x0F;
        return MSGPACK_T_ARRAY;
      } else if (c <= 0xbf) {
        data.len = c & 0x1F;
        return MSGPACK_T_RAW;
      } else if (c >= 0xe0) {
        data.i = (int8_t) c;
        return MSGPACK_T_INT;
      } else {
        switch (c) {
          case 0xc0:
            return MSGPACK_T_NIL;
          case 0xc4:
          case 0xc5:
          case 0xc1:
          case 0xc6:
          case 0xc7:
          case 0xc8:
          case 0xc9:
          case 0xd4:
          case 0xd5:
          case 0xd6:
          case 0xd7:
          case 0xd8:
          case 0xd9:
            return MSGPACK_T_RESERVED;
          case 0xc2: 
            data.b = false;
            return MSGPACK_T_BOOL;
          case 0xc3:
            data.b = true;
            return MSGPACK_T_BOOL;
          case 0xca:
            data.f = buffer->read_float();
            return MSGPACK_T_FLOAT;
          case 0xcb:
            data.d = buffer->read_double();
            return MSGPACK_T_DOUBLE;
          case 0xcc:
            data.u = buffer->read_byte();
            return MSGPACK_T_UINT;
          case 0xcd:
            data.u = buffer->read2();
            return MSGPACK_T_UINT;
          case 0xce:
            data.u = buffer->read4();
            return MSGPACK_T_UINT;
          case 0xcf:
            data.u = buffer->read8();
            return MSGPACK_T_UINT;
          case 0xd0:
            data.i = (int8_t)buffer->read_byte();
            return MSGPACK_T_INT;
          case 0xd1:
            data.i = (int16_t)buffer->read2();
            return MSGPACK_T_INT;
          case 0xd2:
            data.i = (int32_t)buffer->read4();
            return MSGPACK_T_INT;
          case 0xd3:
            data.i = (int64_t)buffer->read8();
            return MSGPACK_T_INT;
          case 0xda:
            data.len = buffer->read2();
            return MSGPACK_T_RAW;
          case 0xdb:
            data.len = buffer->read4();
            return MSGPACK_T_RAW;
          case 0xdc:
            data.len = buffer->read2();
            return MSGPACK_T_ARRAY;
          case 0xdd:
            data.len = buffer->read4();
            return MSGPACK_T_ARRAY;
          case 0xde:
            data.len = buffer->read2();
            return MSGPACK_T_MAP;
          case 0xdf:
            data.len = buffer->read4();
            return MSGPACK_T_MAP;
        };
      }

      return MSGPACK_T_INVALID;
    }

    // T should be an unsigned type!
    template <class T>
    T read_unsigned()
    {
      DataValue d;

      switch (read_next(d))
      {
        case MSGPACK_T_UINT:
          break;
        case MSGPACK_T_INT:
          if (d.i < 0) throw InvalidDecodeException("unpack_unsigned: negative value");
          break;
	case MSGPACK_T_FLOAT:
	case MSGPACK_T_DOUBLE:
	case MSGPACK_T_NIL:
	case MSGPACK_T_BOOL:
	case MSGPACK_T_ARRAY:
	case MSGPACK_T_MAP:
	case MSGPACK_T_RAW:
	case MSGPACK_T_RESERVED:
	case MSGPACK_T_INVALID:
          throw InvalidDecodeException("unpack_unsigned: no integer given");
      }

      if (d.u <= std::numeric_limits<T>::max())
      {
        return (T)d.u;
      }
      else
      {
        throw InvalidDecodeException("unpack_unsigned: out of range");
      }
    }
 
    // T should be a signed type
    template <class T>
    T read_signed()
    {
      DataValue d;

      switch (read_next(d))
      {
        case MSGPACK_T_INT:
          break;
        case MSGPACK_T_UINT:
          if (d.u > (uint64_t)std::numeric_limits<int64_t>::max())
            throw InvalidDecodeException("unpack_signed: unsigned value too large");
          break;
	case MSGPACK_T_FLOAT:
	case MSGPACK_T_DOUBLE:
	case MSGPACK_T_NIL:
	case MSGPACK_T_BOOL:
	case MSGPACK_T_ARRAY:
	case MSGPACK_T_MAP:
	case MSGPACK_T_RAW:
	case MSGPACK_T_RESERVED:
	case MSGPACK_T_INVALID:
          throw InvalidDecodeException("unpack_signed: no integer given");
      }

      if (d.i >= std::numeric_limits<T>::min() && d.i <= std::numeric_limits<T>::max())
      {
        return (T)d.i;
      }
      else
      {
        throw InvalidDecodeException("unpack_signed: out of range");
      }
    }

    #define DEF_READ(rettype, fnname, msgpacktype, field) \
      rettype read_ ## fnname() { \
      DataValue d; \
      if (read_next(d) == MSGPACK_T_ ## msgpacktype) return d.field; \
      throw InvalidDecodeException("read_" # fnname); \
    }

    DEF_READ(uint64_t, uint, UINT, u)
    DEF_READ(int64_t, int, INT, i)
    DEF_READ(float, float, FLOAT, f)
    DEF_READ(double, double, DOUBLE, d)
    DEF_READ(uint32_t, raw, RAW, len)
    DEF_READ(uint32_t, array, ARRAY, len)
    DEF_READ(uint32_t, map, MAP, len)
    DEF_READ(bool, bool, BOOL, b)

    #undef DEF_READ
 
    void read_raw_body(void *buf, size_t sz)
    {
      if (sz == 0) return;
      buffer->read(buf, sz);
    }

    void read_nil()
    {
      DataValue d;
      if (read_next(d) != MSGPACK_T_NIL)
        throw InvalidDecodeException("read_nil");
    }

    void read_array(uint32_t size)
    {
      if (read_array() != size) {
        throw InvalidDecodeException("read_array gave invalid size");
      }
    }

  };

} /* namespace MessagePack */

#endif
