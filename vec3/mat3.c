//psikounov geometry analytique
#define LIBVEC3_API C_EXPORT
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>

#include <math.h>

#include "libc_math.h"

#include <xmmintrin.h>
#include "vec_types.h"

#ifndef _DEBUG
//int _fltused=1;
#endif

#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
    /* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>1;s=o&1;o>>1;\
    n=o&1;o>>1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
    /* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
    /* Static axes */
#define EulOrdXYZs    EulOrd(X,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(X,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(X,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(X,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(Y,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(Y,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(Y,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(Y,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(Z,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(Z,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(Z,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(Z,EulParOdd,EulRepYes,EulFrmS)
    /* Rotating axes */
#define EulOrdZYXr    EulOrd(X,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(X,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(X,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(X,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(Y,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(Y,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(Y,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(Y,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(Z,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(Z,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(Z,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(Z,EulParOdd,EulRepYes,EulFrmR)

#define EulHPB		  EulOrdYXZr
#define	RIGHT_HANDZ(z)		(-z)

//float matrixes
identity_mat3x3_func_ptr			identity_mat3x3=PTR_INVALID;
determinant_mat3x3_func_ptr			determinant_mat3x3 = PTR_INVALID;
inverse_mat3x3_func_ptr				inverse_mat3x3 = PTR_INVALID;
mul_mat3x3_func_ptr					mul_mat3x3 = PTR_INVALID;
mul_mat3x3_o_func_ptr				mul_mat3x3_o = PTR_INVALID;
mul_mat3x3_rev_func_ptr				mul_mat3x3_rev = PTR_INVALID;
mul_mat3x3_rev_o_func_ptr			mul_mat3x3_rev_o = PTR_INVALID;
get_mat3x3gl_func_ptr				get_mat3x3gl = PTR_INVALID;

//double matrixes
identity_mat3x3d_func_ptr			identity_mat3x3d = PTR_INVALID;
determinant_mat3x3d_func_ptr		determinant_mat3x3d = PTR_INVALID;
inverse_mat3x3d_func_ptr			inverse_mat3x3d = PTR_INVALID;
mul_mat3x3d_func_ptr				mul_mat3x3d = PTR_INVALID;
mul_mat3x3d_o_func_ptr				mul_mat3x3d_o = PTR_INVALID;
mul_mat3x3_revd_func_ptr			mul_mat3x3_revd = PTR_INVALID;
mul_mat3x3_revd_o_func_ptr			mul_mat3x3_revd_o = PTR_INVALID;
mul_mat3x3_revf2d_func_ptr			mul_mat3x3f2d = PTR_INVALID;
mul_mat3x3_revf2d_func_ptr			mul_mat3x3_revf2d = PTR_INVALID;



#define M(col,row) m[col*4+row]


static double RADperDEG = 3.1415926535897932384626433832795f/180.0;	

EulerAngles Eul_(float ai, float aj, float ah, int order)
{
    EulerAngles ea;
    ea.x = ai; ea.y = aj; ea.z = ah;
    ea.w = order;
    return (ea);
}

/* Construct quaternion from Euler angles (in radians). */
Quat Eul_ToQuat(EulerAngles ea)
{
    Quat qu;
    double a[3], ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd(ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {double t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) ea.y = -ea.y;
    ti = ea.x; tj = ea.y; th = ea.z;
	
	libc_cosd(ti, &ci);  libc_cosd(tj, &cj); libc_cosd(th, &ch);
	libc_sind(ti, &si);  libc_sind(tj, &sj); libc_sind(th, &sh);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	a[i] = cj*(cs + sc);	// Could speed up with 
	a[j] = sj*(cc + ss);	// trig identities. 
	a[k] = sj*(cs - sc);
	qu.w = cj*(cc - ss);
    } else {
	a[i] = cj*sc - sj*cs;
	a[j] = cj*ss + sj*cc;
	a[k] = cj*cs - sj*sc;
	qu.w = cj*cc + sj*ss;
    }
    if (n==EulParOdd) a[j] = -a[j];
    qu.x = a[X]; qu.y = a[Y]; qu.z = a[Z];
    return (qu);
}

#if 0
/* Construct matrix from Euler angles (in radians). */
void Eul_ToHMatrix(EulerAngles ea, HMatrix M)
{
    double ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd(ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = -ea.y; ea.z = -ea.z;}
    ti = ea.x;	  tj = ea.y;	th = ea.z;
    ci = libc_cosd(ti); cj = libc_cosd(tj); ch = libc_cosd(th);
    si = libc_sind(ti); sj = libc_sind(tj); sh = libc_sind(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	M[i][i] = cj;	  M[i][j] =  sj*si;    M[i][k] =  sj*ci;
	M[j][i] = sj*sh;  M[j][j] = -cj*ss+cc; M[j][k] = -cj*cs-sc;
	M[k][i] = -sj*ch; M[k][j] =  cj*sc+cs; M[k][k] =  cj*cc-ss;
    } else {
	M[i][i] = cj*ch; M[i][j] = sj*sc-cs; M[i][k] = sj*cc+ss;
	M[j][i] = cj*sh; M[j][j] = sj*ss+cc; M[j][k] = sj*cs-sc;
	M[k][i] = -sj;	 M[k][j] = cj*si;    M[k][k] = cj*ci;
    }
    M[W][X]=M[W][Y]=M[W][Z]=M[X][W]=M[Y][W]=M[Z][W]=0.0; M[W][W]=1.0;
}




/* Convert matrix to Euler angles (in radians). */
EulerAngles Eul_FromHMatrix(HMatrix M, int order)
{
    EulerAngles ea;
    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==EulRepYes) {
	double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);
	if (sy > 16*FLT_EPSILON) {
	    ea.x = atan2(M[i][j], M[i][k]);
	    ea.y = atan2(sy, M[i][i]);
	    ea.z = atan2(M[j][i], -M[k][i]);
	} else {
	    ea.x = atan2(-M[j][k], M[j][j]);
	    ea.y = atan2(sy, M[i][i]);
	    ea.z = 0;
	}
    } else {
	double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);
	if (cy > 16*FLT_EPSILON) {
	    ea.x = atan2(M[k][j], M[k][k]);
	    ea.y = atan2(-M[k][i], cy);
	    ea.z = atan2(M[j][i], M[i][i]);
	} else {
	    ea.x = atan2(-M[j][k], M[j][j]);
	    ea.y = atan2(-M[k][i], cy);
	    ea.z = 0;
	}
    }
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = - ea.y; ea.z = -ea.z;}
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    ea.w = order;
    return (ea);
}

/* Convert quaternion to Euler angles (in radians). */
EulerAngles Eul_FromQuat(Quat q, int order)
{
    HMatrix M;
    double Nq = q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w;
    double s = (Nq > 0.0) ? (2.0 / Nq) : 0.0;
    double xs = q.x*s,	  ys = q.y*s,	 zs = q.z*s;
    double wx = q.w*xs,	  wy = q.w*ys,	 wz = q.w*zs;
    double xx = q.x*xs,	  xy = q.x*ys,	 xz = q.x*zs;
    double yy = q.y*ys,	  yz = q.y*zs,	 zz = q.z*zs;
    M[X][X] = 1.0 - (yy + zz); M[X][Y] = xy - wz; M[X][Z] = xz + wy;
    M[Y][X] = xy + wz; M[Y][Y] = 1.0 - (xx + zz); M[Y][Z] = yz - wx;
    M[Z][X] = xz - wy; M[Z][Y] = yz + wx; M[Z][Z] = 1.0 - (xx + yy);
    M[W][X]=M[W][Y]=M[W][Z]=M[X][W]=M[Y][W]=M[Z][W]=0.0; M[W][W]=1.0;
    return (Eul_FromHMatrix(M, order));
}
#endif

/* Fill v[] with axis (from origin), return angle for quaternion style rotation */
OS_API_C_FUNC(double ) GetAngleAxis(double *hpb, double *v, unsigned int mod)
{
	Quat out;
	double	ang=0,s;
	EulerAngles inAngs;

	inAngs.x = hpb[0];
	inAngs.y = hpb[1];
	inAngs.z = hpb[2];
	
	if(mod==0)
		inAngs.w = EulOrdYXZs;
	else if(mod==1)
		inAngs.w = EulOrdYXZr;

	out = Eul_ToQuat(inAngs);
	
	libc_acosd	(out.w	, &ang);
	libc_sind	(ang	, &s);
	
	if(libc_fabsf(s)>EPSILON)		 // Is this valid???!!! 
	{
		v[0] = out.x/s;
		v[1] = out.y/s;
		v[2] = out.z/s;
	}
	else 
		v[0] = v[1] = v[2] = 0.0;
	
	return(ang);
}

OS_API_C_FUNC(void) copy_mat3x3(mat3x3f_t m, const mat3x3f_t op)
{
	int n=12;
	while(n--){m[n]=op[n];}
}


OS_API_C_FUNC(void) mat3x3_lookat( const vec3f_t eyePosition, const vec3f_t lookAt, const vec3f_t upVector,mat3x3f_t  rotmat)
{
	DECLARE_ALIGNED_VEC3(e);
	DECLARE_ALIGNED_VEC3(l);
	DECLARE_ALIGNED_VEC3(u);

	DECLARE_ALIGNED_VEC3(forward);
	DECLARE_ALIGNED_VEC3(side);
	DECLARE_ALIGNED_VEC3(up);

	
copy_vec3(e ,eyePosition);
copy_vec3(l , lookAt);
copy_vec3(u ,upVector);

// forward vector
sub_vec3(l,e,forward);
normalize_vec3(forward);

// side vector
cross_vec3		(u, forward, side);
normalize_vec3	(side);

cross_vec3		(forward, side, up);
normalize_vec3	(up);

rotmat[0] = side[0];
rotmat[4] = side[1];
rotmat[8] = side[2];

rotmat[1] = up[0];
rotmat[5] = up[1];
rotmat[9] = up[2];

rotmat[2]  = forward[0];
rotmat[6]  = forward[1];
rotmat[10] = forward[2];


}
OS_API_C_FUNC(float) determinant_mat3x3_c(const mat3x3f_t m)

{
	/*
	The inverse of a 3x3 matrix:

	| a00 a01 a02 |-1             |   a22a11-a21a12  -(a22a01-a21a02)   a12a01-a11a02  |
	| a10 a11 a12 |    =  1/DET * | -(a22a10-a20a12)   a22a00-a20a02  -(a12a00-a10a02) |
	| a20 a21 a22 |               |   a21a10-a20a11  -(a21a00-a20a01)   a11a00-a10a01  |

	with DET  =  a00(a22a11-a21a12)-a10(a22a01-a21a02)+a20(a12a01-a11a02)
	*/
	//return (M(0,0)*(M(2,2)*M(1,1)-M(2,1)*M(1,2))-M(1,0)*(M(2,2)*M(0,1)-M(2,1)*M(0,2))+M(2,0)*(M(1,2)*M(0,1)-M(1,1)*M(0,2)));
	return (M(0,0)*(M(2,2)*M(1,1)-M(2,1)*M(1,2))-M(1,0)*(M(2,2)*M(0,1)-M(2,1)*M(0,2))+M(2,0)*(M(1,2)*M(0,1)-M(1,1)*M(0,2)));
}




OS_API_C_FUNC(int) inverse_mat3x3_c(const mat3x3f_t m, mat3x3f_t out)
{
	float det;
	
	det		=	 determinant_mat3x3(m);
	if(det==0)
		return 0;

	det		=	1.0f/det;

	out[0]	=	 (M(1,1)*M(2,2)-M(2,1)*M(1,2))*det;
	out[1]	=	-(M(0,1)*M(2,2)-M(0,2)*M(2,1))*det;
	out[2]	=	 (M(0,1)*M(1,2)-M(0,2)*M(1,1))*det;

	out[4]	=	-(M(1,0)*M(2,2)-M(1,2)*M(2,0))*det;
	out[5]	=	 (M(0,0)*M(2,2)-M(0,2)*M(2,0))*det;
	out[6]	=	-(M(0,0)*M(1,2)-M(1,0)*M(0,2))*det;

	out[8]	=	 (M(1,0)*M(2,1)-M(2,0)*M(1,1))*det;
	out[9]	=	-(M(0,0)*M(2,1)-M(2,0)*M(0,1))*det;
	out[10]	=	 (M(0,0)*M(1,1)-M(1,0)*M(0,1))*det;

	return 1;

}

OS_API_C_FUNC(void) toOpenGL3x3(float *inMat, float *outMat)
{
	int i,j;
	for ( i=0; i<3; i++)
	{ 
		for ( j=0; j<3; j++)
		{
			outMat[3*i+j] = inMat[4*j+i];
		}
	}
}

OS_API_C_FUNC(void) toOpenGL4x4(float *inMat, float *outMat)
{
	int i,j;
	for ( i=0; i<3; i++)
	{ 
		for ( j=0; j<3; j++)
		{
			outMat[4*i+j] = inMat[4*j+i];
		}
		outMat[4 * i + 3] = 0.0f;
	}
	outMat[12 + 0] = 0.0f;
	outMat[12 + 1] = 0.0f;
	outMat[12 + 2] = 0.0f;
	outMat[12 + 3] = 1.0f;
}



OS_API_C_FUNC(void) transpose_mat3x3(const mat3x3f_t m,mat3x3f_t out)
{
	int			i,j;
	
	for ( i=0; i<3; i++)
	{ 
		for ( j=0; j<3; j++)
		{
			out[4*i+j] = m[4*j+i];
		}
	}
}

OS_API_C_FUNC(void) copy_mat3x3d(mat3x3d_t m, const mat3x3d_t op)
{
	int n=12;
	while(n--){m[n]=op[n];}
}

OS_API_C_FUNC(void) identity_mat3x3_c(mat3x3f_t m)
{

	M(0,0)	= M(1,1)  = M(2,2)	 = 1.0;
	M(0,1)	= M(0,2)  = M(1,0)	 =
	M(1,2)	= M(2,0)  = M(2,1)	 = 0.0;

}
OS_API_C_FUNC(void) get_mat3x3gl_c(const mat3x3f_t m, float glMat[9])
{
	glMat[0]=m[0];
	glMat[1]=m[1];
	glMat[2]=m[2];
	glMat[3]=m[4];
	glMat[4]=m[5];
	glMat[5]=m[6];
	glMat[6]=m[8];
	glMat[7]=m[9];
	glMat[8]=m[10];
}
OS_API_C_FUNC(void) mul_mat3x3_c(mat3x3f_t m,const mat3x3f_t op)
{
	float	m0,m1,m2;

	m0=M(0,0);
	m1=M(0,1);
	m2=M(0,2);

	M(0,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
	M(0,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
	M(0,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];

	m0=M(1,0);
	m1=M(1,1);
	m2=M(1,2);

	M(1,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
	M(1,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
	M(1,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];


	m0=M(2,0);
	m1=M(2,1);
	m2=M(2,2);

	M(2,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
	M(2,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
	M(2,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];
}



OS_API_C_FUNC(void) mul_mat3x3_rev_c(mat3x3f_t m, const mat3x3f_t op)
{
	float		op0,op1,op2;
	mat3x3f_t	temp;

	op0=op[0];
	op1=op[1];
	op2=op[2];

	temp[0] = op0 *M(0,0) + op1*M(1,0)  + op2 *M(2,0);
	temp[1] = op0 *M(0,1) + op1*M(1,1)  + op2 *M(2,1);
	temp[2] = op0 *M(0,2) + op1*M(1,2)  + op2 *M(2,2);

	op0=op[4];
	op1=op[5];
	op2=op[6];

	temp[4] = op0 *M(0,0) + op1*M(1,0)  + op2 *M(2,0);
	temp[5] = op0 *M(0,1) + op1*M(1,1)  + op2 *M(2,1);
	temp[6] = op0 *M(0,2) + op1*M(1,2)  + op2 *M(2,2);

	op0=op[8];
	op1=op[9];
	op2=op[10];

	temp[8]  = op0 *M(0,0) + op1*M(1,0)  + op2  *M(2,0);
	temp[9]  = op0 *M(0,1) + op1*M(1,1)  + op2  *M(2,1);
	temp[10] = op0 *M(0,2) + op1*M(1,2)  + op2  *M(2,2);

	copy_mat3x3(m,temp);
}


OS_API_C_FUNC(void) mul_mat3x3_rev_o_c(const mat3x3f_t m, const mat3x3f_t op, mat3x3f_t out)
{
	copy_mat3x3			(out,m);
	mul_mat3x3_rev_c	(out,op);

}
OS_API_C_FUNC(void) mul_mat3x3_o_c(const mat3x3f_t m, const mat3x3f_t op, mat3x3f_t out)
{
	copy_mat3x3			(out,m);
	mul_mat3x3_c		(out,op);
}
/*
The inverse of a 3x3 matrix:

| a00 a01 a02 |-1             |   a22a11-a21a12  -(a22a01-a21a02)   a12a01-a11a02  |
| a10 a11 a12 |    =  1/DET * | -(a22a10-a20a12)   a22a00-a20a02  -(a12a00-a10a02) |
| a20 a21 a22 |               |   a21a10-a20a11  -(a21a00-a20a01)   a11a00-a10a01  |

with DET  =  a00(a22a11-a21a12)-a10(a22a01-a21a02)+a20(a12a01-a11a02)

return (M(0,0)*(M(2,2)*M(1,1)-M(2,1)*M(1,2))-M(1,0)*(M(2,2)*M(0,1)-M(2,1)*M(0,2))+M(2,0)*(M(1,2)*M(0,1)-M(1,1)*M(0,2)));
*/

/*
OS_API_C_FUNC(double) determinant_mat3x3d_c(const mat3x3d_t m)
{
	return (M(0, 0)*(M(2, 2)*M(1, 1) - M(2, 1)*M(1, 2)) - M(1, 0)*(M(2, 2)*M(0, 1) - M(2, 1)*M(0, 2)) + M(2, 0)*(M(1, 2)*M(0, 1) - M(1, 1)*M(0, 2)));
}

OS_API_C_FUNC(int) inverse_mat3x3d_c(const mat3x3d_t m, mat3x3d_t out)
{
	double det;

	det = determinant_mat3x3d(m);
	if (det == 0)
		return 0;

	det = 1.0f / det;


	out[0] = (M(1, 1)*M(2, 2) - M(2, 1)*M(1, 2))*det;
	out[1] = -(M(0, 1)*M(2, 2) - M(0, 2)*M(2, 1))*det;
	out[2] = (M(0, 1)*M(1, 2) - M(0, 2)*M(1, 1))*det;

	out[4] = -(M(1, 0)*M(2, 2) - M(1, 2)*M(2, 0))*det;
	out[5] = (M(0, 0)*M(2, 2) - M(0, 2)*M(2, 0))*det;
	out[6] = -(M(0, 0)*M(1, 2) - M(1, 0)*M(0, 2))*det;

	out[8] = (M(1, 0)*M(2, 1) - M(2, 0)*M(1, 1))*det;
	out[9] = -(M(0, 0)*M(2, 1) - M(2, 0)*M(0, 1))*det;
	out[10] = (M(0, 0)*M(1, 1) - M(1, 0)*M(0, 1))*det;



	return 1;
}

OS_API_C_FUNC(void) identity_mat3x3d_c(mat3x3d_t m)
{
M(0,0)	= M(1,1)  = M(2,2)	 = 1.0;
M(0,1)	= M(0,2)  = M(1,0)	 =
M(1,2)	= M(2,0)  = M(2,1)	 = 0.0;
}

OS_API_C_FUNC(void) mul_mat3x3f2d_c(mat3x3f_t m, const mat3x3d_t op)
{
float	m0,m1,m2;

m0=M(0,0);
m1=M(0,1);
m2=M(0,2);

M(0,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
M(0,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
M(0,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];

m0=M(1,0);
m1=M(1,1);
m2=M(1,2);

M(1,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
M(1,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
M(1,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];


m0=M(2,0);
m1=M(2,1);
m2=M(2,2);

M(2,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
M(2,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
M(2,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];
}

OS_API_C_FUNC(void) mul_mat3x3d_c(mat3x3d_t m,const mat3x3d_t op)
{
	double	m0,m1,m2;

	m0=M(0,0);
	m1=M(0,1);
	m2=M(0,2);

	M(0,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
	M(0,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
	M(0,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];

	m0=M(1,0);
	m1=M(1,1);
	m2=M(1,2);

	M(1,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
	M(1,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
	M(1,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];


	m0=M(2,0);
	m1=M(2,1);
	m2=M(2,2);

	M(2,0) = m0 *op[0] + m1*op[4]  + m2 *op[8] ;
	M(2,1) = m0 *op[1] + m1*op[5]  + m2 *op[9] ;
	M(2,2) = m0 *op[2] + m1*op[6]  + m2 *op[10];
}
OS_API_C_FUNC(void) mul_mat3x3_revf2d_c(mat3x3f_t m, const mat3x3d_t op)
{
	float		op0,op1,op2;
	mat3x3f_t	temp;

	op0=op[0];
	op1=op[1];
	op2=op[2];

	temp[0] = op0 *M(0,0) + op1*M(1,0)  + op2 *M(2,0);
	temp[1] = op0 *M(0,1) + op1*M(1,1)  + op2 *M(2,1);
	temp[2] = op0 *M(0,2) + op1*M(1,2)  + op2 *M(2,2);

	op0=op[4];
	op1=op[5];
	op2=op[6];

	temp[4] = op0 *M(0,0) + op1*M(1,0)  + op2 *M(2,0);
	temp[5] = op0 *M(0,1) + op1*M(1,1)  + op2 *M(2,1);
	temp[6] = op0 *M(0,2) + op1*M(1,2)  + op2 *M(2,2);

	op0=op[8];
	op1=op[9];
	op2=op[10];

	temp[8]  = op0 *M(0,0) + op1*M(1,0)  + op2 *M(2,0);
	temp[9]  = op0 *M(0,1) + op1*M(1,1)  + op2 *M(2,1);
	temp[10] = op0 *M(0,2) + op1*M(1,2)  + op2 *M(2,2);

	copy_mat3x3(m,temp);
}

OS_API_C_FUNC(void) mul_mat3x3_revd_c(mat3x3d_t m, const mat3x3d_t op)
{
	double		op0,op1,op2;
	mat3x3d_t	temp;

	op0=op[0];
	op1=op[1];
	op2=op[2];

	temp[0] = op0 *M(0,0) + op1*M(1,0)  + op2 *M(2,0);
	temp[1] = op0 *M(0,1) + op1*M(1,1)  + op2 *M(2,1);
	temp[2] = op0 *M(0,2) + op1*M(1,2)  + op2 *M(2,2);

	op0=op[4];
	op1=op[5];
	op2=op[6];

	temp[4] = op0 *M(0,0) + op1*M(1,0)  + op2 *M(2,0);
	temp[5] = op0 *M(0,1) + op1*M(1,1)  + op2 *M(2,1);
	temp[6] = op0 *M(0,2) + op1*M(1,2)  + op2 *M(2,2);

	op0=op[8];
	op1=op[9];
	op2=op[10];

	temp[8]  = op0 *M(0,0) + op1*M(1,0)  + op2 *M(2,0);
	temp[9]  = op0 *M(0,1) + op1*M(1,1)  + op2 *M(2,1);
	temp[10] = op0 *M(0,2) + op1*M(1,2)  + op2 *M(2,2);

	copy_mat3x3d(m,temp);
}


OS_API_C_FUNC(void) mul_mat3x3_revd_o_c(const mat3x3d_t m, const mat3x3d_t op, mat3x3d_t out)
{
	copy_mat3x3d	(out,op);
	mul_mat3x3d		(out,m);
}

OS_API_C_FUNC(void) mul_mat3x3d_o_c(const mat3x3d_t m, const mat3x3d_t op, mat3x3d_t out)
{
	copy_mat3x3d	(out,m);
	mul_mat3x3d		(out,op);
}
*/







void rotate_X_3f(vec3f_t v,float aD){
	
	float a;
	float vnew[3];

	a=(aD*PIf)/180.0f;

	vnew[1] = libc_cosf(a) * v[1] - libc_sinf(a) * v[1];
	vnew[2] = libc_sinf(a) * v[2] + libc_cosf(a) * v[2];

	v[1]	= vnew[1];
	v[2]	= vnew[2];
    
}

void rotate_Y_3f(vec3f_t v,float aD){
	
	float a;
	float vnew[3];

	a=(aD*PIf)/180.0f;

	vnew[0] = libc_cosf(a) * v[0] - libc_sinf(a) * v[2];
	vnew[2] = libc_sinf(a) * v[0] + libc_cosf(a) * v[2];

	v[0]	= vnew[0];
	v[2]	= vnew[2];
    
} 

void rotate_Z_3f(vec3f_t v,float aD){
	
	float a;
	float vnew[3];

	a		=(aD*PIf)/180.0f;

	vnew[0] = libc_cosf(a) * v[0] - libc_sinf(a) * v[1];
	vnew[1] = libc_sinf(a) * v[0] + libc_cosf(a) * v[1];
	
	v[0]	= vnew[0];
	v[1]	= vnew[1];
   
} 


OS_API_C_FUNC(void) project_vec3(vec3f_t A, vec3f_t B, vec3f_t out)
 {
	float dot1,dot2;
	float coef;

	dot1=dot_vec3(A,B);
	dot2=dot_vec3(B,B);
	coef=dot1/dot2;
	out[0]=B[0]*(coef);
	out[1]=B[1]*(coef);
	out[2]=B[2]*(coef);

 }


OS_API_C_FUNC(void) set_vec4i			(int *vec,int a,int b,int c,int d)
{
	vec[0]=a;
	vec[1]=b;
	vec[2]=c;
	vec[3]=d;

}

OS_API_C_FUNC(void) set_vec3(vec3f_t vec, float a, float b, float c)
{
	vec[0]=a;
	vec[1]=b;
	vec[2]=c;
}





OS_API_C_FUNC(int) refract_vec3(const vec3f_t normal, const vec3f_t incident, double n1, double n2, vec3f_t out)
{
	/*
	const double n = n1 / n2;
	const double cosI = dot_vec3(normal, incident);
	const double sinT2 = n * n * (1.0 - cosI * cosI);
	if (sinT2 > 1.0)
	{
		return 0;
	}

	out[0]=n*incident[0]-(n+libc_sqrtd(1.0-sinT2))*normal[0];
	out[1]=n*incident[1]-(n+libc_sqrtd(1.0-sinT2))*normal[1];
	out[2]=n*incident[2]-(n+libc_sqrtd(1.0-sinT2))*normal[2];

	*/
	return 1;
}


OS_API_C_FUNC(void) reflect_vec3(const vec3f_t v,const vec3f_t n,vec3f_t r)
{
	float dot;

	dot	=	dot_vec3(v,n);

	r[0]=-(2*dot*n[0]-v[0]);
	r[1]=-(2*dot*n[1]-v[1]);
	r[2]=-(2*dot*n[2]-v[2]);


	//R = 2*(V dot N)*N - V
}







OS_API_C_FUNC(void) init_mat3_fncs_c()
{
mul_mat3x3			=	mul_mat3x3_c;
get_mat3x3gl		=	get_mat3x3gl_c;
mul_mat3x3_o		=	mul_mat3x3_o_c;
mul_mat3x3_rev		=	mul_mat3x3_rev_c;
mul_mat3x3_rev_o	=	mul_mat3x3_rev_o_c;
identity_mat3x3		=	identity_mat3x3_c;
determinant_mat3x3	=	determinant_mat3x3_c;
inverse_mat3x3		=	inverse_mat3x3_c;

/*
mul_mat3x3d			= mul_mat3x3d_c;
mul_mat3x3d_o		=	mul_mat3x3d_o_c;
mul_mat3x3_revd		=	mul_mat3x3_revd_c;
mul_mat3x3_revd_o	=	mul_mat3x3_revd_o_c;
mul_mat3x3f2d		=	mul_mat3x3f2d_c;
identity_mat3x3d	=	identity_mat3x3d_c;
determinant_mat3x3d	=	determinant_mat3x3d_c;
inverse_mat3x3d		=	inverse_mat3x3d_c;
mul_mat3x3_revf2d	=	mul_mat3x3_revf2d_c;
*/

}

OS_API_C_FUNC(void) create_rotation_mat(mat3x3f_t m,float angle, float aX, float aY, float aZ)
{
	float  x,y,z;
	float  xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
	int    optimized;

	x	=	aX;
	y	=	aY;
	z	=	aZ;

	s 	= libc_sinf(angle);
	c	= libc_cosf(angle);

	identity_mat3x3(m);
	optimized = 0;

	if (x == 0.0) {
		if (y == 0.0) {
			if (z != 0.0) {
				optimized = 1;
				/* rotate only around z-axis */
				M(0,0) = c;
				M(1,1) = c;
				if (z < 0.0) {
					M(0,1) = s;
					M(1,0) = -s;
				}
				else {
					M(0,1) = -s;
					M(1,0) = s;
				}
			}
		}
		else if (z == 0.0) {
			optimized = 1;
			/* rotate only around y-axis */
			M(0,0) = c;
			M(2,2) = c;
			if (y < 0.0) {
				M(0,2) = -s;
				M(2,0) = s;
			}
			else {
				M(0,2) = s;
				M(2,0) = -s;
			}
		}
	}
	else if (y == 0.0) {
		if (z == 0.0) {
			optimized = 1;
			/* rotate only around x-axis */
			M(1,1) = c;
			M(2,2) = c;
			if (x < 0.0) {
				M(1,2) = s;
				M(2,1) = -s;
			}
			else {
				M(1, 2) = -s;
				M(2, 1) = s;
				 }
			  }
		   }

	if (!optimized) {
		float mag;

		sqrtf_c(x * x + y * y + z * z, &mag);

		if (mag <= 1.0e-4) {
			/* no rotation, leave mat as-is */
			return;
		}

		x /= mag;
		y /= mag;
		z /= mag;


		/*
		*     Arbitrary axis rotation matrix.
		*
		*  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
		*  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
		*  (which is about the X-axis), and the two composite transforms
		*  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
		*  from the arbitrary axis to the X-axis then back.  They are
		*  all elementary rotations.
		*
		*  Rz' is a rotation about the Z-axis, to bring the axis vector
		*  into the x-z plane.  Then Ry' is applied, rotating about the
		*  Y-axis to bring the axis vector parallel with the X-axis.  The
		*  rotation about the X-axis is then performed.  Ry and Rz are
		*  simply the respective inverse transforms to bring the arbitrary
		*  axis back to it's original orientation.  The first transforms
		*  Rz' and Ry' are considered inverses, since the data from the
		*  arbitrary axis gives you info on how to get to it, not how
		*  to get away from it, and an inverse must be applied.
		*
		*  The basic calculation used is to recognize that the arbitrary
		*  axis vector (x, y, z), since it is of unit length, actually
		*  represents the sines and cosines of the angles to rotate the
		*  X-axis to the same orientation, with theta being the angle about
		*  Z and phi the angle about Y (in the order described above)
		*  as follows:
		*
		*  cos ( theta ) = x / sqrt ( 1 - z^2 )
		*  sin ( theta ) = y / sqrt ( 1 - z^2 )
		*
		*  cos ( phi ) = sqrt ( 1 - z^2 )
		*  sin ( phi ) = z
		*
		*  Note that cos ( phi ) can further be inserted to the above
		*  formulas:
		*
		*  cos ( theta ) = x / cos ( phi )
		*  sin ( theta ) = y / sin ( phi )
		*
		*  ...etc.  Because of those relations and the standard trigonometric
		*  relations, it is pssible to reduce the transforms down to what
		*  is used below.  It may be that any primary axis chosen will give the
		*  same results (modulo a sign convention) using thie method.
		*
		*  Particularly nice is to notice that all divisions that might
		*  have caused trouble when parallel to certain planes or
		*  axis go away with care paid to reducing the expressions.
		*  After checking, it does perform correctly under all cases, since
		*  in all the cases of division where the denominator would have
		*  been zero, the numerator would have been zero as well, giving
		*  the expected result.
		*/

		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * s;
		ys = y * s;
		zs = z * s;
		one_c = 1.0 - c;

		/* We already hold the identity-matrix so we can skip some statements */
		M(0, 0) = (one_c * xx) + c;
		M(0, 1) = (one_c * xy) - zs;
		M(0, 2) = (one_c * zx) + ys;


		M(1, 0) = (one_c * xy) + zs;
		M(1, 1) = (one_c * yy) + c;
		M(1, 2) = (one_c * yz) - xs;


		M(2, 0) = (one_c * zx) - ys;
		M(2, 1) = (one_c * yz) + xs;
		M(2, 2) = (one_c * zz) + c;

	}
#undef M
	}
#if 0
/***********************/
/*   Useful Routines   */
/***********************/

/* return roots of ax^2+bx+c */
/* stable algebra derived from Numerical Recipes by Press et al.*/
int quadraticRoots(a, b, c, roots)
double a, b, c, *roots;
{
double d, q;
int count = 0;
	d = (b*b)-(4*a*c);
	if (d < 0.0) { *roots = *(roots+1) = 0.0;  return(0); }
	q =  -0.5 * (b + (SGN(b)*sqrt(d)));
	if (a != 0.0)  { *roots++ = q/a; count++; }
	if (q != 0.0) { *roots++ = c/q; count++; }
	return(count);
	}


/* generic 1d regula-falsi step.  f is function to evaluate */
/* interval known to contain root is given in left, right */
/* returns new estimate */
double RegulaFalsi(f, left, right)
double (*f)(), left, right;
{
double d = (*f)(right) - (*f)(left);
	if (d != 0.0) return (right - (*f)(right)*(right-left)/d);
	return((left+right)/2.0);
	}

/* generic 1d Newton-Raphson step. f is function, df is derivative */
/* x is current best guess for root location. Returns new estimate */
double NewtonRaphson(f, df, x)
double (*f)(), (*df)(), x;
{
double d = (*df)(x);
	if (d != 0.0) return (x-((*f)(x)/d));
	return(x-1.0);
	}


/* hybrid 1d Newton-Raphson/Regula Falsi root finder. */
/* input function f and its derivative df, an interval */
/* left, right known to contain the root, and an error tolerance */
/* Based on Blinn */
double findroot(left, right, tolerance, f, df)
double left, right, tolerance;
double (*f)(), (*df)();
{
double newx = left;
	while (ABS((*f)(newx)) > tolerance) {
		newx = NewtonRaphson(f, df, newx);
		if (newx < left || newx > right) 
			newx = RegulaFalsi(f, left, right);
		if ((*f)(newx) * (*f)(left) <= 0.0) right = newx;  
			else left = newx;
		}
	return(newx);
	} 
#endif

