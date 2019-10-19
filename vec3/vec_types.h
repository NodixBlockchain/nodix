#ifndef LIBVEC3_API
#define LIBVEC3_API C_IMPORT
#endif

#define PREC_MUL 256
#define PREC_SHIFT 8

#define PIf 3.1415926535897932384626433832795f
#define PId 3.1415926535897932384626433832795

typedef float		/*_CRT_ALIGN (16)*/ mat3x3f_t[16];
typedef double		mat3x3d_t[16];

typedef float		/*_CRT_ALIGN (16)*/ vec3f_t[4];
typedef double		vec3d_t[4];

typedef float		/*_CRT_ALIGN (16)*/ vec2f_t[2];
typedef double		vec2d_t[2];

typedef int			vec3i_t[4];
typedef int			*p_vec3i_t;
typedef const int	*cp_vec3i_t;
typedef int			 vec4i_t[4];
typedef int			*p_vec4i_t;



/*
#define ALIGN_STACK(n) unsigned int aligned_##n=((((unsigned int )(((&aligned_##n))+4))&0xFFFFFFF0)+16); float n[ aligned_##n-(unsigned long long )&aligned_##n)];
#define DECLARE_ALIGNED_VEC3(n) float _##n[8];float *n=(float *)((((unsigned long long )(_##n))&0xFFFFFFF0)+16)
#define DECLARE_ALIGNED_MAT3(n) float _##n[16];float *n=(float *)((((unsigned long long )(_##n))&0xFFFFFFF0)+16)
*/

#define DECLARE_ALIGNED_VEC3(n) _CRT_ALIGN (16) vec3f_t n
#define DECLARE_ALIGNED_MAT3(n) _CRT_ALIGN (16) mat3x3f_t n
#define DECLARE_ALIGNED_VEC3_ARRAY(n,x) float _##n[4*x+4];vec3f_t *n=(vec3f_t *)((((unsigned long long )(_##n))&0xFFFFFFF0)+16)

typedef struct {double x, y, z, w;} Quat; /* Quaternion */
enum QuatPart {X, Y, Z, W};
typedef float HMatrix[4][4]; /* Right-handed, for column vectors */
typedef Quat EulerAngles;    /* (x,y,z)=ang 1,2,3, w=order code  */



#define	EPSILON 0.000000001
#define D3_EPSILON 0.000000001f


typedef void	C_API_FUNC get_mat3x3gl_func(const mat3x3f_t m, float glMat[9]);
typedef void 	C_API_FUNC copy_mat3x3_func(mat3x3f_t m, const mat3x3f_t src);
typedef void 	C_API_FUNC copy_mat3x3d_func(mat3x3d_t m, const mat3x3d_t src);

typedef void 	C_API_FUNC mul_mat3x3_func(mat3x3f_t m, const mat3x3f_t op);
typedef void 	C_API_FUNC mul_mat3x3d_func(mat3x3d_t m, const mat3x3d_t op);

typedef void 	C_API_FUNC mul_mat3x3_o_func(const mat3x3f_t m, const mat3x3f_t op, mat3x3f_t out);
typedef void 	C_API_FUNC mul_mat3x3d_o_func(const mat3x3d_t m, const mat3x3d_t op, mat3x3d_t out);
typedef void 	C_API_FUNC mul_mat3x3f2d_func(mat3x3f_t m, const mat3x3d_t op);

typedef void	C_API_FUNC identity_mat3x3_func(mat3x3f_t m);
typedef float	C_API_FUNC determinant_mat3x3_func(const mat3x3f_t m);
typedef int		C_API_FUNC inverse_mat3x3_func(const mat3x3f_t m, mat3x3f_t out);

typedef void 	C_API_FUNC mul_mat3x3_rev_func(mat3x3f_t m, const mat3x3f_t op);
typedef void 	C_API_FUNC mul_mat3x3_rev_o_func(const mat3x3f_t m, const mat3x3f_t op, mat3x3f_t out);
typedef void 	C_API_FUNC mul_mat3x3_revf2d_func(mat3x3f_t m, const mat3x3d_t op);

typedef void 	C_API_FUNC mul_mat3x3_revd_func(mat3x3d_t m, const mat3x3d_t op);
typedef void 	C_API_FUNC mul_mat3x3_revd_o_func(const mat3x3d_t m, const mat3x3d_t op, mat3x3d_t out);

typedef void	C_API_FUNC identity_mat3x3d_func(mat3x3d_t m);
typedef double	C_API_FUNC determinant_mat3x3d_func(const mat3x3d_t m);
typedef int		C_API_FUNC inverse_mat3x3d_func(const mat3x3d_t m, mat3x3d_t out);

typedef 		copy_mat3x3d_func			*copy_mat3x3d_func_ptr;
typedef 		copy_mat3x3_func			*copy_mat3x3_func_ptr;

typedef 		mul_mat3x3_func				*mul_mat3x3_func_ptr;
typedef 		mul_mat3x3d_func			*mul_mat3x3d_func_ptr;
						 	
typedef 		mul_mat3x3_o_func			*mul_mat3x3_o_func_ptr;
typedef 		mul_mat3x3d_o_func			*mul_mat3x3d_o_func_ptr;
typedef 		mul_mat3x3f2d_func			*mul_mat3x3f2d_func_ptr;

typedef 		identity_mat3x3_func		*identity_mat3x3_func_ptr;
typedef 		determinant_mat3x3_func		*determinant_mat3x3_func_ptr;
typedef 		inverse_mat3x3_func			*inverse_mat3x3_func_ptr;
						 
typedef 		mul_mat3x3_rev_func			*mul_mat3x3_rev_func_ptr;
typedef 		mul_mat3x3_rev_o_func		*mul_mat3x3_rev_o_func_ptr;
typedef 		mul_mat3x3_revf2d_func		*mul_mat3x3_revf2d_func_ptr;
						 
typedef 		mul_mat3x3_revd_func		*mul_mat3x3_revd_func_ptr;
typedef 		mul_mat3x3_revd_o_func		*mul_mat3x3_revd_o_func_ptr;

typedef 		identity_mat3x3d_func		*identity_mat3x3d_func_ptr;
typedef 		determinant_mat3x3d_func	*determinant_mat3x3d_func_ptr;
typedef 		inverse_mat3x3d_func		*inverse_mat3x3d_func_ptr;

typedef 		get_mat3x3gl_func			*get_mat3x3gl_func_ptr;

EulerAngles Eul_(float ai, float aj, float ah, int order);

Quat		Eul_ToQuat					(EulerAngles ea);
void		Eul_ToHMatrix				(EulerAngles ea, HMatrix M);
EulerAngles Eul_FromHMatrix				(HMatrix M, int order);
EulerAngles Eul_FromQuat				(Quat q, int order);
LIBVEC3_API double  C_API_FUNC GetAngleAxis(double *hpb, double *v, unsigned int mod);

LIBVEC3_API void   C_API_FUNC  set_vec4i(int *vec, int a, int b, int c, int d);
LIBVEC3_API void   C_API_FUNC  set_vec3(vec3f_t vec, float a, float b, float c);

void   rotate_X_3f						(vec3f_t v,float aD);
void   rotate_Y_3f						(vec3f_t v,float aD);
void   rotate_Z_3f						(vec3f_t v,float aD);
LIBVEC3_API void  C_API_FUNC project_vec3(vec3f_t A, vec3f_t B, vec3f_t out);

LIBVEC3_API void   C_API_FUNC reflect_vec3(const vec3f_t v, const vec3f_t n, vec3f_t r);
LIBVEC3_API int	  C_API_FUNC  refract_vec3(const vec3f_t normal, const vec3f_t incident, double n1, double n2, vec3f_t out);

LIBVEC3_API void C_API_FUNC  copy_mat3x3(mat3x3f_t m, const mat3x3f_t op);
LIBVEC3_API void C_API_FUNC  copy_mat3x3d(mat3x3d_t m, const mat3x3d_t op);
LIBVEC3_API void C_API_FUNC  transpose_mat3x3(const mat3x3f_t m, mat3x3f_t out);
LIBVEC3_API void C_API_FUNC toOpenGL3x3(float *inMat, float *outMat);
LIBVEC3_API void C_API_FUNC 	 toOpenGL4x4(float *inMat, float *outMat);
LIBVEC3_API void	C_API_FUNC  create_rotation_mat(mat3x3f_t m, float angle, float aX, float aY, float aZ);

LIBVEC3_API void C_API_FUNC identity_mat3x3_c(mat3x3f_t m);
LIBVEC3_API int	 C_API_FUNC inverse_mat3x3_c (const mat3x3f_t m, mat3x3f_t out);
LIBVEC3_API void C_API_FUNC mat3x3_lookat	(const vec3f_t eyePosition, const vec3f_t lookAt, const vec3f_t upVector, mat3x3f_t  rotmat);


/*
LIBVEC3_API identity_mat3x3_func_ptr		identity_mat3x3;
LIBVEC3_API determinant_mat3x3_func_ptr		determinant_mat3x3;
LIBVEC3_API inverse_mat3x3_func_ptr			inverse_mat3x3;
LIBVEC3_API get_mat3x3gl_func_ptr			get_mat3x3gl;
LIBVEC3_API identity_mat3x3d_func_ptr		identity_mat3x3d;
LIBVEC3_API determinant_mat3x3d_func_ptr	determinant_mat3x3d;
LIBVEC3_API inverse_mat3x3d_func_ptr		inverse_mat3x3d;

LIBVEC3_API mul_mat3x3_func_ptr				mul_mat3x3;
LIBVEC3_API mul_mat3x3d_func_ptr			mul_mat3x3d;
									
LIBVEC3_API mul_mat3x3_o_func_ptr			mul_mat3x3_o;
LIBVEC3_API mul_mat3x3d_o_func_ptr			mul_mat3x3d_o;
LIBVEC3_API mul_mat3x3f2d_func_ptr			mul_mat3x3f2d;

LIBVEC3_API mul_mat3x3_rev_func_ptr			mul_mat3x3_rev;
LIBVEC3_API mul_mat3x3_rev_o_func_ptr		mul_mat3x3_rev_o;
LIBVEC3_API mul_mat3x3_revf2d_func_ptr		mul_mat3x3_revf2d;
									
LIBVEC3_API mul_mat3x3_revd_func_ptr		mul_mat3x3_revd;
LIBVEC3_API mul_mat3x3_revd_o_func_ptr		mul_mat3x3_revd_o;
*/

LIBVEC3_API void C_API_FUNC init_mat3_fncs_c();
LIBVEC3_API void C_API_FUNC init_mat3_fncs();



static __inline void zero_vec3			(vec3f_t v)
{
	_mm_store_ps(v,_mm_setzero_ps());
}
static __inline void copy_vec3			(vec3f_t v,const vec3f_t v2)
{
	_mm_store_ps(v,_mm_load_ps(v2));
}


static __inline void sub_vec3			(const vec3f_t v,const vec3f_t v2,vec3f_t out)
{
	if (v!=PTR_NULL)
		_mm_store_ps(out,_mm_sub_ps( _mm_load_ps(v),_mm_load_ps(v2)));
}

static __inline void mul_vec3(vec3f_t v,const vec3f_t v2)
{
	_mm_store_ps(v,_mm_mul_ps( _mm_load_ps(v),_mm_load_ps(v2)));
}



static __inline void mul_vec4( vec3f_t v,const vec3f_t v2)
{
	_mm_store_ps(v,_mm_mul_ps( _mm_load_ps(v),_mm_load_ps(v2)));
}


static __inline void add_vec3(const vec3f_t v,const vec3f_t v2,vec3f_t out)
{
	_mm_store_ps(out,_mm_add_ps( _mm_load_ps(v),_mm_load_ps(v2)));
}


static __inline void sub_vec4(const vec3f_t v,const vec3f_t v2,vec3f_t out)
{
	_mm_store_ps(out,_mm_sub_ps( _mm_load_ps(v),_mm_load_ps(v2)));
}


static __inline void scale_vec3(vec3f_t v,float d)
{
	__m128 _d;
	_d		= _mm_load_ss(&d);
	_mm_store_ps	(v,_mm_mul_ps(_mm_load_ps(v), _mm_shuffle_ps(_d,_d,0x00)));
}
static __inline void add_scale_vec3_o(const vec3f_t v, const vec3f_t v1, float d, vec3f_t o)
{
	__m128 _d;
	_d = _mm_load_ss(&d);
	_mm_store_ps(o, _mm_add_ps(_mm_load_ps(v), _mm_mul_ps(_mm_load_ps(v1), _mm_shuffle_ps(_d, _d, 0x00))));
}

static __inline void scale_vec3_o(const vec3f_t v,float d,vec3f_t out)
{
	__m128 _d;
	_d		= _mm_load_ss(&d);
	_mm_store_ps	(out,_mm_mul_ps(_mm_load_ps(v),_mm_shuffle_ps(_d,_d,0x00)));
}

static __inline void sub_scale_vec3_o(const vec3f_t v,const vec3f_t dr,float d,vec3f_t out)
{
	__m128 _d;
	_d		= _mm_load_ss(&d);
	_mm_store_ps	(out,_mm_sub_ps(_mm_load_ps(v),_mm_mul_ps(_mm_load_ps(dr),_mm_shuffle_ps(_d,_d,0x00))));

	/*
	out[0]		=	v[0]-dr[0]*d;
	out[1]		=	v[1]-dr[1]*d;
	out[2]		=	v[2]-dr[2]*d;
	*/

}
static __inline void interpolate_pilxels(int *pix1, unsigned char *pix2, long fac)
{
	pix1[0] = pix1[0]+(((pix2[0] - pix1[0])*fac) >> 8);
	pix1[1] = pix1[1]+(((pix2[1] - pix1[1])*fac) >> 8);
	pix1[2] = pix1[2]+(((pix2[2] - pix1[2])*fac) >> 8);
}

static __inline void cross_vec3(const vec3f_t v,const vec3f_t v2,vec3f_t out ) {
__m128 a ;
__m128 b;
__m128 ea , eb;
__m128 xa ;
__m128 xb ;
a=_mm_load_ps(v);
b=_mm_load_ps(v2);
// set to a[1][2][0][3] , b[2][0][1][3]
ea = _mm_shuffle_ps( a, a, _MM_SHUFFLE(3,0,2,1) );
eb = _mm_shuffle_ps( b, b, _MM_SHUFFLE(3,1,0,2) );
// multiply
xa = _mm_mul_ps( ea , eb );
// set to a[2][0][1][3] , b[1][2][0][3]
a = _mm_shuffle_ps( a, a, _MM_SHUFFLE(3,1,0,2) );
b = _mm_shuffle_ps( b, b, _MM_SHUFFLE(3,0,2,1) );
// multiply
xb = _mm_mul_ps( a , b );
// subtract
_mm_store_ps(out,_mm_sub_ps( xa , xb ));

/*
    Out[0] = (A[1]*B[2])-(B[1]*A[2]);
    Out[1] = (B[0]*A[2])-(A[0]*B[2]);
    Out[2] = (A[0]*B[1])-(A[1]*B[0]);
*/
}

static __inline float dot_vec3(const vec3f_t mv1, const vec3f_t mv2)
{
	//return (mv1[0] * mv2[0] + mv1[1] * mv2[1] + mv1[2] * mv2[2]);
	float sresult;

	__m128 result = _mm_mul_ps(_mm_load_ps(mv1), _mm_load_ps(mv2));
	__m128 part1 = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 3, 3, 2));
	__m128 part2 = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 3, 3, 1));

	result = _mm_add_ps(result, part1);
	result = _mm_add_ps(result, part2);

	_mm_store_ss(&sresult, result);
	return sresult;

}

static __inline float dot_self_vec3(const vec3f_t v)
{
	float sresult;
	__m128 result = _mm_mul_ps(_mm_load_ps(v), _mm_load_ps(v));
	__m128 part1 = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 3, 3, 2));
	__m128 part2 = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 3, 3, 1));
	result = _mm_add_ps(result, part1);
	result = _mm_add_ps(result, part2);
	_mm_store_ss(&sresult, result);
	return sresult;
}

static __inline void normalize_vec3(vec3f_t v)
{
	float d;
	__m128 scl;
	d	 = dot_self_vec3(v);
	scl  =_mm_rsqrt_ss(_mm_load_ss(&d));
	_mm_store_ps(v,_mm_mul_ps(_mm_load_ps(v),_mm_shuffle_ps(scl,scl,0x00)));
}

static __inline void normalize_vec3_o(const vec3f_t v,vec3f_t out)
{
	float d;
	__m128 scl;
	d	 = dot_self_vec3(v);
	scl  =_mm_rsqrt_ss(_mm_load_ss(&d));
	_mm_store_ps(out,_mm_mul_ps(_mm_load_ps(v),_mm_shuffle_ps(scl,scl,0x00)));
}

static __inline float sq_dist_vec3(const vec3f_t vec1,const vec3f_t vec2)
{
	vec3f_t  dv;
	sub_vec3(vec2,vec1,dv);
	return libc_fabsf(dot_self_vec3(dv));
}

static __inline float dist_vec3(const vec3f_t vec1,const vec3f_t vec2)
{
	vec3f_t  dv;
	float    sqrt;
	sub_vec3(vec2,vec1,dv);
	sqrtf_c(dot_self_vec3(dv), &sqrt);
	return sqrt;
}


static __inline float length_vec3(vec3f_t v)
{
	float    sqrt;
	sqrtf_c(dot_self_vec3(v), &sqrt);
	return sqrt;
}


static __inline void mul_vec3x3(vec3f_t v,const mat3x3f_t m)
{
	__m128 vec,mul;

	vec		= _mm_load_ps(v);
		
	mul		= _mm_mul_ps(_mm_shuffle_ps(vec,vec,0x00),_mm_load_ps(&m[0]));
	mul		= _mm_add_ps(mul,_mm_mul_ps (_mm_shuffle_ps(vec,vec,0x55),_mm_load_ps(&m[4])));
	mul		= _mm_add_ps(mul,_mm_mul_ps (_mm_shuffle_ps(vec,vec,0xAA),_mm_load_ps(&m[8])));

	_mm_store_ps(v,mul);
/*

	float	x,y,z;
	x		= v[0];
	y		= v[1];
	z		= v[2];

	v[0]=x*m[0]+y*m[4]+z*m[8];
	v[1]=x*m[1]+y*m[5]+z*m[9];
	v[2]=x*m[2]+y*m[6]+z*m[10];
*/
}


static __inline void mul_vec3x3_o(const vec3f_t v,const mat3x3f_t m,vec3f_t out)
{
	__m128 mul,vec;

	vec		= _mm_load_ps(v);

	mul		= _mm_mul_ps( _mm_shuffle_ps(vec,vec,0x00),_mm_load_ps(&m[0]));
	mul		= _mm_add_ps(mul,_mm_mul_ps (_mm_shuffle_ps(vec,vec,0x55),_mm_load_ps(&m[4])));
	mul		= _mm_add_ps(mul,_mm_mul_ps (_mm_shuffle_ps(vec,vec,0xAA),_mm_load_ps(&m[8])));

	_mm_store_ps(out,mul);
	/*
	float	x,y,z;
		
	x		= v[0];
	y		= v[1];
	z		= v[2];

	out[0]=x*m[0]+y*m[4]+z*m[8];
	out[1]=x*m[1]+y*m[5]+z*m[9];
	out[2]=x*m[2]+y*m[6]+z*m[10];
	*/
}



static __inline double dot_self_vec3d(const vec3d_t v)
{
	return (v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
}
static __inline double length_vec3d(vec3d_t v)
{
	double sqr;
	libc_sqrtd(dot_self_vec3d(v), &sqr);
	return sqr;
}


