#ifndef __MESSAGEPACK_WRITER__HEADER__
#define __MESSAGEPACK_WRITER__HEADER__

namespace MessagePack
{

  /*
   * Abstract base class of all WriteBuffer implementations
   */
  class Writer
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

  class FileWriter : public Writer
  {
    private:

    FILE *file;

    public:

    FileWriter(FILE *file)
    {
      this->file = file;
    }

    virtual ~FileWriter()
    {
      this->file = nullptr;
    }

    virtual void write(const void *buf, size_t len)
    {
      if (fwrite(buf, 1, len, this->file) != len)
        throw FileException("write error");
    }
  };

  class BufferedMemoryWriter : public Writer
  {
    private:

    ResizableBuffer _buf;
    size_t _write_pos;

    public:

    BufferedMemoryWriter(size_t initial_size)
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

} /* namespace MessagePack */

#endif
