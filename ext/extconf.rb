require 'mkmf'

#$LDFLAGS << ' -Wl,-R/usr/local/lib'
raise unless find_header('MessagePack.h', "../include")
raise unless have_library('stdc++')
create_makefile('MessagePackExt')
