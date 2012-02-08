require 'MessagePackExt'

module MessagePack
  def dump(obj, depth=-1, buf_sz=32)
    _dump(obj, depth || -1, buf_sz)
  end
  module_function :dump

  def each(str, &block)
    raise unless _each(str, &block)
  end
  module_function :each
end
