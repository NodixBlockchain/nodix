#define LIBC_API C_EXPORT
#include "../../base/std_base.h"
#include "../../base/std_def.h"
#include "../../base/std_mem.h"
#include "../../base/mem_base.h"
#include "../../base/std_str.h"

#include "include/strs.h"
#include "include/fsio.h"

#include "zlib.h"
#include "minizip-master/zip.h"
#include "minizip-master/ioapi_mem.h"

zlib_filefunc_def	my_def = { PTR_INVALID };
ourmemory_t			mem = { PTR_INVALID };

OS_API_C_FUNC(int) do_zip(const char *fileName, struct string *initial_data,const char **files, size_t nFiles,struct string *zipData)
{
	zipFile		myzip;
	
	mem.grow = 1;

	if (initial_data != PTR_NULL)
	{
		mem.base = initial_data->str;
		mem.cur_offset = 0;
		mem.size = initial_data->size;
		mem.limit = 0;
	}

	fill_memory_filefunc (&my_def, &mem);
	myzip		=	zipOpen2(fileName, APPEND_STATUS_ADDINZIP, PTR_NULL, &my_def);
	if (myzip == PTR_NULL)return 0;

	while (nFiles--)
	{
		mem_ptr		  data;
		size_t		  size;
		if (get_file(files[nFiles], &data, &size)>0)
		{
			zipOpenNewFileInZip (myzip, files[nFiles], PTR_NULL, PTR_NULL, 0, PTR_NULL, 0, "", Z_DEFLATED, 1);
			zipWriteInFileInZip	(myzip, data, size);
			zipCloseFileInZip	(myzip);
			free_c				(data);
		}
		else
		{
			char xx[16];

			itoa_s(nFiles, xx, 16, 10);
			log_output("unable to open zip file[");
			log_output(xx);
			log_output("] : '");
			log_output(files[nFiles]);
			log_output("'\n");
		}
	}
	zipClose(myzip, "Nodix");

	zipData->str = mem.base;
	zipData->len = mem.size;
	zipData->size = mem.limit;
	

	return 1;
}