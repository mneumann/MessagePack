#ifndef __MESSAGEPACK_READER__HEADER__
#define __MESSAGEPACK_READER__HEADER__

#include<boost/numeric/conversion/cast.hpp>

namespace MessagePack
{

  /*
   * Abstract base class of all Reader implementations
   */
  class Reader
  {
    public:

    virtual void read(void *buffer, size_t sz) = 0;
    virtual bool at_end() = 0;

    virtual ~Reader(){}

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

  class MemoryReader : public Reader
  {
    private:

    const char *_data;
    size_t _pos;
    size_t _size;

    public:

    MemoryReader(const char *str, size_t sz)
    {
      _data = str; 
      _size = sz;
      _pos = 0;
    }

    virtual ~MemoryReader() {}

    virtual void read(void *buffer, size_t sz)
    {
      needs_bytes(sz);
      memcpy(buffer, &_data[_pos], sz);
      _pos += sz;
    }

    virtual bool at_end()
    {
      assert(_pos <= _size);
      return(_pos == _size);
    }

    private:

    void needs_bytes(size_t n)
    {
      if (_pos + n > _size)
        throw EofException("read over buffer boundaries");
    }
  };

  class FileReader : public Reader
  {
    private:

    FILE *_file;
    size_t _pos;
    size_t _size;
    bool _close_file;

    public:

    FileReader(FILE *file, size_t filesz)
    {
      _file = file;
      _size = filesz;
      _pos = 0;
      _close_file = false;
    }

    FileReader(const char *filename)
    {
      FILE *file = fopen(filename, "r");
      if (!file) throw FileException("Failed to open file");

      if (fseek(file, 0, SEEK_END) != 0)
      {
        fclose(file);
        throw FileException("fseek failed");
      }

      long sz = ftell(file);
      if (sz < 0)
      {
        fclose(file);
        throw FileException("ftell failed");
      }

      if (fseek(file, 0, SEEK_SET) != 0)
      {
        fclose(file);
        throw FileException("fseek failed");
      }

      _file = file;
      _size = boost::numeric_cast<size_t>(sz);
      _pos = 0;
      _close_file = true;
    }

    virtual ~FileReader()
    {
      if (_close_file && _file)
      {
        fclose(_file);
        _file = nullptr;
      }
    }

    virtual void read(void *buffer, size_t sz)
    {
      needs_bytes(sz);
      if (fread(buffer, sz, 1, _file) != 1)
      {
        throw FileException("fread failed");
      }
      _pos += sz;
    }

    virtual bool at_end()
    {
      assert(_pos <= _size);
      return(_pos == _size);
    }

    private:

    inline void needs_bytes(size_t n)
    {
      if (_pos + n > _size)
        throw EofException("read over buffer boundaries");
    }
  };

} /* namespace MessagePack */

#endif
