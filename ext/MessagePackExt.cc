#define USE_MSGPACK_EXTENSIONS

#include "MessagePack.h"
#include "ruby.h"
#include "ruby/st.h"
#include <assert.h>

struct hash_iter_ctx
{
  MessagePack::Packer *packer;
  int depth;
};

static int hash_iter(VALUE key, VALUE value, VALUE arg);

static void
recurse(MessagePack::Packer *t, VALUE obj, int depth)
{
  if (depth == 0)
    rb_raise(rb_eArgError, "nesting too deep");

  switch (TYPE(obj)) {
    case T_NIL:
      t->pack_nil();
      break;
    case T_TRUE:
      t->pack_true();
      break;
    case T_FALSE:
      t->pack_false();
      break;
    case T_FLOAT:
      t->pack_double(NUM2DBL(obj));
      break;
    case T_STRING:
      t->pack_raw(RSTRING_PTR(obj), RSTRING_LEN(obj));
      break;
    case T_SYMBOL:
      {
        // XXX: Can be optimized when no \0 is contained in the symbol
        VALUE str = rb_id2str(SYM2ID(obj));
        t->pack_raw(RSTRING_PTR(str), RSTRING_LEN(str));
      }
      break;
    case T_ARRAY:
      t->pack_array(RARRAY_LEN(obj));
      for (size_t i = 0; i < RARRAY_LEN(obj); ++i) {
        recurse(t, rb_ary_entry(obj, i), depth-1);
      }
      break;
    case T_HASH:
      hash_iter_ctx ctx;
      ctx.packer = t;
      ctx.depth = depth;
      t->pack_map(RHASH_SIZE(obj));
      rb_hash_foreach(obj, (int (*)(ANYARGS))hash_iter, (VALUE)&ctx);
      break;
    case T_FIXNUM:
      if (POSFIXABLE(obj))
      {
        t->pack_uint(FIX2ULONG(obj));
      }
      else
      {
        t->pack_int(FIX2LONG(obj));
      }
      break;
    case T_BIGNUM:
      if (RBIGNUM_POSITIVE_P(obj))
      {
        t->pack_uint(NUM2ULONG(obj));
      }
      else
      {
        t->pack_int(NUM2LONG(obj));
      }
      break;
    default:
      if (!rb_respond_to(obj, rb_intern("to_msgpack")))
        rb_raise(rb_eArgError, "Unsupported type (cannot msgpack object)");

      VALUE str = rb_funcall(obj, rb_intern("to_msgpack"), 0);
      Check_Type(str, T_STRING);
      t->get_buffer()->write(RSTRING_PTR(str), RSTRING_LEN(str));
  };
}

static int hash_iter(VALUE key, VALUE value, VALUE arg)
{
  hash_iter_ctx *ctx = (hash_iter_ctx*) arg;

  recurse(ctx->packer, key, ctx->depth-1);
  recurse(ctx->packer, value, ctx->depth-1);

  return ST_CONTINUE;
}

static VALUE
Packer_s__dump(VALUE self, VALUE obj, VALUE depth, VALUE init_buffer_sz)
{
  // depth == -1: infinitively
  MessagePack::Buffer buffer(FIX2INT(init_buffer_sz));
  MessagePack::Packer t(&buffer);
  recurse(&t, obj, FIX2INT(depth));
  return rb_str_new(t.get_buffer()->data(), t.get_buffer()->size());
}

VALUE unpack_value(MessagePack::Unpacker &uk, bool &success, bool *in_dynarray)
{
  using namespace MessagePack;
  Data d;

  success = true;
  if (uk.read_next(d) != 0)
  {
    success = false;
    return Qnil;
  }

  switch (d.type) {
    case MSGPACK_T_UINT:
      return ULONG2NUM(d.value.u);
    case MSGPACK_T_INT:
      return LONG2NUM(d.value.i);
    case MSGPACK_T_NIL:
      return Qnil;
    case MSGPACK_T_TRUE:
      return Qtrue;
    case MSGPACK_T_FALSE:
      return Qfalse;
    case MSGPACK_T_ARRAY:
      {
        VALUE ary = rb_ary_new2(d.value.len);
        for (size_t i=0; i < d.value.len; i++)
        {
          rb_ary_store(ary, i, unpack_value(uk, success, NULL));
          if (!success) return Qnil;
        }
        return ary;
      }
      break;
#ifdef USE_MSGPACK_EXTENSIONS
    case MSGPACK_T_ARRAY_BEG:
      {
        VALUE ary = rb_ary_new();
        bool exit_dynarray = false;
        while (true)
        {
          VALUE v = unpack_value(uk, success, &exit_dynarray);
          if (!success) return Qnil;
          if (exit_dynarray) return ary; 
          rb_ary_push(ary, v);
        }
      }
      break;
    case MSGPACK_T_ARRAY_END:
      {
          if (in_dynarray != NULL) {
              *in_dynarray = true; // notify the end of the dynarray
          } else {
              // invalid occurrence of MSGPACK_T_ARRAY_END
              success = false;
          }
          return Qnil;
      }
      break;
#endif
    case MSGPACK_T_MAP:
      {
        VALUE hash = rb_hash_new();
        VALUE key = Qnil;
        VALUE val = Qnil;
        for (size_t i=0; i < d.value.len; i++)
        {
          key = unpack_value(uk, success, NULL);
          if (!success) return Qnil;
          val = unpack_value(uk, success, NULL);
          if (!success) return Qnil;
          rb_hash_aset(hash, key, val);
        }
        return hash;
      }
    case MSGPACK_T_RAW:
      {
        VALUE str = rb_str_buf_new(d.value.len);
        rb_str_set_len(str, d.value.len);
        assert(RSTRING_LEN(str) == d.value.len);
        assert(uk.read_raw_body(RSTRING_PTR(str), RSTRING_LEN(str)));
        return str;
      }
    case MSGPACK_T_FLOAT:
      return DBL2NUM((double)d.value.f);
    case MSGPACK_T_DOUBLE:
      return DBL2NUM((double)d.value.d);
  }

  success = false;
  return Qnil;
}

static VALUE
Unpacker_s_each(VALUE self, VALUE str)
{
  Check_Type(str, T_STRING);

  MessagePack::ReadBuffer buffer(RSTRING_PTR(str), RSTRING_LEN(str));
  MessagePack::Unpacker uk(&buffer);

  bool success = true; 
  VALUE v = Qnil;

  while (!uk.buffer->at_end())
  {
    v = unpack_value(uk, success, NULL);
    if (!success)
    {
      return Qfalse;
    }
    rb_yield(v);
  }

  return Qtrue;
}

/*
 * Reads only first object from "stream"
 */
static VALUE
Unpacker_s_load(VALUE self, VALUE str)
{
  Check_Type(str, T_STRING);

  MessagePack::ReadBuffer buffer(RSTRING_PTR(str), RSTRING_LEN(str));
  MessagePack::Unpacker uk(&buffer);

  bool success; 
  VALUE v = unpack_value(uk, success, NULL);
  if (!success)
    rb_raise(rb_eArgError, "Invalid msgpack string");
  return v;
}

extern "C"
void Init_MessagePackExt()
{
  VALUE mMessagePack = rb_define_module("MessagePack");
  rb_define_module_function(mMessagePack, "_each", (VALUE (*)(...))Unpacker_s_each, 1);
  rb_define_module_function(mMessagePack, "load", (VALUE (*)(...))Unpacker_s_load, 1);
  rb_define_module_function(mMessagePack, "_dump", (VALUE (*)(...))Packer_s__dump, 3);
}
