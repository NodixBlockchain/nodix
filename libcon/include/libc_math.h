#ifndef LIBVEC3_API
#define LIBVEC3_API C_IMPORT
#endif


//#include <xmmintrin.h>

#define PIf 3.1415926535897932384626433832795f
#define PId 3.1415926535897932384626433832795

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define POSITIVE_INFINITY 3.402823466e+38F
#define NEGATIVE_INFINITY -POSITIVE_INFINITY 

typedef float float_func (float a);
typedef double double_func (double a);

typedef float_func  *float_func_ptr;
typedef double_func *double_func_ptr;


LIBC_API	 float  ASM_API_FUNC libc_atanf(float a);
LIBC_API	 float  ASM_API_FUNC libc_sinf(float a);
LIBC_API	 float  ASM_API_FUNC libc_cosf(float a);
LIBC_API	 float  ASM_API_FUNC libc_sqrtf(float a);
LIBC_API	 void   ASM_API_FUNC libc_ftol(float a, long *b);
LIBC_API	 unsigned char ASM_API_FUNC libc_ftouc(float a);
LIBC_API	 void	ASM_API_FUNC sqrtf_c(float a, float *sqr);

LIBC_API	 void ASM_API_FUNC libc_atand(double a, double *at);
LIBC_API	 void ASM_API_FUNC libc_sind(double a, double *s);
LIBC_API	 void ASM_API_FUNC libc_cosd(double a, double *c);
LIBC_API	 void ASM_API_FUNC libc_sqrtd(double a, double *sqr);




static __inline float libc_asinf(float A) {
	float sqrt;

	if (A > 1.0f || A < -1.0f)return 0.0f;
	if (A == 1.0f)   return PIf / 2.0f;
	if (A == -1.0f)return PIf / -2.0f;

	sqrtf_c(1.0f - A * A, &sqrt);


	return libc_atanf(A / sqrt);
}


static __inline float libc_acosf(float A) {
	float sqrt;
	if (A > 1.0f || A < -1.0f) return 0.0f;
	if (A == 0.0f)		   return PIf / 2.0f;

	sqrtf_c(1.0f - A * A, &sqrt);
	A = libc_atanf(sqrt / A);
	if (A < 0.0f)A += PIf;
	return A;
}


static __inline void libc_acosd(double A, double *B) {
	double sq;
	if (A > 1.0 || A < -1.0) { *B = 0.0; return; }
	if (A == 0.0) { *B = PId / 2.0; return; }
	libc_sqrtd(1.0 - A * A, &sq);
	libc_atand(sq / A, B);
	if ((*B) < 0.0)(*B) += PId;
}

static __inline void libc_asind(double A, double *B) {

	double sq;
	if (A > 1.0 || A < -1.0) { *B = 0.0; return; }
	if (A == 1.0) { *B = PId / 2.0; return; }
	if (A == -1.0) { *B = PId / -2.0; return; }
	libc_sqrtd(1.0f - A * A, &sq);
	libc_atand(A / sq, B);
}

static __inline float libc_fabsf(float a)
{	
	unsigned int t;
	t=*(((unsigned int *)(&a)))&0x7FFFFFFF;
	return *((float *)(&t));
}


static __inline int iabs_c(int i)
{
	return (i > 0) ? i : -i;
}

LIBC_API void ASM_API_FUNC powd_c(double a, double b, double * result);

LIBC_API void ASM_API_FUNC exp_c(double * result,double a);

LIBC_API void ASM_API_FUNC log2_c(double * result, double a);



LIBC_API void ASM_API_FUNC asm_pow_f_c(float a, float b, float* result);


LIBC_API double C_API_FUNC powf_c(double a, double b);
LIBC_API float C_API_FUNC  libc_powf(float a, float b);
