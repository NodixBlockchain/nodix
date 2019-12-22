More informations at

http://nodix.eu


Prerequisites:

install nasm

	apt-get install nasm

For building on 64 bits linux, you need gcc multilib to build and link 32 bit elf

	apt-get install gcc-multilib


Building instruction :


git clone https://github.com/NodixBlockchain/nodix


cd nodix


Libcon and launcher:

	make 


Modules: (optional)

	make modz

Running:

	cd export
	./launcher (use ~/nodix)
	(-gc to enable mark & sweep garbage collection)


No dependencies, all is included.

	nodix/export# ldd libcon.so 

        linux-gate.so.1 (0xf772d000)
        libm.so.6 => /lib/i386-linux-gnu/libm.so.6 (0xf7651000)
        libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xf749a000)
        /lib/ld-linux.so.2 (0x565ed000)


	nodix/export# ldd launcher

	linux-gate.so.1 (0xf77e6000)
	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xf760d000)
	libm.so.6 => /lib/i386-linux-gnu/libm.so.6 (0xf75b8000)
	libpthread.so.0 => /lib/i386-linux-gnu/libpthread.so.0 (0xf7599000)
	/lib/ld-linux.so.2 (0x5655e000)

Enjoy ! :-)
