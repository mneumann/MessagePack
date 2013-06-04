/*
 * Implements the msgpack.org specification.
 *
 * Copyright (c) 2011-2013 by Michael Neumann (mneumann@ntecs.de)
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
#ifdef __linux__
  #include <endian.h>
#elif __APPLE__
  #include "MacEndian.h"
#else
  #include <sys/endian.h>
#endif
#include <assert.h>   /* assert */
#include <limits> /* numeric_limits */

#if !(defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L)
  #define nullptr NULL
#endif

#include "Exception.h"
#include "ResizableBuffer.h"
#include "Reader.h"
#include "Writer.h"
#include "Encoder.h"
#include "Decoder.h"

namespace MessagePack
{
  template <typename T>
  inline void load_from_file(const char *filename, T &store)
  {
    FileReader r(filename);
    Decoder dec(&r);
    dec >> store;
  }

} /* namespace MessagePack */

#endif
