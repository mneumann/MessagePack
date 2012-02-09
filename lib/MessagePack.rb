require 'MessagePackExt'

module MessagePack
  def dump(obj, depth=-1, buf_sz=32)
    _dump(obj, depth || -1, buf_sz)
  end
  module_function :dump

  def dump_to_file(obj, filename, depth=-1)
    _dump_to_file(obj, filename, depth || -1)
  end
  module_function :dump_to_file

  def each(str, &block)
    raise unless _each(str, &block)
  end
  module_function :each

  def self.include_path
    File.join(File.dirname(__FILE__), "../include")
  end
end
