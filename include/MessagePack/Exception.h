#ifndef __MESSAGEPACK_EXCEPTION__HEADER__
#define __MESSAGEPACK_EXCEPTION__HEADER__

namespace MessagePack
{

  struct Exception
  {
    const char *msg;
    Exception() : msg("") {}
    Exception(const char *_msg) : msg(_msg) {}
   };

  struct InvalidDecodeException : Exception
  {
    InvalidDecodeException() {}
    InvalidDecodeException(const char *_msg) : Exception(_msg) {}
  };

  struct EofException : Exception
  {
    EofException() {}
    EofException(const char *_msg) : Exception(_msg) {}
  };

  struct OutOfMemoryException : Exception
  {
    OutOfMemoryException() {}
    OutOfMemoryException(const char *_msg) : Exception(_msg) {}
  };

  struct FileException : Exception
  {
    FileException() {}
    FileException(const char *_msg) : Exception(_msg) {}
  };

} /* namespace MessagePack */

#endif
