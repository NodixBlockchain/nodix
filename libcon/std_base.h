#ifdef __GNUC__
#define C_EXPORT __attribute__ ((visibility ("default")))
#define C_IMPORT __attribute__ ((visibility ("default"))) 
#define struct_packed			__attribute__((packed))

#define _CRT_ALIGN(x) __attribute__((aligned(x)))

#if defined(_M_X64) || defined(__amd64__)

		#define KERN_API_FUNC		__attribute__((__stdcall))

		#define C_API_FUNC			
		#define ASM_API_FUNC		
		#define C_INT_FUNC			

		#define	MOD_NAME_DECO		GCC_STDCALL_64

		typedef long int				int64_t;
		typedef unsigned long int		uint64_t;


#else
		#define KERN_API_FUNC		__attribute__((__stdcall))

		#define C_API_FUNC			
		#define ASM_API_FUNC		
		#define C_INT_FUNC			

		#define	MOD_NAME_DECO		GCC_STDCALL_32
		typedef long long				int64_t;
		typedef	unsigned long long		uint64_t;

#endif

	
#endif

#ifdef _MSC_VER
	#define C_EXPORT		__declspec(dllexport)
	#define C_IMPORT		__declspec(dllimport)

	#if defined(_M_X64) || defined(__amd64__)

	#else
		#define C_API_FUNC			__stdcall
		#define ASM_API_FUNC		__cdecl
		#define C_INT_FUNC			__cdecl

		#define	MOD_NAME_DECO		MSVC_STDCALL_32
	#endif	

	#define struct_packed 
	typedef __int64					int64_t;
	typedef unsigned __int64		uint64_t;
#endif

typedef	void		   void_func();
typedef	void_func	   *void_func_ptr;

#ifndef MOD_NAME_DECO
#error compiler not supported !
#endif


#define OS_API_C_FUNC(return_type) C_EXPORT return_type C_API_FUNC 
#define OS_INT_C_FUNC(return_type) return_type C_INT_FUNC 
#define OS_API_XTRN_ASM_FUNC(return_type) extern return_type ASM_API_FUNC

