#include "MessagePack/MessagePack.h"
#include "ruby.h"
#include "ruby/st.h"
#include <assert.h>
#include <stdio.h> /* fopen() */
#include <sys/types.h> /* fstat() */
#include <sys/stat.h> /* fstat() */
#include <unistd.h> /* fstat() */

static ID to_msgpack_obj;
static ID to_msgpack;
static VALUE mMessagePack;

struct recurse_state
{
  MessagePack::Encoder &encoder;
  int depth;

  recurse_state(MessagePack::Encoder &enc, int dep) : encoder(enc), depth(dep) {}

  recurse_state recurse() const { return recurse_state(encoder, depth-1); }
};

static int hash_iter(VALUE key, VALUE value, VALUE arg);

static void
recurse(const recurse_state &st, VALUE obj)
{
  if (st.depth == 0)
    rb_raise(rb_eArgError, "nesting too deep");

  switch (TYPE(obj)) {
    case T_NIL:
      st.encoder.emit_nil();
      break;
    case T_TRUE:
      st.encoder.emit_true();
      break;
    case T_FALSE:
      st.encoder.emit_false();
      break;
    case T_FLOAT:
      st.encoder.emit_double(NUM2DBL(obj));
      break;
    case T_STRING:
      st.encoder.emit_raw(RSTRING_PTR(obj), RSTRING_LEN(obj));
      break;
    case T_SYMBOL:
      {
        // XXX: Can be optimized when no \0 is contained in the symbol
        VALUE str = rb_id2str(SYM2ID(obj));
        st.encoder.emit_raw(RSTRING_PTR(str), RSTRING_LEN(str));
      }
      break;
    case T_ARRAY:
      st.encoder.emit_array(RARRAY_LEN(obj));
      for (size_t i = 0; i < RARRAY_LEN(obj); ++i) {
        recurse(st.recurse(), rb_ary_entry(obj, i));
      }
      break;
    case T_HASH:
      st.encoder.emit_map(RHASH_SIZE(obj));
      rb_hash_foreach(obj, (int (*)(ANYARGS))hash_iter, (VALUE)&st);
      break;
    case T_FIXNUM:
      if (POSFIXABLE(obj))
      {
        st.encoder.emit_uint(FIX2ULONG(obj));
      }
      else
      {
        st.encoder.emit_int(FIX2LONG(obj));
      }
      break;
    case T_BIGNUM:
      if (RBIGNUM_POSITIVE_P(obj))
      {
        st.encoder.emit_uint(NUM2ULONG(obj));
      }
      else
      {
        st.encoder.emit_int(NUM2LONG(obj));
      }
      break;
    default:

      if (rb_respond_to(obj, to_msgpack_obj))
      {
        //
        // Try first method #to_msgpack_obj, which returns an object
        //
        recurse(st.recurse(), rb_funcall(obj, to_msgpack_obj, 0));
      }
      else if (rb_respond_to(obj, to_msgpack))
      {
        //
        // Then try #to_msgpack, which returns a string in msgpack format
        //
        VALUE str = rb_funcall(obj, to_msgpack, 0);
        Check_Type(str, T_STRING);
        st.encoder.get_writer()->write(RSTRING_PTR(str), RSTRING_LEN(str));
      }
      else
      {
        rb_raise(rb_eArgError, "Unsupported type (cannot msgpack object)");
      }
  };
}

static int hash_iter(VALUE key, VALUE value, VALUE arg)
{
  recurse_state *stp = (recurse_state*) arg;

  recurse(stp->recurse(), key);
  recurse(stp->recurse(), value);

  return ST_CONTINUE;
}

static VALUE
Packer_s__dump(VALUE self, VALUE obj, VALUE depth, VALUE init_buffer_sz)
{
  try {
    // depth == -1: infinitively
    MessagePack::BufferedMemoryWriter writer(FIX2INT(init_buffer_sz));
    MessagePack::Encoder encoder(&writer);
    recurse(recurse_state(encoder, FIX2INT(depth)), obj);
    return rb_str_new((const char*)writer.data(), writer.size());
  }
  catch(MessagePack::Exception &e)
  {
    rb_raise(rb_eRuntimeError, "Exception: %s", e.msg);
  }
}

static VALUE
Packer_s__dump_to_file(VALUE self, VALUE obj, VALUE filename, VALUE depth)
{
  Check_Type(filename, T_STRING);

  // depth == -1: infinitively

  FILE *file = fopen(RSTRING_PTR(filename), "w+");
  if (file)
  {
    try {
      MessagePack::FileWriter writer(file);
      MessagePack::Encoder encoder(&writer);
      recurse(recurse_state(encoder, FIX2INT(depth)), obj);
    }
    catch(MessagePack::Exception &e)
    {
      fclose(file);
      rb_raise(rb_eRuntimeError, "Exception: %s", e.msg);
    }
    fclose(file);
  }
  else
  {
    rb_raise(rb_eArgError, "Failed to open file %s", RSTRING_PTR(filename));
  }

  return Qnil;
}

VALUE unpack_value(MessagePack::Decoder &dec, bool &success, bool *in_dynarray)
{
  using namespace MessagePack;
  DataValue value;

  success = true;

  switch (dec.read_next(value)) {
    case MSGPACK_T_UINT:
      return ULONG2NUM(value.u);
    case MSGPACK_T_INT:
      return LONG2NUM(value.i);
    case MSGPACK_T_NIL:
      return Qnil;
    case MSGPACK_T_BOOL:
      return (value.b ? Qtrue : Qfalse);
    case MSGPACK_T_ARRAY:
      {
        VALUE ary = rb_ary_new2(value.len);
        for (size_t i=0; i < value.len; i++)
        {
          rb_ary_store(ary, i, unpack_value(dec, success, NULL));
          if (!success) return Qnil;
        }
        return ary;
      }
      break;
    case MSGPACK_T_MAP:
      {
        VALUE hash = rb_hash_new();
        VALUE key = Qnil;
        VALUE val = Qnil;
        for (size_t i=0; i < value.len; i++)
        {
          key = unpack_value(dec, success, NULL);
          if (!success) return Qnil;
          val = unpack_value(dec, success, NULL);
          if (!success) return Qnil;
          rb_hash_aset(hash, key, val);
        }
        return hash;
      }
    case MSGPACK_T_RAW:
      {
        VALUE str = rb_str_buf_new(value.len);
        rb_str_set_len(str, value.len);
        assert(RSTRING_LEN(str) == value.len);
        dec.read_raw_body(RSTRING_PTR(str), RSTRING_LEN(str));
        return str;
      }
    case MSGPACK_T_FLOAT:
      return DBL2NUM((double)value.f);
    case MSGPACK_T_DOUBLE:
      return DBL2NUM((double)value.d);

    case MSGPACK_T_RESERVED:
      rb_raise(rb_eArgError, "Reserved data type");
      success = false;
      return Qnil;

    case MSGPACK_T_INVALID:
    default:
      rb_raise(rb_eArgError, "Invalid data type");
      success = false;
      return Qnil;
  }

  success = false;
  return Qnil;
}

static VALUE
unpack_each(MessagePack::Decoder &dec)
{
  bool success = true; 
  VALUE v = Qnil;

  while (!dec.get_reader()->at_end())
  {
    v = unpack_value(dec, success, NULL);
    if (!success)
    {
      return Qfalse;
    }
    rb_yield(v);
  }

  return Qtrue;
}

static VALUE
unpack_load(MessagePack::Decoder &dec)
{
  bool success = true;
  VALUE v = unpack_value(dec, success, NULL);
  if (!success)
    rb_raise(rb_eArgError, "Invalid msgpack string");
  return v;
}

static VALUE
Unpacker_s_each(VALUE self, VALUE str)
{
  Check_Type(str, T_STRING);
  try {
    MessagePack::MemoryReader reader(RSTRING_PTR(str), RSTRING_LEN(str));
    MessagePack::Decoder dec(&reader);
    return unpack_each(dec);
  }
  catch(MessagePack::Exception &e)
  {
    rb_raise(rb_eRuntimeError, "Exception: %s", e.msg);
  }
}

/*
 * Reads only first object from "stream"
 */
static VALUE
Unpacker_s_load(VALUE self, VALUE str)
{
  Check_Type(str, T_STRING);
  try {
    MessagePack::MemoryReader reader(RSTRING_PTR(str), RSTRING_LEN(str));
    MessagePack::Decoder dec(&reader);
    return unpack_load(dec);
  }
  catch(MessagePack::Exception &e)
  {
    rb_raise(rb_eRuntimeError, "Exception: %s", e.msg);
  }
}

static VALUE
Unpacker_s_load_from_file(VALUE self, VALUE filename)
{
  Check_Type(filename, T_STRING);

  try {
    MessagePack::FileReader reader(RSTRING_PTR(filename));
    MessagePack::Decoder dec(&reader);
    return unpack_load(dec);
  }
  catch(MessagePack::Exception &e)
  {
    rb_raise(rb_eRuntimeError, "Exception: %s", e.msg);
  }
}

extern "C"
void Init_MessagePackExt()
{
  to_msgpack_obj = rb_intern("to_msgpack_obj");
  to_msgpack = rb_intern("to_msgpack");

  mMessagePack = rb_define_module("MessagePack");
  rb_define_module_function(mMessagePack, "_each", (VALUE (*)(...))Unpacker_s_each, 1);
  rb_define_module_function(mMessagePack, "load", (VALUE (*)(...))Unpacker_s_load, 1);
  rb_define_module_function(mMessagePack, "load_from_file", (VALUE (*)(...))Unpacker_s_load_from_file, 1);
  rb_define_module_function(mMessagePack, "_dump", (VALUE (*)(...))Packer_s__dump, 3);
  rb_define_module_function(mMessagePack, "_dump_to_file", (VALUE (*)(...))Packer_s__dump_to_file, 3);
}
