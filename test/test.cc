#include <stdio.h>
#include <iostream>
#include "MessagePack.h"

int main(int argc, char **argv)
{
  using namespace MessagePack;
  using namespace std;

  MemoryWriteBuffer buf(1024);
  Packer pk(&buf);

  pk.pack_uint8(123);
  pk.pack_uint32(123);
  pk.pack_double(123.12);

  /*
  cout << buf.size() << endl;
  FILE *f = fopen("test", "w+");
  fwrite(buf.data(), buf.size(), 1, f);
  fclose(f);
  */

  MemoryReadBuffer rbuf((const char*)buf.data(), buf.size());
  Unpacker uk(&rbuf);

  cout << uk.get_uint() << endl;
  cout << uk.get_uint() << endl;
  cout << uk.get_double() << endl;

  Data d;

  while (uk.read_next(d) == 0) {
    switch (d.type) {
      case MSGPACK_T_UINT:
        cout << d.value.u << endl;
        break;
      case MSGPACK_T_INT:
        cout << d.value.i << endl;
        break;
      case MSGPACK_T_NIL:
        cout << "nil" << endl;
        break;
      case MSGPACK_T_TRUE:
        cout << "true" << endl;
        break;
      case MSGPACK_T_FALSE:
        cout << "false" << endl;
        break;
      case MSGPACK_T_ARRAY:
        cout << "array of size: " << d.value.len << endl;
        break;
      case MSGPACK_T_MAP:
        cout << "map of size: " << d.value.len << endl;
        break;
      case MSGPACK_T_RAW:
        cout << "raw of size: " << d.value.len << endl;
        break;
      case MSGPACK_T_FLOAT:
        cout << "float: " << d.value.f << endl;
        break;
      case MSGPACK_T_DOUBLE:
        cout << "double: " << d.value.d << endl;
        break;
    }
  }

  return 0;
}
