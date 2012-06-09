/*
 * Implements the msgpack.org specification.
 *
 * Copyright (c) 2011, 2012 by Michael Neumann (mneumann@ntecs.de)
 *
 * Why another implementation?
 *
 *   - The C++ implementation found on msgpack.org does not support
 *     direct deserialization.
 *
 *   - No inheritance of the Buffer implementations.
 *
 *   - I do not fully understand the code! :) :) :)
 *
 * Extensions to the msgpack.org standard (USE_MSGPACK_EXTENSIONS):
 *
 *   (1) Size-less arrays: 0xc4 item0, ..., itemN 0xc5
 *
 *       This can be used to easily dump objects as arrays, without
 *       manually counting the number of fields beforehand.
 */

#ifndef __MESSAGEPACK__HEADER__
#define __MESSAGEPACK__HEADER__

#include <stdint.h>   /* uint32_t ... */
#include <string.h>   /* memcpy() */
#include <stdlib.h>   /* malloc() */
#include <stdio.h>    /* FILE, fwrite() */
#ifdef linux
  #include <endian.h>
#else
  #include <sys/endian.h>
#endif
#include <assert.h>   /* assert */

namespace MessagePack
{

  /*
   * Abstract base class of all WriteBuffer implementations
   */
  class WriteBuffer
  {
    public:

    virtual void write_byte(uint8_t byte)
    {
      write(&byte, 1);
    }

    virtual void write2(uint16_t v)
    {
      v = htobe16(v);
      write(&v, 2);
    }

    virtual void write4(uint32_t v)
    {
      v = htobe32(v);
      write(&v, 4);
    }

    virtual void write8(uint64_t v)
    {
      v = htobe64(v);
      write(&v, 8);
    }

    virtual void write_float(float v)
    {
      write4(*((uint32_t*)&v));
    }

    virtual void write_double(double v)
    {
      write8(*((uint64_t*)&v));
    }

    virtual void write(const void *buf, size_t len) = 0;
  };

  class FileWriteBuffer : public WriteBuffer
  {
    private:

    FILE *file;

    public:

    FileWriteBuffer(FILE *file)
    {
      this->file = file;
    }

    ~FileWriteBuffer()
    {
      this->file = NULL;
    }

    virtual void write(const void *buf, size_t len)
    {
      if (fwrite(buf, 1, len, this->file) != len)
        throw "write error";
    }
  };

  class ResizableBuffer
  {
    private:

    void *_data;
    size_t _capacity;
    char _empty_buf; // is used as a special case when _capacity = 0 (see data()).

    public:

    ResizableBuffer()
    {
      _data = NULL;
      _capacity = 0;
      _empty_buf = 0;
    }

    ~ResizableBuffer()
    {
      if (_data)
      {
        free(_data);
	_data = NULL;
      }
      _capacity = 0;
    }

    size_t capacity() const
    {
      return _capacity;
    }

    const void *data() const
    {
      if (_capacity == 0)
      {
        return &_empty_buf;
      }
      else
      {
        return _data;
      }
    }

    void *ptr_at(size_t offs, size_t len)
    {
      assert(len > 0);
      resize(offs + len);
      assert(_data);
      return (void*)(((char*)_data)+offs);
    }

    void resize(size_t req)
    {
      if (req < _capacity) return;

      void *d = NULL;

      size_t new_size = _capacity * 2;
      if (new_size < 16) new_size = 16;
      while (req > new_size) new_size *= 2;

      if (_data)
      {
        d = realloc(_data, new_size);
      }
      else
      {
        d = malloc(new_size);
      }

      if (d)
      {
        _data = d;
	_capacity = new_size;
      }
      else
      {
        throw "insufficient memory";
      }
    }
  };

  class MemoryWriteBuffer : public WriteBuffer
  {
    private:

    ResizableBuffer _buf;
    size_t _write_pos;

    public:

    MemoryWriteBuffer(size_t initial_size)
    {
      _buf.resize(initial_size);
      _write_pos = 0;
    }

    size_t size() const
    {
      return _write_pos;
    }

    const void *data() const
    {
      return _buf.data();
    }

    void reset()
    {
      _write_pos = 0;
    }

    virtual void write_byte(uint8_t byte)
    {
      *((uint8_t*)_buf.ptr_at(_write_pos, 1)) = byte;
      ++_write_pos;
    }

    virtual void write(const void *buf, size_t len)
    {
      memcpy(_buf.ptr_at(_write_pos, len), buf, len);
      _write_pos += len;
    }
  };

  struct Packer
  {
    WriteBuffer *buffer;

    Packer(WriteBuffer *buf) : buffer(buf) {}

    void set_buffer(WriteBuffer *buf)
    {
      buffer = buf;
    }

    WriteBuffer *get_buffer()
    {
      return buffer;
    }

    /*
     * handles positive fixnum (1 byte) and uint8 (2 bytes)
     */
    void pack_uint8(uint8_t v)
    {
      if ((v & 128) != 0)
      {
        buffer->write_byte(0xcc);
      }
      buffer->write_byte(v);
    }

    void pack_uint16(uint16_t v)
    {
      buffer->write_byte(0xcd);
      buffer->write2(v);
    }

    void pack_uint32(uint32_t v)
    {
      buffer->write_byte(0xce);
      buffer->write4(v);
    }

    void pack_uint64(uint64_t v)
    {
      buffer->write_byte(0xcf);
      buffer->write8(v);
    }

    void pack_uint(uint64_t v)
    {
      if      ((v & 0xFFFFFFFFFFFFFF00) == 0) pack_uint8(v);
      else if ((v & 0xFFFFFFFFFFFF0000) == 0) pack_uint16(v);
      else if ((v & 0xFFFFFFFF00000000) == 0) pack_uint32(v);
      else                                    pack_uint64(v);
    }

    /*
     * handles negative fixnum (1 byte) and int8 (2 bytes)
     */
    void pack_int8(int8_t v)
    {
      if ((v & 0xe0) != 0xe0)
      {
        buffer->write_byte(0xd0);
      }
      buffer->write_byte((uint8_t)v);
    }

    void pack_int16(int16_t v)
    {
      buffer->write_byte(0xd1);
      buffer->write2(v);
    }

    void pack_int32(int32_t v)
    {
      buffer->write_byte(0xd2);
      buffer->write4(v);
    }

    void pack_int64(int64_t v)
    {
      buffer->write_byte(0xd3);
      buffer->write8(v);
    }

    void pack_int(int64_t v)
    {
      if      (v >= -(1L<<7)  && v <= (1L<<7)-1)  pack_int8(v);
      else if (v >= -(1L<<15) && v <= (1L<<15)-1) pack_int16(v);
      else if (v >= -(1L<<31) && v <= (1L<<31)-1) pack_int32(v);
      else /*if (v >= -(1L<<63) && v <= (1L<<63)-1)*/ pack_int64(v);
    }

    void pack_nil()
    {
      buffer->write_byte(0xc0);
    }

    void pack_true()
    {
      buffer->write_byte(0xc3);
    }

    void pack_false()
    {
      buffer->write_byte(0xc2);
    }

    void pack_bool(bool v)
    {
      if (v) pack_true();
      else pack_false();
    }

    void pack_float(float v)
    {
      buffer->write_byte(0xca);
      buffer->write_float(v);
    }

    void pack_double(double v)
    {
      buffer->write_byte(0xcb);
      buffer->write_double(v);
    }

    void pack_raw(const char *raw, uint32_t len)
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

      buffer->write(raw, len);
    }

    void pack_array(uint32_t len)
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

#ifdef USE_MSGPACK_EXTENSIONS
    void pack_array_beg()
    {
        buffer->write_byte(0xc4);
    }

    void pack_array_end()
    {
        buffer->write_byte(0xc5);
    }
#endif

    void pack_map(uint32_t len)
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

  /*
   * Abstract base class of all ReadBuffer implementations
   */
  class ReadBuffer
  {
    public:

    virtual bool can_read(size_t n) const = 0;
    virtual void unread(size_t n) = 0;
    virtual void read(void *buffer, size_t sz) = 0;

    bool at_end() const
    {
      return !can_read(1);
    }

    uint8_t read_byte()
    {
      uint8_t v;
      read(&v, 1);
      return v;
    }

    uint16_t read2()
    {
      uint16_t v;
      read(&v, 2);
      return be16toh(v);
    }

    uint32_t read4()
    {
      uint32_t v;
      read(&v, 4);
      return be32toh(v);
    }

    uint64_t read8()
    {
      uint64_t v;
      read(&v, 8);
      return be64toh(v);
    }

    float read_float()
    {
      uint32_t v = read4();
      return *((float*)&v);
    }

    double read_double()
    {
      uint64_t v = read8();
      return *((double*)&v);
    }
  };

  class MemoryReadBuffer : public ReadBuffer
  {
    private:

    const char *_data;
    size_t _pos;
    size_t _size;

    public:

    MemoryReadBuffer(const char *str, size_t sz)
    {
      _data = str; 
      _size = sz;
      _pos = 0;
    }

    virtual bool can_read(size_t n) const
    {
      if (_pos + n <= _size)
        return true;
      else
        return false;
    }

    virtual void unread(size_t n)
    {
      if (n > _pos)
        throw "cannot unread more than read";
      _pos -= n;
    }

    virtual void read(void *buffer, size_t sz)
    {
      needs_bytes(sz);
      memcpy(buffer, &_data[_pos], sz);
      _pos += sz;
    }

    private:

    void needs_bytes(size_t n)
    {
      if (_pos + n > _size)
        throw "read over buffer boundaries";
    }
  };

  struct FileException
  {
    const char *msg;
    FileException() : msg("") {}
    FileException(const char *m) : msg(m) {}
  };

  class FileReadBuffer : public ReadBuffer
  {
    private:

    FILE *_file;
    size_t _pos;
    size_t _size;
    bool _close_file;

    public:

    FileReadBuffer(FILE *file, size_t filesz)
    {
      _file = file;
      _size = filesz;
      _pos = 0;
      _close_file = false;
    }

    FileReadBuffer(const char *filename)
    {
      FILE *file = fopen(filename, "r");
      if (!file) throw new FileException("Failed to open file");

      if (fseek(file, 0, SEEK_END) != 0)
      {
        fclose(file);
	throw new FileException("fseek failed");
      }

      long sz = ftell(file);
      if (sz < 0)
      {
        fclose(file);
	throw new FileException("ftell failed");
      }

      if (fseek(file, 0, SEEK_SET) != 0)
      {
        fclose(file);
	throw new FileException("fseek failed");
      }

      _file = file;
      _size = sz;
      _pos = 0;
      _close_file = true;
    }

    ~FileReadBuffer()
    {
      if (_close_file && _file)
      {
        fclose(_file);
	_file = NULL;
      }
    }

    virtual bool can_read(size_t n) const
    {
      if (_pos + n <= _size)
        return true;
      else
        return false;
    }

    virtual void unread(size_t n)
    {
      if (n > _pos) throw "cannot unread more than read";
      _pos -= n;
      if (fseek(_file, -n, SEEK_CUR) != 0)
      {
        throw new FileException("fseek failed");
      }
    }

    virtual void read(void *buffer, size_t sz)
    {
      needs_bytes(sz);
      if (fread(buffer, sz, 1, _file) != 1)
      {
        throw new FileException("fread failed");
      }
      _pos += sz;
    }

    private:

    void needs_bytes(size_t n)
    {
      if (_pos + n > _size)
        throw "read over buffer boundaries";
    }
  };

  enum DataType {
    MSGPACK_T_UINT,
    MSGPACK_T_INT,
    MSGPACK_T_FLOAT,
    MSGPACK_T_DOUBLE,
    MSGPACK_T_NIL,
    MSGPACK_T_TRUE,
    MSGPACK_T_FALSE,
    MSGPACK_T_ARRAY,
    MSGPACK_T_MAP,
    MSGPACK_T_RAW,
#ifdef USE_MSGPACK_EXTENSIONS
    MSGPACK_T_ARRAY_BEG,
    MSGPACK_T_ARRAY_END,
#endif
    MSGPACK_T_RESERVED,
    MSGPACK_T_INVALID,
    MSGPACK_T_NEED_MORE_DATA
  };

  struct Data
  {
    DataType type;
    union {
      uint64_t u;
      int64_t  i;
      float    f;
      double   d;
      uint32_t len;
    } value;
  };

  struct InvalidUnpackException
  {
  };

  struct Unpacker
  {
    ReadBuffer *buffer;

    Unpacker(ReadBuffer *buf) : buffer(buf) { }

    bool can_read(size_t n, Data &data)
    {
      if (!buffer->can_read(n)) {
        data.type = MSGPACK_T_NEED_MORE_DATA;
        data.value.len = n;
        return false;
      }
      return true;
    }

    /*
     * Returns the number of bytes required to read the next item.
     *
     * On success, return 0.
     *
     */
    size_t read_next(Data &data)
    {
      #define CAN_READ(n, unr) \
        if (!can_read(n, data)) { \
          buffer->unread(unr); \
          return data.value.len+unr; \
        }

      if (!can_read(1, data)) return data.value.len;
      uint8_t c = buffer->read_byte();
      if (c <= 0x7f) {
        data.type = MSGPACK_T_UINT;
        data.value.u = c;
      } else if (c <= 0x8f) {
        data.type = MSGPACK_T_MAP;
        data.value.len = c & 0x0F;
      } else if (c <= 0x9f) {
        data.type = MSGPACK_T_ARRAY;
        data.value.len = c & 0x0F;
      } else if (c <= 0xbf) {
        data.type = MSGPACK_T_RAW;
        data.value.len = c & 0x1F;
      } else if (c >= 0xe0) {
        data.type = MSGPACK_T_INT;
        data.value.i = (int8_t) c;
      } else {
        switch (c) {
          case 0xc0:
            data.type = MSGPACK_T_NIL;
            break; 
#ifdef USE_MSGPACK_EXTENSIONS
          case 0xc4:
	    data.type = MSGPACK_T_ARRAY_BEG;
	    break;
          case 0xc5:
 	    data.type = MSGPACK_T_ARRAY_END;
	    break;
#else
          case 0xc4:
          case 0xc5:
            data.type = MSGPACK_T_RESERVED;
            break;
#endif
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
            data.type = MSGPACK_T_RESERVED;
            break;
          case 0xc2: 
            data.type = MSGPACK_T_FALSE;
            break;
          case 0xc3:
            data.type = MSGPACK_T_TRUE;
            break;
          case 0xca:
            CAN_READ(4, 1)
            data.type = MSGPACK_T_FLOAT;
            data.value.f = buffer->read_float();
            break;
          case 0xcb:
            CAN_READ(8, 1)
            data.type = MSGPACK_T_DOUBLE;
            data.value.d = buffer->read_double();
            break;
          case 0xcc:
            CAN_READ(1, 1)
            data.type = MSGPACK_T_UINT;
            data.value.u = buffer->read_byte();
            break;
          case 0xcd:
            CAN_READ(2, 1)
            data.type = MSGPACK_T_UINT;
            data.value.u = buffer->read2();
            break;
          case 0xce:
            CAN_READ(4, 1)
            data.type = MSGPACK_T_UINT;
            data.value.u = buffer->read4();
            break;
          case 0xcf:
            CAN_READ(8, 1)
            data.type = MSGPACK_T_UINT;
            data.value.u = buffer->read8();
            break;
          case 0xd0:
            CAN_READ(1, 1)
            data.type = MSGPACK_T_INT;
            data.value.i = (int8_t)buffer->read_byte();
            break;
          case 0xd1:
            CAN_READ(2, 1)
            data.type = MSGPACK_T_INT;
            data.value.i = (int16_t)buffer->read2();
            break;
          case 0xd2:
            CAN_READ(4, 1)
            data.type = MSGPACK_T_INT;
            data.value.i = (int32_t)buffer->read4();
            break;
          case 0xd3:
            CAN_READ(8, 1)
            data.type = MSGPACK_T_INT;
            data.value.i = (int64_t)buffer->read8();
            break;
          case 0xda:
            CAN_READ(2, 1)
            data.type = MSGPACK_T_RAW;
            data.value.len = buffer->read2();
            CAN_READ(data.value.len, 2+1)
            break;
          case 0xdb:
            CAN_READ(4, 1)
            data.type = MSGPACK_T_RAW;
            data.value.len = buffer->read4();
            CAN_READ(data.value.len, 4+1)
            break;
          case 0xdc:
            CAN_READ(2, 1)
            data.type = MSGPACK_T_ARRAY;
            data.value.len = buffer->read2();
            break;
          case 0xdd:
            CAN_READ(4, 1)
            data.type = MSGPACK_T_ARRAY;
            data.value.len = buffer->read4();
            break;
          case 0xde:
            CAN_READ(2, 1)
            data.type = MSGPACK_T_MAP;
            data.value.len = buffer->read2();
            break;
          case 0xdf:
            CAN_READ(4, 1)
            data.type = MSGPACK_T_MAP;
            data.value.len = buffer->read4();
            break;
          default:
            data.type = MSGPACK_T_INVALID;
        };
      }

      return 0;
    }

    bool read_uint(uint64_t &v)
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_UINT)
      {
        v = d.value.u;
        return true;
      }
      return false;
    }

    bool read_int(int64_t &v)
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_INT)
      {
        v = d.value.i;
        return true;
      }
      return false;
    }

    bool read_float(float &v)
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_FLOAT)
      {
        v = d.value.f;
        return true;
      }
      return false;
    }

    bool read_double(double &v)
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_DOUBLE)
      {
        v = d.value.d;
        return true;
      }
      return false;
    }

    bool read_raw(uint32_t &v)
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_RAW)
      {
        v = d.value.len;
        return true;
      }
      return false;
    }

    bool read_raw_body(void *buf, size_t sz)
    {
      if (buffer->can_read(sz))
      {
        buffer->read(buf, sz);
        return true;
      }
      return false;
    }

    bool read_array(uint32_t &v)
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_ARRAY)
      {
        v = d.value.len;
        return true;
      }
      return false;
    }

#ifdef USE_MSGPACK_EXTENSIONS
    bool read_array_beg()
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_ARRAY_BEG)
      {
        return true;
      }
      return false;
    }

    bool read_array_end()
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_ARRAY_END)
      {
        return true;
      }
      return false;
    }
#endif

    bool read_map(uint32_t &v)
    {
      Data d;
      if (read_next(d) == 0 && d.type == MSGPACK_T_MAP)
      {
        v = d.value.len;
        return true;
      }
      return false;
    }

    bool read_bool(bool &v)
    {
      Data d;
      if (read_next(d) != 0) return false;

      if (d.type == MSGPACK_T_TRUE) {
        v = true;
        return true;
      }

      if (d.type == MSGPACK_T_FALSE) {
        v = false;
        return true;
      }

      return false;
    }

    uint64_t get_uint()
    {
      uint64_t v;
      if (read_uint(v)) return v;
      else throw new InvalidUnpackException;
    }

    int64_t get_int()
    {
      int64_t v;
      if (read_int(v)) return v;
      else throw new InvalidUnpackException;
    }

    bool get_bool()
    {
      bool v;
      if (read_bool(v)) return v;
      else throw new InvalidUnpackException;
    }

    float get_float()
    {
      float v;
      if (read_float(v)) return v;
      else throw new InvalidUnpackException;
    }

    double get_double()
    {
      double v;
      if (read_double(v)) return v;
      else throw new InvalidUnpackException;
    }

    uint32_t get_raw()
    {
      uint32_t v;
      if (read_raw(v)) return v;
      else throw new InvalidUnpackException;
    }

    /*
     * Read the raw body into buf which must be of size 'sz' 
     */
    void get_raw_body(void *buf, size_t sz)
    {
      if (buffer->can_read(sz))
      {
        buffer->read(buf, sz);
      }
      else throw new InvalidUnpackException;
    }

    uint32_t get_array()
    {
      uint32_t v;
      if (read_array(v)) return v;
      else throw new InvalidUnpackException;
    }

    void get_array(uint32_t expected_size)
    {
      if (get_array() != expected_size)
        throw new InvalidUnpackException;
    }

    uint32_t get_map()
    {
      uint32_t v;
      if (read_map(v)) return v;
      else throw new InvalidUnpackException;
    }

#ifdef USE_MSGPACK_EXTENSIONS
    void get_array_beg()
    {
      if (!read_array_beg())
          throw new InvalidUnpackException;
    }

    void get_array_end()
    {
      if (!read_array_end())
          throw new InvalidUnpackException;
    }
#endif

  }; /* Unpacker */

}

#endif
