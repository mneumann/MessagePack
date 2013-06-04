spec = Gem::Specification.new do |s|
  s.name = 'MessagePack'
  s.version = '0.3'
  s.summary = 'An alternative msgpack.org implementation for Ruby and C++'
  s.author = 'Michael Neumann'
  s.license = 'BSD License'
  s.files = ['MessagePack.gemspec',
             'include/MessagePack/Decoder.h',
	     'include/MessagePack/Encoder.h',
	     'include/MessagePack/Exception.h',
             'include/MessagePack/MacEndian.h',
	     'include/MessagePack/MessagePack.h',
             'include/MessagePack/Reader.h',
	     'include/MessagePack/ResizableBuffer.h',
             'include/MessagePack/Serialize.h',
	     'include/MessagePack/Writer.h',
             'lib/MessagePack.rb',
	     'ext/extconf.rb',
	     'ext/MessagePackExt.cc']
  s.extensions = ['ext/extconf.rb']
  s.require_paths = ['lib']
end
