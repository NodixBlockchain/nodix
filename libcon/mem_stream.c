/* copyright iadix 2016 */
#define LIBC_API C_EXPORT
#include "base/std_def.h"
#include "base/std_mem.h"
#include "base/mem_base.h"
#include "include/mem_stream.h"
#include "zlib.h"
unsigned int kernel_log_id;

/*typedef voidpf (*alloc_func) OF((voidpf opaque, uInt items, uInt size));*/
mem_ptr		zlib_alloc	(mem_ptr q,unsigned int n,unsigned int s)
{
	mem_zone_ref	pouet={PTR_NULL};

	allocate_new_zone	(0x0,n*s,&pouet);
	
	return get_zone_ptr(&pouet,0);
}

void zlib_free	(mem_ptr q,mem_ptr addr)
{
	mem_zone_ref	pouet;

	pouet.zone			=q;
	/* kernel_log			(kernel_log_id,"zlib free called \n"); */
	release_zone_ref	(&pouet);
}

OS_API_C_FUNC(void) mem_stream_init(mem_stream *stream,mem_zone_ref *stream_zone,unsigned int ofset)
{
	unsigned int sig;

	copy_zone_ref			(&stream->data,stream_zone);
	stream->current_ptr		=0;
	stream->buf_ofs			=ofset;
	sig						=mem_stream_peek_32	(stream);

	if(sig==0x504D435A)
	{
		unsigned int	decomp_size;
		unsigned int	comp_size;

		mem_stream_skip	(stream,4);
		decomp_size		=	mem_stream_read_32	(stream);
		comp_size		=	mem_stream_read_32	(stream);

		
		mem_stream_decomp	(stream,comp_size,decomp_size);

		stream->current_ptr	=	0;
	}

	stream->error = 0;
}

OS_API_C_FUNC(void) mem_stream_close(mem_stream *stream)
{
	release_zone_ref		(&stream->data);
	stream->current_ptr		=0;
}

OS_API_C_FUNC(size_t) mem_stream_get_pos(mem_stream *stream)
{
	return mem_sub(get_zone_ptr(&stream->data, stream->buf_ofs), uint_to_mem(stream->current_ptr));
}

/*
OS_API_C_FUNC(size_t)	mem_stream_write(mem_stream *stream,unsigned char *data,size_t len)
{
	if(realloc_zone (&stream->data,(stream->current_ptr+stream->buf_ofs+len))<0)return 0;
	memcpy_c(get_zone_ptr(&stream->data,stream->current_ptr+stream->buf_ofs),data,len);

	stream->current_ptr+=len;
	return len;
}


OS_API_C_FUNC(size_t)	mem_stream_write_8(mem_stream *stream, unsigned char data)
{
	if (realloc_zone(&stream->data, (stream->current_ptr + stream->buf_ofs + 1))<0)return 0;
	*((unsigned char *)(get_zone_ptr(&stream->data, stream->current_ptr + stream->buf_ofs))) = data;
	stream->current_ptr ++;
	return 1;
}
*/
OS_API_C_FUNC(size_t)	mem_stream_write_16(mem_stream *stream, unsigned short data)
{
	if (realloc_zone(&stream->data, (stream->current_ptr + stream->buf_ofs + 2))<0)return 0;
	*((unsigned short *)(get_zone_ptr(&stream->data, stream->current_ptr + stream->buf_ofs))) = data;
	stream->current_ptr+=2;
	return 1;
}
OS_API_C_FUNC(size_t)	mem_stream_write_32(mem_stream *stream, unsigned int data)
{
	if (realloc_zone(&stream->data, (stream->current_ptr + stream->buf_ofs + 4))<0)return 0;
	*((unsigned int *)(get_zone_ptr(&stream->data, stream->current_ptr + stream->buf_ofs))) = data;
	stream->current_ptr += 4;
	return 1;
}

OS_API_C_FUNC(size_t)	mem_stream_read		(mem_stream *stream,char *data,size_t len)
{
	mem_size		left;
	mem_size		to_read;
	mem_size		zone_size;

	zone_size	=	get_zone_size(&stream->data);
	if((stream->current_ptr+stream->buf_ofs)>=zone_size)
	{
		return 0;
	}


	left		=	zone_size-(stream->current_ptr+stream->buf_ofs);
	to_read		=   left>=len?len:left;
	memcpy_c		(data,get_zone_ptr(&stream->data,stream->current_ptr+stream->buf_ofs),to_read);
		
	stream->current_ptr+=to_read;
	return to_read;
}


OS_API_C_FUNC(size_t)	mem_stream_skip_to(mem_stream *stream,size_t position)
{	
	mem_size		zone_size;
	mem_size		new_pos;

	zone_size	=	get_zone_size(&stream->data);
	new_pos		=	((position+stream->buf_ofs) < zone_size) ? position : (zone_size-stream->buf_ofs); 

	stream->current_ptr	=	new_pos;

	return new_pos;
	
}

OS_API_C_FUNC(size_t)	mem_stream_skip(mem_stream *stream,unsigned int len)
{	
	mem_size		left;
	mem_size		to_skip;
	mem_size		zone_size;

	zone_size	=	get_zone_size(&stream->data);
	if((stream->current_ptr+stream->buf_ofs)>=zone_size)return 0;

	left		=	zone_size-(stream->current_ptr+stream->buf_ofs);
	to_skip		=   left>=len?len:left;

	stream->current_ptr	+=	to_skip;

	return to_skip;
	
}


OS_API_C_FUNC(int) mem_stream_decomp	(mem_stream *stream,unsigned int comp_size,unsigned int decomp_size)
{
	mem_zone_ref		decomp_data_ref;
	unsigned char		*decomp_data;
	unsigned char		*data;
	unsigned long		final_size;
	int					ret;
	z_stream			strm;

	decomp_data_ref.zone	=	PTR_NULL;
	allocate_new_zone	(0x0,decomp_size,&decomp_data_ref);

	decomp_data			=get_zone_ptr(&decomp_data_ref,0);
	
	data				=get_zone_ptr (&stream->data,stream->current_ptr+stream->buf_ofs);
	final_size			=decomp_size;

	/*
	kernel_log	(kernel_log_id,"inflating tpo mod !!");
	writeint	(comp_size,10);
	writestr	("=>");
	writeint	(final_size,10);
	writestr	("\n");
	*/


    strm.zalloc		= zlib_alloc;
    strm.zfree		= zlib_free;
    strm.opaque		= Z_NULL;
    
	strm.next_in	= data;
	strm.avail_in	= comp_size;
	strm.next_out	= decomp_data;
	strm.avail_out	= decomp_size;
	


	if(inflateInit_(&strm,ZLIB_VERSION, sizeof(z_stream))!=Z_OK)
	{
		return 0;
	}

	/* kernel_log(kernel_log_id,"inflating \n"); */

	if((ret=inflate		(&strm, Z_NO_FLUSH))!=Z_STREAM_END)
	{
		inflateEnd	(&strm);
		return 0;
	}



	inflateEnd			(&strm);


	copy_zone_ref		(&stream->data,&decomp_data_ref);
	release_zone_ref	(&decomp_data_ref);

	stream->buf_ofs		=	0;


	return 1;
}

OS_API_C_FUNC(unsigned int) mem_stream_read_32(mem_stream *stream)
{
	unsigned int res;
	if((stream->current_ptr+stream->buf_ofs+4)>=get_zone_size(&stream->data)) {
		stream->error = 1;
		return 0;
	}
	res=*(((unsigned int *)(get_zone_ptr(&stream->data,stream->current_ptr+stream->buf_ofs))));
	stream->current_ptr+=4;
	return res;
}


OS_API_C_FUNC(unsigned int) mem_stream_peek_32(mem_stream *stream)
{
	unsigned int res;
	if((stream->current_ptr+stream->buf_ofs+4)>=get_zone_size(&stream->data))return 0;
	res=*(((unsigned int *)(get_zone_ptr(&stream->data,stream->current_ptr+stream->buf_ofs))));
	return res;
}

OS_API_C_FUNC(unsigned short) mem_stream_read_16(mem_stream *stream)
{
	unsigned short res;
	if ((stream->current_ptr + stream->buf_ofs + 2) >= get_zone_size(&stream->data))
	{
		stream->error = 1;
		return 0;
	}
	res=*(((unsigned short *)(get_zone_ptr(&stream->data,stream->current_ptr+stream->buf_ofs))));
	stream->current_ptr+=2;
	return res;

}
/*
OS_API_C_FUNC(unsigned char) mem_stream_peek_8(mem_stream *stream)
{
	unsigned char res;
	if((stream->current_ptr+stream->buf_ofs+1)>=get_zone_size(&stream->data))return 0;
	res=*(((unsigned char *)(get_zone_ptr(&stream->data,stream->current_ptr+stream->buf_ofs))));
	return res;
}
*/
OS_API_C_FUNC(unsigned char) mem_stream_read_8(mem_stream *stream)
{
	unsigned char res;
	if((stream->current_ptr+stream->buf_ofs+1)>=get_zone_size(&stream->data))return 0;
	res=*(((unsigned char *)(get_zone_ptr(&stream->data,stream->current_ptr+stream->buf_ofs))));
	stream->current_ptr+=1;
	return res;
}
