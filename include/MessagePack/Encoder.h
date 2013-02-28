#ifndef __MESSAGEPACK_ENCODER__HEADER__
#define __MESSAGEPACK_ENCODER__HEADER__

namespace MessagePack
{

  class Encoder
  {
    private:
    
    Writer *buffer;

    public:

    Encoder(Writer *buf) : buffer(buf) {}

    void set_writer(Writer *buf)
    {
      buffer = buf;
    }

    Writer *get_writer()
    {
      return buffer;
    }

    /*
     * handles positive fixnum (1 byte) and uint8 (2 bytes)
     */
    void emit_uint8(uint8_t v)
    {
      if ((v & 128) != 0)
      {
        buffer->write_byte(0xcc);
      }
      buffer->write_byte(v);
    }

    void emit_uint16(uint16_t v)
    {
      buffer->write_byte(0xcd);
      buffer->write2(v);
    }

    void emit_uint32(uint32_t v)
    {
      buffer->write_byte(0xce);
      buffer->write4(v);
    }

    void emit_uint64(uint64_t v)
    {
      buffer->write_byte(0xcf);
      buffer->write8(v);
    }

    void emit_uint(uint64_t v)
    {
      if      ((v & 0xFFFFFFFFFFFFFF00) == 0) emit_uint8(v);
      else if ((v & 0xFFFFFFFFFFFF0000) == 0) emit_uint16(v);
      else if ((v & 0xFFFFFFFF00000000) == 0) emit_uint32(v);
      else                                    emit_uint64(v);
    }

    /*
     * handles negative fixnum (1 byte) and int8 (2 bytes)
     */
    void emit_int8(int8_t v)
    {
      if ((v & 0xe0) != 0xe0)
      {
        buffer->write_byte(0xd0);
      }
      buffer->write_byte((uint8_t)v);
    }

    void emit_int16(int16_t v)
    {
      buffer->write_byte(0xd1);
      buffer->write2(v);
    }

    void emit_int32(int32_t v)
    {
      buffer->write_byte(0xd2);
      buffer->write4(v);
    }

    void emit_int64(int64_t v)
    {
      buffer->write_byte(0xd3);
      buffer->write8(v);
    }

    void emit_int(int64_t v)
    {
      if      (v >= -(1L<<7)  && v <= (1L<<7)-1)  emit_int8(v);
      else if (v >= -(1L<<15) && v <= (1L<<15)-1) emit_int16(v);
      else if (v >= -(1L<<31) && v <= (1L<<31)-1) emit_int32(v);
      else /*if (v >= -(1L<<63) && v <= (1L<<63)-1)*/ emit_int64(v);
    }

    void emit_nil()
    {
      buffer->write_byte(0xc0);
    }

    void emit_true()
    {
      buffer->write_byte(0xc3);
    }

    void emit_false()
    {
      buffer->write_byte(0xc2);
    }

    void emit_bool(bool v)
    {
      if (v) emit_true();
      else emit_false();
    }

    void emit_float(float v)
    {
      buffer->write_byte(0xca);
      buffer->write_float(v);
    }

    void emit_double(double v)
    {
      buffer->write_byte(0xcb);
      buffer->write_double(v);
    }

    void emit_raw(const char *raw, uint32_t len)
    {
      if (len <= 31)
      {
        // fix raw 101XXXXX
        buffer->write_byte(0xa0 | len);
      }
      else if (len <= 0xFFFF) 
      {
        // raw 16
        buffer->write_byte(0xda);
        buffer->write2(len);
      }
      else
      {
        buffer->write_byte(0xdb);
        buffer->write4(len);
      }

      if (len > 0)
      {
        buffer->write(raw, len);
      }
    }

    void emit_array(uint32_t len)
    {
      if (len <= 15)
      {
        // fix array 1001XXXX
        buffer->write_byte(0x90 | len);
      }
      else if (len <= 0xFFFF)
      {
        buffer->write_byte(0xdc);
        buffer->write2(len);
      }
      else
      {
        buffer->write_byte(0xdd);
        buffer->write4(len);
      }
    }

    void emit_map(uint32_t len)
    {
      if (len <= 15)
      {
        // fix map 1000XXXX
        buffer->write_byte(0x80 | len);
      }
      else if (len <= 0xFFFF)
      {
        buffer->write_byte(0xde);
        buffer->write2(len);
      }
      else
      {
        buffer->write_byte(0xdf);
        buffer->write4(len);
      }
    }

  };

} /* namespace MessagePack */

#endif
