CONSRC=libcon/base/utf.c libcon/bintree.c libcon/base/string.c libcon/base/mem_base.c libcon/unix/stat_file.c libcon/unix/connect.c libcon/strs.c libcon/mem_stream.c libcon/tpo_mod.c libcon/exp.c libcon/zlibexp.c
XMLSRC=libcon/expat/xmlparse/xmlparse.c libcon/expat/xmltok/xmltok.c libcon/expat/xmltok/xmlrole.c
ZLIBSRC=libcon/zlib-1.2.8/zutil.c libcon/zlib-1.2.8/uncompr.c libcon/zlib-1.2.8/inftrees.c libcon/zlib-1.2.8/compress.c libcon/zlib-1.2.8/infback.c libcon/zlib-1.2.8/trees.c libcon/zlib-1.2.8/inflate.c libcon/zlib-1.2.8/crc32.c libcon/zlib-1.2.8/inffast.c libcon/zlib-1.2.8/adler32.c libcon/zlib-1.2.8/deflate.c libcon/minizip-master/ioapi.c libcon/minizip-master/ioapi_mem.c libcon/minizip-master/zip.c libcon/dozip.c

CFLAGS=-m32 -msse #-O2 -std=c99 -pedantic -D_DEFAULT_SOURCE
COMMON_INCS =  -Ilibcon -Ilibcon/include -Ilibbase/include -Ilibcon/zlib-1.2.8
MOD_LDFLAGS = -nodefaultlibs -nostdlib -Wl,--export-dynamic,-melf_i386

default: export/libcon.a export/launcher
	@echo 'done'

export/launcher: launcher/main.c export/libcon.a
	gcc $(CFLAGS) -lc -lm -pthread -Ilibcon  -Ilibcon/zlib-1.2.8/ -Ilibcon/include -Ilibbase/include launcher/main.c export/libcon.a -o export/launcher

export/libcon.a: $(CONSRC) $(XMLSRC) $(ZLIBSRC)
	nasm -f elf32 libcon/tpo.asm -o tpo.o
	nasm -f elf32 libcon/runtime.asm -o runtime.o
	gcc  $(CFLAGS)  $(COMMON_INCS) -Ilibcon/unix/include -Ilibcon/expat/xmlparse -Ilibcon/expat/xmltok $(CONSRC) $(XMLSRC) $(ZLIBSRC) -DNOCRYPT -c
	ld *.o -melf_i386 -soname libcon.so -shared -o export/libcon.so
	ar -cvq export/libcon.a *.o

export/libnodix.so:export/libbase.so export/libnode_adx.so export/libcon.so nodix/main.c
	gcc $(CFLAGS)  -Lexport -lcon -lblock_adx -lnode_adx -lbase $(COMMON_INCS)  nodix/main.c $(MOD_LDFLAGS),-soname,libnodix.so -shared -o export/libnodix.so

export/libstake_pos2.so:export/libbase.so export/libcon.so stake_pos2/kernel.c
	gcc $(CFLAGS)  -Lexport -lcon -lbase -lblock_adx $(COMMON_INCS) stake_pos2/kernel.c $(MOD_LDFLAGS),-soname,libstake_pos2.so -shared -o export/libstake_pos2.so

export/libstake_pos3.so:export/libbase.so export/libcon.so stake_pos3/kernel.c
	gcc $(CFLAGS)  -Lexport -lcon -lbase -lblock_adx $(COMMON_INCS) stake_pos3/kernel.c $(MOD_LDFLAGS),-soname,libstake_pos3.so -shared -o export/libstake_pos3.so

export/libecdsa.so:ecdsa/uECC.c export/libbase.so export/libcon.so 
	gcc $(CFLAGS)  -Lexport -lcon -lbase $(COMMON_INCS) ecdsa/uECC.c $(MOD_LDFLAGS),-soname,libecdsa.so -shared -o export/libecdsa.so

export/libblock_explorer.so:export/libblock_adx.so export/libbase.so export/libcon.so block_explorer/block_explorer.c
	gcc $(CFLAGS)  -Lexport -lcon -lbase -lblock_adx $(COMMON_INCS) block_explorer/block_explorer.c $(MOD_LDFLAGS),-soname,libstake_pos3.so -shared -o export/libblock_explorer.so

export/libblock_adx.so:block_adx/main.c block_adx/script.c block_adx/block.c block_adx/scrypt.c export/libprotocol_adx.so export/libbase.so export/libcon.so
	gcc $(CFLAGS)  -Lexport -lcon -lbase -lprotocol_adx $(COMMON_INCS) block_adx/main.c block_adx/block.c  block_adx/store.c block_adx/script.c block_adx/scrypt.c $(MOD_LDFLAGS),-soname,libblock_adx.so -shared -o export/libblock_adx.so

export/libprotocol_adx.so:protocol_adx/main.c protocol_adx/protocol.c export/libbase.so export/libcon.so
	gcc $(CFLAGS)  -Lexport -lcon -lbase $(COMMON_INCS) protocol_adx/main.c protocol_adx/protocol.c $(MOD_LDFLAGS),-soname,libprotocol_adx.so -shared -o export/libprotocol_adx.so

export/libwallet.so:wallet/wallet.c export/libblock_adx.so export/libbase.so export/libcon.so
	gcc $(CFLAGS)  -Lexport -lcon -lbase -lblock_adx $(COMMON_INCS) wallet/wallet.c $(MOD_LDFLAGS),-soname,libwallet.so -shared -o export/libwallet.so

export/librpc_wallet.so:rpc_wallet/rpc_methods.c export/libnode_adx.so export/libblock_adx.so export/libbase.so export/libcon.so
	gcc $(CFLAGS)  -Lexport -lcon -lbase -lblock_adx -lnode_adx $(COMMON_INCS) rpc_wallet/rpc_methods.c $(MOD_LDFLAGS),-soname,librpc_wallet.so -shared -o export/librpc_wallet.so

export/libvec3.so:vec3/mat3.c vec3/ray_float.c export/libbase.so export/libcon.so
	nasm -f elf32 vec3/box_f.asm -o vec3/box_f.o
	gcc $(CFLAGS)  -Lexport -lcon -lbase -lblock_adx $(COMMON_INCS) vec3/box_f.o vec3/mat3.c vec3/ray_float.c $(MOD_LDFLAGS),-soname,libvec3.so -shared -o export/libvec3.so

export/libnode_adx.so: node/node_impl.c node_adx/node_api.h
	gcc $(CFLAGS) -Lexport $(COMMON_INCS) -lcon  -lprotocol_adx -lblock_adx  -lbase node/node_impl.c -shared -o export/libnode_adx.so


export/libbase.so:libbaseimpl/funcs.c
	gcc $(CFLAGS) $(COMMON_INCS) libbaseimpl/funcs.c -shared -o export/libbase.so

modz:export/modz/vec3.tpo export/modz/protocol_adx.tpo export/modz/ecdsa.tpo export/modz/wallet.tpo export/modz/stake_pos3.tpo export/modz/nodix.tpo export/modz/rpc_wallet.tpo export/modz/block_explorer.tpo
	@echo "modz ok"

export/modz/block_explorer.tpo:export/mod_maker export/libblock_explorer.so
	export/mod_maker export/libblock_explorer.so ./export/modz
	mv export/modz/libblock_explorer.tpo export/modz/block_explorer.tpo

export/modz/ecdsa.tpo:export/mod_maker export/libecdsa.so
	export/mod_maker export/libecdsa.so ./export/modz
	mv export/modz/libecdsa.tpo export/modz/ecdsa.tpo

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
	gcc -m32 -Lexport  -lpthread $(COMMON_INCS) mod_maker/coff.c mod_maker/main.c mod_maker/elf.c export/libcon.a -o export/mod_maker
	
clean:
	rm -f export/libcon.a export/libcon.so export/launcher *.o

clean_mod:
	rm -f export/mod_maker export/libblock_explorer.so  export/libnodix.so export/libstake_pos3.so export/libstake_pos2.so export/modz/protocol_adx.tpo export/modz/block_adx.tpo export/modz/nodix.tpo export/modz/stake_pos3.tpo  export/modz/stake_pos2.tpo export/modz/block_explorer.tpo

