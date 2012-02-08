require 'mkmf'


#$CFLAGS.sub!(/-O\d/, "-O3")
$LDFLAGS << ' -Wl,-R/usr/local/lib'
raise unless find_header('MessagePack.h', ".")
raise unless have_library('stdc++')
create_makefile('MessagePackExt')
