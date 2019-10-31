CONSRC=libcon/bintree.c libcon/base/utf.c libcon/base/string.c libcon/base/mem_base.c libcon/unix/stat_file.c libcon/unix/connect.c libcon/strs.c libcon/mem_stream.c libcon/tpo_mod.c libcon/exp.c libcon/zlibexp.c
XMLSRC=libcon/expat/xmlparse/xmlparse.c libcon/expat/xmltok/xmltok.c libcon/expat/xmltok/xmlrole.c
ZLIBSRC=libcon/zlib-1.2.8/zutil.c libcon/zlib-1.2.8/uncompr.c libcon/zlib-1.2.8/inftrees.c libcon/zlib-1.2.8/compress.c libcon/zlib-1.2.8/infback.c libcon/zlib-1.2.8/trees.c libcon/zlib-1.2.8/inflate.c libcon/zlib-1.2.8/crc32.c libcon/zlib-1.2.8/inffast.c libcon/zlib-1.2.8/adler32.c libcon/zlib-1.2.8/deflate.c libcon/minizip-master/ioapi_mem.c libcon/minizip-master/zip.c libcon/dozip.c libcon/minizip-master/ioapi.c 
LIBBASESRC=libbaseimpl/funcs.c


CFLAGS = -m32 -msse  #-O2 -msse -pedantic -std=c89 -D_DEFAULT_SOURCE
COMMON_INCS = -Ilibcon -Ilibcon/include -Ilibbase/include -Ilibcon/zlib-1.2.8
MODFLAGS = -fvisibility=hidden -nostdlib -ffreestanding -fno-stack-protector -Wl,-melf_i386,--export-dynamic -shared
CC=gcc

default: export/libcon.a export/launcher
	@echo 'done'

export/launcher: launcher/main.c export/libcon.a
	$(CC) $(CFLAGS) -lc -lm -pthread $(COMMON_INCS) launcher/main.c export/libcon.a -Lexport/ -o export/launcher

export/libcon.a: $(CONSRC) $(XMLSRC) $(ZLIBSRC)
	nasm -f elf32 libcon/x86/tpo.asm -o tpo.o
	nasm -f elf32 libcon/x86/runtime.asm -o runtime.o
	$(CC) $(CFLAGS) $(COMMON_INCS) -Ilibcon/unix/include -Ilibcon/expat/xmlparse -Ilibcon/expat/xmltok $(CONSRC) $(XMLSRC) $(ZLIBSRC) -DNOCRYPT -c
	$(CC) *.o -Wl,--export-dynamic,-melf_i386 -m32 -lc -lm -lpthread -shared -o export/libcon.so
	ar -cvq export/libcon.a *.o

export/raytrace: raytrace/main.c export/libcon.a
	$(CC) $(CFLAGS) $(COMMON_INCS) raytrace/main.c export/libcon.a -lc -lm -pthread -o export/raytrace

export/libnodix.so:export/libbase.so export/libnode_adx.so export/libcon.so nodix/main.c
	$(CC) $(CFLAGS) $(COMMON_INCS) nodix/main.c -Inode_adx/ $(MODFLAGS) -Wl,-soname,libnodix.so  -Lexport -lcon -lbase -lblock_adx -lnode_adx -lwallet -o export/libnodix.so

export/libstake_pos2.so:export/libbase.so export/libcon.so stake_pos2/kernel.c
	$(CC) $(CFLAGS) $(COMMON_INCS) stake_pos2/kernel.c $(MODFLAGS) -Wl,-soname,libstake_pos2.so -Lexport -lcon -lbase -lblock_adx  -o export/libstake_pos2.so

export/libstake_pos3.so:export/libbase.so export/libcon.so stake_pos3/kernel.c
	$(CC) $(CFLAGS) $(COMMON_INCS) stake_pos3/kernel.c  $(MODFLAGS) -Wl,-soname,libstake_pos3.so -Lexport -lcon -lbase -lblock_adx -o export/libstake_pos3.so

export/libecdsa.so:ecdsa/uECC.c export/libbase.so export/libcon.so 
	$(CC) $(CFLAGS) $(COMMON_INCS) ecdsa/uECC.c $(MODFLAGS) -Wl,-soname,libecdsa.so -Lexport -lcon -lbase -o export/libecdsa.so

export/libblock_explorer.so:export/libblock_adx.so export/libbase.so export/libcon.so block_explorer/block_explorer.c
	$(CC) $(CFLAGS) $(COMMON_INCS) block_explorer/block_explorer.c  $(MODFLAGS) -Wl,-soname,libstake_pos3.so -Lexport -lcon -lbase -lblock_adx -o export/libblock_explorer.so

export/libblock_adx.so:block_adx/main.c block_adx/script.c block_adx/block.c block_adx/scrypt.c block_adx/store.c block_adx/app_store.c export/libprotocol_adx.so export/libbase.so export/libcon.so
	$(CC) $(CFLAGS) $(COMMON_INCS) block_adx/ripemd160.c block_adx/main.c block_adx/block.c block_adx/script.c block_adx/scrypt.c  block_adx/store.c block_adx/app_store.c $(MODFLAGS) -Wl,-soname,block_adx.so -Lexport -lcon -lbase -lprotocol_adx -o export/libblock_adx.so

export/libprotocol_adx.so:protocol_adx/main.c protocol_adx/protocol.c export/libbase.so export/libcon.so
	$(CC) $(CFLAGS) $(COMMON_INCS) protocol_adx/main.c protocol_adx/protocol.c $(MODFLAGS) -Wl,-soname,protocol_adx.so -Lexport -lcon -lbase -o export/libprotocol_adx.so

export/libnode_adx.so: node/node_impl.c node_adx/node_api.h
	$(CC) $(CFLAGS) $(COMMON_INCS) node/node_impl.c $(MODFLAGS) -Wl,-soname,node_adx.so -Lexport -lcon -lprotocol_adx -lblock_adx -o export/libnode_adx.so

export/libwallet.so:wallet/wallet.c export/libblock_adx.so export/libbase.so export/libcon.so
	$(CC) $(CFLAGS) $(COMMON_INCS) wallet/wallet.c  $(MODFLAGS) -Wl,-soname,libwallet.so -Lexport -lcon -lbase -lblock_adx -o export/libwallet.so

export/librpc_wallet.so:rpc_wallet/rpc_methods.c export/libnode_adx.so export/libblock_adx.so export/libbase.so export/libcon.so
	$(CC) $(CFLAGS) $(COMMON_INCS) rpc_wallet/rpc_methods.c $(MODFLAGS) -Wl,-soname,librpc_wallet.so  -Lexport -lcon -lbase -lblock_adx -lnode_adx  -o export/librpc_wallet.so

export/libvec3.so:vec3/mat3.c vec3/ray_float.c export/libbase.so export/libcon.so
	nasm -f elf32 vec3/box_f.asm -o vec3/box_f.o
	$(CC) $(CFLAGS) $(COMMON_INCS) vec3/box_f.o vec3/mat3.c vec3/ray_float.c $(MODFLAGS) -Wl,-soname,libvec3.so -Lexport -lcon -lbase -o export/libvec3.so

export/libbase.so:$(LIBBASESRC)
	$(CC) $(CFLAGS) $(COMMON_INCS) -Ilibcon/expat/xmlparse -Ilibcon/expat/xmltok  $(LIBBASESRC) $(MODFLAGS) -Wl,-soname,libbase.so -Lexport -lcon -o export/libbase.so

modz:export/modz/vec3.tpo export/modz/protocol_adx.tpo export/modz/ecdsa.tpo export/modz/block_adx.tpo export/modz/wallet.tpo export/modz/stake_pos3.tpo export/modz/nodix.tpo export/modz/rpc_wallet.tpo export/modz/block_explorer.tpo
	@echo "modz ok"

export/modz/block_explorer.tpo:export/mod_maker export/libblock_explorer.so
	export/mod_maker export/libblock_explorer.so ./export/modz
	mv export/modz/libblock_explorer.tpo export/modz/block_explorer.tpo

export/modz/ecdsa.tpo:export/mod_maker export/libecdsa.so
	export/mod_maker export/libecdsa.so ./export/modz
	mv export/modz/libecdsa.tpo export/modz/ecdsa.tpo

export/modz/vec3.tpo:export/mod_maker export/libvec3.so
	export/mod_maker export/libvec3.so ./export/modz
	mv export/modz/libvec3.tpo export/modz/vec3.tpo

export/modz/stake_pos2.tpo:export/mod_maker export/libstake_pos2.so
	export/mod_maker export/libstake_pos2.so ./export/modz
	mv export/modz/libstake_pos2.tpo export/modz/stake_pos2.tpo

export/modz/stake_pos3.tpo:export/mod_maker export/libstake_pos3.so
	export/mod_maker export/libstake_pos3.so ./export/modz
	mv export/modz/libstake_pos3.tpo export/modz/stake_pos3.tpo

export/modz/nodix.tpo:export/mod_maker export/libnodix.so
	export/mod_maker export/libnodix.so ./export/modz
	mv export/modz/libnodix.tpo export/modz/nodix.tpo

export/modz/block_adx.tpo:export/mod_maker export/libblock_adx.so
	export/mod_maker ./export/libblock_adx.so ./export/modz
	mv export/modz/libblock_adx.tpo export/modz/block_adx.tpo

export/modz/protocol_adx.tpo:export/mod_maker export/libprotocol_adx.so
	export/mod_maker ./export/libprotocol_adx.so ./export/modz
	mv export/modz/libprotocol_adx.tpo export/modz/protocol_adx.tpo

export/modz/wallet.tpo:export/mod_maker export/libwallet.so
	export/mod_maker ./export/libwallet.so ./export/modz
	mv export/modz/libwallet.tpo export/modz/wallet.tpo

export/modz/rpc_wallet.tpo:export/mod_maker export/librpc_wallet.so
	export/mod_maker ./export/librpc_wallet.so ./export/modz
	mv export/modz/librpc_wallet.tpo export/modz/rpc_wallet.tpo

export/mod_maker:  export/libcon.so
	$(CC) -m32 $(COMMON_INCS) mod_maker/coff.c mod_maker/main.c mod_maker/elf.c export/libcon.a -Lexport -lpthread -o export/mod_maker

clean:
	rm -f export/libcon.a export/libcon.so export/launcher *.o

clean_mod:
	rm -f export/mod_maker 
	rm -f export/libprotocol_adx.so export/libvec3.so export/libecdsa.so export/libblock_adx.so export/libstake_pos3.so export/libstake_pos2.so export/libnodix.so export/modz/libwallet.so export/libblock_explorer.so export/rpc_wallet.so
	rm -f export/modz/protocol_adx.tpo export/modz/vec3.tpo export/modz/ecdsa.tpo export/modz/block_adx.so export/modz/stake_pos3.tpo  export/modz/stake_pos2.tpo export/modz/nodix.tpo export/modz/wallet.tpo export/modz/block_explorer.tpo export/modz/rpc_wallet.tpo



