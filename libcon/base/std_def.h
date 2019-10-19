
#ifndef __STD_DEF__
#define __STD_DEF__

#include "std_base.h"
	
#if defined(_M_X64) || defined(__amd64__)
	typedef int64_t			ptrdiff_t;
	typedef uint64_t		size_t;
	typedef int64_t			ctime_t;

	#define INVALID_SIZE	0xffffffffUL

	#define LONG_MAX		0x7FFFFFFFL
	#define LONG_MIN		((long) 0x80000000L)

	
	#define LONGLONG_MAX	0x8000000000000000
	#define LONGLONG_MIN    (-LONGLONG_MAX - 1)
	
	#define UINT_MAX	    0xffffffffUL
	#define ULONG_MAX	    0xffffffffUL
	#define CHAR_BIT	 8						/* number of bits in a char */
	#define USHRT_MAX    0xffff					/* maximum unsigned short value */
	#define SHRT_MIN    (-32768)				/* minimum (signed) short value */
	#define SHRT_MAX      32767					/* maximum (signed) short value */
	#define INT_MIN     (-2147483647 - 1)		/* minimum (signed) int value */
	#define INT_MAX       2147483647			/* maximum (signed) int value */
	#define offsetof(s,m)   (size_t)( (ptrdiff_t)&(((s *)0)->m) )

	#ifndef MAX_PATH 
		#define MAX_PATH 256
	#endif

#else
	
	typedef unsigned int	size_t;
	typedef int64_t			ctime_t;



	typedef int				ptrdiff_t;
	#define offsetof(s,m)   ((size_t)&(((s *)0)->m))


	#define INVALID_SIZE	0xffffffffUL

	#ifndef MAX_PATH 
		#define MAX_PATH 256
	#endif
	
	#ifndef UINT_MAX		
		#define LONG_MAX		0x7FFFFFFFL
		#define LONG_MIN		((long) 0x80000000L)
		#define LONGLONG_MAX	0x7FFFFFFFFFFFFFFFL
		#define LONGLONG_MIN	((long) 0x80000000L)


		#define UINT_MAX		0xffffffffUL
		#define ULONG64_MAX		0xFFFFFFFFFFFFFFFFUL
		#define ULONG64_MIN		0x00000000L

		#define ULONG_MAX		0xffffffffUL
		#define ULONG_MIN		0x00000000L
		#define CHAR_BIT		8						/* number of bits in a char */
		#define USHRT_MAX		0xffff					/* maximum unsigned short value */
		#define SHRT_MIN		(-32768)				/* minimum (signed) short value */
		#define SHRT_MAX		32767					/* maximum (signed) short value */
		#define INT_MIN			(-2147483647 - 1)		/* minimum (signed) int value */
		#define INT_MAX			2147483647			/* maximum (signed) int value */
	#endif
#endif

typedef union
{
	struct {
		uint64_t	val;
	}uint64;
	struct {
		unsigned int ints[2];
	}uint32;
}large_uint_t;


#endif

