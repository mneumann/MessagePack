#ifndef __MESSAGEPACK_RESIZABLE_BUFFER__HEADER__
#define __MESSAGEPACK_RESIZABLE_BUFFER__HEADER__

namespace MessagePack
{

  class ResizableBuffer
  {
    private:

    void *_data;
    size_t _capacity;
    char _empty_buf; // is used as a special case when _capacity = 0 (see data()).

    public:

    ResizableBuffer()
    {
      _data = nullptr;
      _capacity = 0;
      _empty_buf = 0;
    }

    ~ResizableBuffer()
    {
      if (_data)
      {
        free(_data);
        _data = nullptr;
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

      void *d = nullptr;

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
        throw OutOfMemoryException("insufficient memory");
      }
    }
  };

} /* namespace MessagePack */

#endif
