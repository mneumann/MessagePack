require 'mkmf'

raise unless find_header('MessagePack.h', "../include")
raise unless have_library('stdc++')
create_makefile('MessagePackExt')
