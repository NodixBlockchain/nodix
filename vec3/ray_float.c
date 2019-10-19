#define LIBVEC3_API C_EXPORT 

#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>
#include <strs.h>

#include "tree.h"
#include "libc_math.h"

#include <xmmintrin.h>
#include "vec_types.h"
#include "ray_float.h"


DECLARE_ALIGNED_VEC3(cube_norm[6]) = { 0xFF };

intersect_ray_boxf_func_ptr		intersect_ray_boxf=PTR_INVALID;
intersect_ray_boxesf_func_ptr	intersect_ray_boxesf = PTR_INVALID;


unsigned int NODE_HASH_center = 0xFFFFFFFF, NODE_HASH_imat = 0xFFFFFFFF, NODE_HASH_inmat = 0xFFFFFFFF, NODE_HASH_cubeMin = 0xFFFFFFFF, NODE_HASH_cubeMax = 0xFFFFFFFF, NODE_HASH_sqradius=0xFFFFFFFF;

extern int  ASM_API_FUNC intersect_ray_boxesf_sse(const vec3f_t ray_o, const vec3f_t ray_dir, unsigned int ray_sign, const box_testf_t *boxes, unsigned int n_boxes, unsigned int cast, unsigned int *bound_ids, float *min_max);
extern int  ASM_API_FUNC intersect_ray_boxf_sse(const vec3f_t r_origin, const vec3f_t r_inv_dir, float *parameters_1, float *out);

OS_API_C_FUNC(void)	get_cube_norm					(unsigned int which,vec3f_t out)
{
	copy_vec3(out,cube_norm[which]);
}

int intersect_ray_boxf_c	(const vec3f_t r_origin,const vec3f_t r_inv_dir,float *parameters,float *out)
{
  DECLARE_ALIGNED_VEC3(v_xy);
  float		v_z[2];

  sub_vec4	(parameters,r_origin,v_xy);
  mul_vec4	(v_xy,r_inv_dir);
    
  if ( (v_xy[0] > v_xy[3]) || (v_xy[1]>v_xy[2] ) ) 
    return 0;
  
  v_xy[0] = max(v_xy[0], v_xy[1]);
  v_xy[1] = min(v_xy[2], v_xy[3]);
  
  v_z[0]  = (parameters[4]-r_origin[4])*r_inv_dir[4];
  v_z[1]  = (parameters[5]-r_origin[5])*r_inv_dir[5];

  if ( (v_xy[0]  > v_z[1]) || ( v_z[0] > v_xy[1]) ) 
    return 0;

  out[0]= max(v_xy[0], v_z[0]);
  out[1]= min(v_xy[1], v_z[1]);
  return 1;

}

OS_API_C_FUNC(int) intersect_ray_boxesf_c(const vec3f_t ray_o,const vec3f_t ray_dir,unsigned int ray_sign,const box_testf_t *boxes,unsigned int n_boxes,unsigned int cast,unsigned int *bound_ids,float *min_max)
{
	unsigned int	intersect;
	unsigned int	n;
	float			c_minmax[4];

	intersect	=	0;
	min_max[0]		=	100000;
	min_max[1]		=	-100000;

	n			=	0;
	while(n<n_boxes)
	{
		if(cast!=boxes[n].id)
		{
			if(intersect_ray_boxf_c(ray_o,ray_dir,&boxes[n]._parameters[ray_sign],c_minmax))
			{
				if(c_minmax[0]<min_max[0])
					min_max[0]=c_minmax[0];

				if(c_minmax[1]>min_max[1])
					min_max[1]=c_minmax[1];
				intersect=1;
		
				(*bound_ids++)=boxes[n].id;
			}
		}
		n++;
	}
	(*bound_ids)=0;

	return intersect;

}


OS_API_C_FUNC(void) init_rayf(rayf_t *ray,vec3f_t o,vec3f_t d)
{

	  copy_vec3						(ray->origin,o);
	  copy_vec3						(ray->direction, d);

	  if (libc_fabsf(ray->direction[0]) < EPSILON)
		  ray->inv_direction[0] = 0;
	  else
		  ray->inv_direction[0]=1.0f/ray->direction[0];

	  if (libc_fabsf(ray->direction[1])< EPSILON)
		 ray->inv_direction[1] = 0;
	  else
		ray->inv_direction[1] =	1.0f/ray->direction[1];

	  if (libc_fabsf(ray->direction[2]) < EPSILON)
		  ray->inv_direction[2]		= 0;
	  else
		  ray->inv_direction[2]	=	1.0f/ray->direction[2];

	  ray->sign[0]					= 	(ray->inv_direction[0] < 0);
      ray->sign[1]					= 	(ray->inv_direction[1] < 0);
      ray->sign[2]					= 	(ray->inv_direction[2] < 0);

	  ray->f_origin[0]				=	ray->origin[0];
	  ray->f_origin[1]				=	ray->origin[1];
	  ray->f_origin[2]				=	ray->origin[0];
	  ray->f_origin[3]				=	ray->origin[1];
	  
	  ray->f_origin[4]				=	ray->origin[2];
	  ray->f_origin[5]				=	ray->origin[2];
	  ray->f_origin[6]				=	ray->origin[2];
	  ray->f_origin[7]				=	ray->origin[2];

	  ray->f_inv_direction[0]		=	ray->inv_direction[0];
	  ray->f_inv_direction[1]		=	ray->inv_direction[1];
	  ray->f_inv_direction[2]		=	ray->inv_direction[0];
	  ray->f_inv_direction[3]		=	ray->inv_direction[1];

	  ray->f_inv_direction[4]		=	ray->inv_direction[2];
	  ray->f_inv_direction[5]		=	ray->inv_direction[2];
	  ray->f_inv_direction[6]		=	ray->inv_direction[2];
	  ray->f_inv_direction[7]		=	ray->inv_direction[2];

	  ray->o_sign01					=	(ray->sign[0]|(ray->sign[1]<<1)|(ray->sign[2]<<2))*8;

}


int rayIntersectsTriangle(const float *p, const float *d,const float *v0, const float *v1, const float *v2,float *t) 
{

	float e1[4],e2[4],h[4],s[4],q[4];
	float a,f,u,v;
	
	sub_vec3(v1,v0,e1);
	sub_vec3(v2,v0,e2);

	cross_vec3(d,e2,h);
	a = dot_vec3(e1,h);

	if (a > -0.00001 && a < 0.00001)
		return(0);

	f = 1/a;
	sub_vec3(p,v0,s);
	u = f * (dot_vec3(s,h));

	if (u < 0.0 || u > 1.0)
		return(0);

	cross_vec3(s,e1,q);
	v = f * dot_vec3(d,q);

	if (v < 0.0 || u + v > 1.0)
		return(0);

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	*t = f * dot_vec3(e2,q);

	if (*t > 0.00001) // ray intersection
		return(1);

	else // this means that there is a line intersection
		 // but not a ray intersection
		 return (0);
}


int intersect_ray_box_soft (float *ray_origin,float *ray_inv_direction,float *min,float *max,float *t)
{
  float tmin, tmax, tymin, tymax, tzmin, tzmax;
    
  if(ray_inv_direction[0]>0)
  {
	tmin = ((min[0] - ray_origin[0]) * ray_inv_direction[0]);
	tmax = ((max[0] - ray_origin[0]) * ray_inv_direction[0]);
  }
  else
  {
	tmin = ((max[0]  - ray_origin[0]) * ray_inv_direction[0]);
	tmax = ((min[0]  - ray_origin[0]) * ray_inv_direction[0]);
  }

  if(ray_inv_direction[1]>0)
  {
	tymin = ((min[1] - ray_origin[1])   * ray_inv_direction[1]);
	tymax  =  ((max[1] - ray_origin[1]) * ray_inv_direction[1]);
  }
  else
  {
	tymin = ((max[1]  - ray_origin[1]) * ray_inv_direction[1]);
	tymax = ((min[1]  - ray_origin[1]) * ray_inv_direction[1]);
  }
  
 
  if ( (tmin > tymax) || (tymin > tmax) ) 
    return 0;

  if (tymin > tmin)
    tmin = tymin;

  if (tymax < tmax)
    tmax = tymax;

  if(ray_inv_direction[2]>0)
  {
	tzmin = ((min[2] - ray_origin[2]) * ray_inv_direction[2]);
	tzmax = ((max[2] - ray_origin[2]) * ray_inv_direction[2]);
  }
  else
  {
	tzmin = ((max[2]  - ray_origin[2]) * ray_inv_direction[2]);
	tzmax = ((min[2]  - ray_origin[2]) * ray_inv_direction[2]);
  }  

  if ( (tmin > tzmax) || (tzmin > tmax) ) 
    return 0;
  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;

  t[0]=tmin;
  t[1]=tmax;
  
  return 1;
}



OS_API_C_FUNC(int) rayIntersectCube(cubef_t *cube, const rayf_t *ray, float *out_dist, vec3f_t outPos)
{
	DECLARE_ALIGNED_VEC3	(raydir);
	DECLARE_ALIGNED_VEC3	(raypos);
	DECLARE_ALIGNED_VEC3	(rayori);
	DECLARE_ALIGNED_VEC3	(rayidir);
	float					dist;
	float					minmax[2];
	
	mul_vec3x3_o		(ray->direction	,cube->inv_norm_mat	,raydir);
	sub_vec3			(ray->origin	,cube->center		,rayori);
	mul_vec3x3_o		(rayori			,cube->inv_matrix	,raypos);
	
	rayidir[0]	= 1.0f/raydir[0];
	rayidir[1]	= 1.0f/raydir[1];
	rayidir[2]	= 1.0f/raydir[2];

	if (intersect_ray_box_soft(raypos, rayidir, cube->cubeMin, cube->cubeMax, minmax))
	{
		if(minmax[0]>0)
		{
			(*out_dist)=minmax[0];
			if(outPos!=NULL)
			{
				dist	= minmax[0];

				outPos[0]	= raypos[0]+raydir[0]*dist;
				outPos[1]	= raypos[1]+raydir[1]*dist;
				outPos[2]	= raypos[2]+raydir[2]*dist;
			}
			return 1;
		}
	}
	
	return 0;
}




 int intcyl(const vec3f_t raybase, const vec3f_t raydir, float radius, float sqradius, float *tVals)
 {
     DECLARE_ALIGNED_VEC3(n);
	 DECLARE_ALIGNED_VEC3(O);
     int	 hit; // True if ray intersects cyl
     float   d; // Shortest distance between the ray and the cylinder
     float	 t, s; // Distances along the ray

     float	 ln;

	 //RC = raybase.subtract3(base);
	 //n = raycos.cross3(axis);

	 n[0] = -raydir[2];
	 n[1] =  0.0;
     n[2] =  raydir[0];
     
     // Ray is parallel to the cylinder's axis.
     if ((ln = length_vec3(n)) < EPSILON)
     {
		 d	=	dot_vec3(raybase,raybase);
         
         tVals[0] = NEGATIVE_INFINITY;
         tVals[1] = POSITIVE_INFINITY;
         // True if ray is in cylinder.
		 return (d <= (sqradius));
     }
	 scale_vec3(n,1.0f/ln);

     d	 = libc_fabsf(dot_vec3(raybase,n)); // Shortest distance.
	 hit = (d <= radius);

     // If ray hits cylinder.
     if (hit)
     {
		 float sqr,dd;
		 O[0] = -raybase[2];
		 O[1] =  0.0;
		 O[2] =  raybase[0];
         t		  = -dot_vec3	(O,n) / ln;

		 O[0] = -n[2];
		 O[1] =  0.0;
		 O[2] =  n[0];

		// normalize_vec3	(O);
		 dd		  = dot_vec3	(raydir, O);
		 sqrtf_c	(sqradius - d * d,&sqr);
         s		  = libc_fabsf	(sqr/dd);
         tVals[0] = t - s; // Entering distance.
         tVals[1] = t + s; // Exiting distance.
     }

     return hit;
 }

 int clipcyl(const vec3f_t raybase, const vec3f_t raydir,float half_height, float *tVals)
{
  float dc, dwb, dwt, tb, tt;
  float in, out; // Object intersection distances.

  in  = tVals[0];
  out = tVals[1];

  dc  = raydir[1];
  dwb = raybase[1] + half_height;;
  dwt = raybase[1] - half_height;

  // Ray is parallel to the cylinder end-caps.
  if (dc == 0.0)
  {
      if (dwb <= 0.0)
          return 0;
      if (dwt >= 0.0)
          return 0;
  }
  else
  {
      // Intersect the ray with the bottom end-cap.
      tb = -dwb / dc;
      // Intersect the ray with the top end-cap.
      tt = -dwt / dc;

      // Bottom is near cap, top is far cap.
      if (dc >= 0.0)
      {
          if (tb > out)
              return 0;
          if (tt < in)
              return 0;
          if (tb > in && tb < out)
              in = tb;
          if (tt > in && tt < out)
              out = tt;
      }
      // Bottom is far cap, top is near cap.
      else
      {
          if (tb < in)
              return 0;
          if (tt > out)
              return 0;
          if (tb > in && tb < out)
              out = tb;
          if (tt > in && tt < out)
              in = tt;
      }
  }

  tVals[0] = in;
  tVals[1] = out;
  return (in < out);
 }

 
 OS_API_C_FUNC(int) intersectRayCyl(const rayf_t *ray, const cylf_t *cyl, float *tVals, vec3f_t outPos)
{
	DECLARE_ALIGNED_VEC3(raypos);
	DECLARE_ALIGNED_VEC3(raydir);

	sub_vec3	(ray->origin	,cyl->center		,raypos);

	mul_vec3x3	(raypos			,cyl->inv_matrix);
	mul_vec3x3_o(ray->direction	,cyl->inv_norm_mat	,raydir);
	

	if (!intcyl(raypos, raydir, cyl->radius, cyl->radiussq, tVals))
	{
        return 0;
	}

    if (!clipcyl(raypos, raydir, cyl->half_height, tVals))
	{
        return 0;
	}
	
	if(outPos!=NULL)
	{
		add_scale_vec3_o(raypos, raydir, tVals[0], outPos);
	}
	return 1;
}


OS_API_C_FUNC(int)	Sphere_intersect(const rayf_t *ray,/*mem_zone_ref_ptr sphere,*/spheref_t *sphere,float* t,float *t2,vec3f_t outPos)
{
	DECLARE_ALIGNED_VEC3(raypos);
	DECLARE_ALIGNED_VEC3(raydir);

	float a,b,c,disc;
	float distSqrt,q;
	float t0;
	float t1 ;
	/*
	float sq_radius;
	float *center;
	float *inv_matrix;
	float *inv_norm_mat;
	
	tree_manager_get_child_data_ptr(sphere, NODE_HASH_center, &center);
	tree_manager_get_child_data_ptr(sphere, NODE_HASH_imat, &inv_matrix);
	tree_manager_get_child_data_ptr(sphere, NODE_HASH_inmat, &inv_norm_mat);
	tree_manager_get_child_value_float(sphere, NODE_HASH_sqradius, &sq_radius);
	*/
	
	sub_vec3	(ray->origin	,sphere->center			,raypos);
	mul_vec3x3	(raypos			,sphere->inv_matrix);
	mul_vec3x3_o(ray->direction, sphere->inv_norm_mat, raydir);

	
    //Compute A, B and C coefficients
	a = dot_vec3(raydir, raydir);
    b = 2 * dot_vec3(raydir,raypos);
	c = dot_vec3(raypos, raypos) - sphere->sq_radius;

    //Find discriminant
    disc = b * b - 4 * a * c;
    
    // if discriminant is negative there are no real roots, so return 
    // false as ray misses sphere
    if (disc < 0)
        return 0;


    // compute q as described above
	sqrtf_c(disc,&distSqrt);



    if (b < 0)
        q = (-b - distSqrt)/(2.0f*a);
    else
        q = (-b + distSqrt)/(2.0f*a);

    // compute t0 and t1
    t0 = (q / a);
    t1 = (c / q);

    // make sure t0 is smaller than t1
    if (t0 > t1)
    {
        // if t0 is bigger than t1 swap them around
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // if t1 is less than zero, the object is in the ray's negative direction
    // and consequently the ray misses the sphere
    if (t1 < 0)
        return 0;
		
	
    // if t0 is less than zero, the intersection point is at t1
    if (t0 < 0)
    {
        *t = t1;
		if(t2)
			*t2=t0;
    }
    // else the intersection point is at t0
    else
    {
        *t = t0;

		if(t2)
			*t2=t1;
	}

	if(outPos!=NULL)
	{
		add_scale_vec3_o(raypos, raydir, (*t), outPos);
	}
	return 1;
}


OS_API_C_FUNC(void) init_vec_funcsf()
{
	intersect_ray_boxf = intersect_ray_boxf_sse;
	intersect_ray_boxesf = intersect_ray_boxesf_sse;
	
	set_vec3(cube_norm[0], 0, 0, 1);
	set_vec3(cube_norm[1], 1, 0, 0);
	set_vec3(cube_norm[2], 0, 0, -1);
	set_vec3(cube_norm[3], -1, 0, 0);
	set_vec3(cube_norm[4], 0, 1, 0);
	set_vec3(cube_norm[5], 0, -1, 0);

	NODE_HASH_center = NODE_HASH("center");
	NODE_HASH_imat = NODE_HASH("imat");
	NODE_HASH_inmat = NODE_HASH("inmat");
	NODE_HASH_cubeMin = NODE_HASH("cubeMin");
	NODE_HASH_cubeMax = NODE_HASH("cubeMax");
	NODE_HASH_sqradius = NODE_HASH("sqradius");
}

/*




float point_in_cyl_test_c( const float *pt1, const float *pt2, float lengthsq, float radius_sq, const float *testpt,float *normVec,float *uv_coord  )
{
	float dx, dy, dz;	// vector d  from line segment point 1 to point 2
	float pdx, pdy, pdz;	// vector pd from point 1 to test point
	float dot, dsq;

	dx = pt2[0] - pt1[0];	// translate so pt1 is origin.  Make vector from
	dy = pt2[1] - pt1[1];     // pt1 to pt2.  Need for this is easily eliminated
	dz = pt2[2] - pt1[2];

	pdx = testpt[0] - pt1[0];		// vector from pt1 to test point.
	pdy = testpt[1] - pt1[1];
	pdz = testpt[2] - pt1[2];

	// Dot the d and pd vectors to see if point lies behind the 
	// cylinder cap at pt1[0], pt1[1], pt1[2]

	dot = pdx * dx + pdy * dy + pdz * dz;

	// If dot is less than zero the point is behind the pt1 cap.
	// If greater than the cylinder axis line segment length squared
	// then the point is outside the other end cap at pt2.

	if( dot < 0.0f || dot > lengthsq )
	{
		return( -1.0f );
	}
	else 
	{
		// Point lies within the parallel caps, so find
		// distance squared from point to line, using the fact that sin^2 + cos^2 = 1
		// the dot = cos() * |d||pd|, and cross*cross = sin^2 * |d|^2 * |pd|^2
		// Carefull: '*' means mult for scalars and dotproduct for vectors
		// In short, where dist is pt distance to cyl axis: 
		// dist = sin( pd to d ) * |pd|
		// distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
		// dsq = ( 1 - (pd * d)^2 / (|pd|^2 * |d|^2) ) * |pd|^2
		// dsq = pd * pd - dot * dot / lengthsq
		//  where lengthsq is d*d or |d|^2 that is passed into this function 

		// distance squared to the cylinder axis:

		dsq = (pdx*pdx + pdy*pdy + pdz*pdz) - dot*dot/lengthsq;

		if( dsq > radius_sq )
		{
			return( -1.0f );
		}
		else
		{
			vec3f_t X,A,C,V,P;

			C[0]=(pt2[0]+pt1[0])/2.0f;
			C[1]=(pt2[1]+pt1[1])/2.0f;
			C[2]=(pt2[2]+pt1[2])/2.0f;

			copy_vec3	(X,testpt);
			sub_vec3	(pt2,pt1,A);
			normalize_vec3	(A);

			//V = X - C
			sub_vec3		(X,C,V);
			project_vec3	(V,A,P);

			sub_vec3	(V,P,normVec); // Project V onto A
			normalize_vec3(normVec);
			

			return( dsq );		// return distance squared to axis
		}
	}
}




int pointInSpheref(float *vec,spheref_t *sphere,float *normVec)
{
	float	l,sql;
	float	 distVec[3];


	sub_vec3(vec,sphere->center,distVec);
	l=dot_vec3(distVec,distVec);

	if(l>sphere->sq_radius)
		return 0;

	sql			=1.0f/libc_sqrtf(l);
	normVec[0]	=distVec[0]*sql;
	normVec[1]	=distVec[1]*sql;
	normVec[2]	=distVec[2]*sql;

	return 1;
}

void getCubeUvCoords(cubef_t *cube,int col,float *vec,float *uv_coord)
{
	float AP[3];
	float CP[3];
	float AB[3];
	float AC[3];
	float N[3];
	float v_num;
	float u_num;
	float denom;
	float ABxAP[3];
	float CAxCP[3];
	float plane_point[3];
	plane_point[0]	=	(vec[0]);
	plane_point[1]	=	(vec[1]);
	plane_point[2]	=	(vec[2]);
	
	AP[0]		=	plane_point[0]-cube->vecs[col][0][0];
	AP[1]		=	plane_point[1]-cube->vecs[col][0][1];
	AP[2]		=	plane_point[2]-cube->vecs[col][0][2];

	CP[0]		=	plane_point[0]-cube->vecs[col][3][0];
	CP[1]		=	plane_point[1]-cube->vecs[col][3][1];
	CP[2]		=	plane_point[2]-cube->vecs[col][3][2];

	AB[0]		=	(cube->vecs[col][1][0]-cube->vecs[col][0][0]);
	AB[1]		=	(cube->vecs[col][1][1]-cube->vecs[col][0][1]);
	AB[2]		=	(cube->vecs[col][1][2]-cube->vecs[col][0][2]);

	AC[0]		=	(cube->vecs[col][3][0]-cube->vecs[col][0][0]);
	AC[1]		=	(cube->vecs[col][3][1]-cube->vecs[col][0][1]);
	AC[2]		=	(cube->vecs[col][3][2]-cube->vecs[col][0][2]);


	cross_vec3(AB, AC,N);

	cross_vec3(AB, AP,ABxAP);
	v_num	=  dot_vec3(N, ABxAP);

	cross_vec3(CP, AC,CAxCP);
	u_num = dot_vec3(N, CAxCP);

	denom = dot_vec3(N, N); 

	uv_coord[0]		=	u_num/denom;
	uv_coord[1]		=	v_num/denom;
}
int pointInCubef(float *vec,cubef_t *cube,float *normVec,float *uv_coord)
{
	int		n;
	int		col;
	float	min_dist,dist;
	
	

	min_dist	=	1000000000;
	n			=	0;
	
	while(n<6)
	{
		dist=cube->planes[n].norm[0]*vec[0]+cube->planes[n].norm[1]*vec[1]+cube->planes[n].norm[2]*vec[2]+cube->planes[n].det;
		if(dist>0)
			return 0;

		dist=libc_fabsf(dist);

		if(dist<min_dist)
		{
			min_dist=dist;
			col     =n;
		}
		n++;
	}
	normVec[0]		=	cube->planes[col].norm[0];
	normVec[1]		=	cube->planes[col].norm[1];
	normVec[2]		=	cube->planes[col].norm[2];

	if(uv_coord!=NULL)
	{
		getCubeUvCoords(cube,col,vec,uv_coord);
		
	}
	return 1;
}*/



#if 0
int d3RayCylinderIntersection(const cylf_t *cylinder,const float *org,const float *dir,float *lambda,float *normal,float *newPosition)
// Ray and cylinder intersection
// If hit, returns true and the intersection point in 'newPosition' with a normal and distance along
// the ray ('lambda')
{
  vec3f_t RC;
  vec3f_t n,O,axis;
  vec3f_t HB;

  float d;
  float t,s;
  float ln;
  float in,out;
  float scale;


  sub_vec3		(org,cylinder->pt1,RC);
  sub_vec3		(cylinder->pt2,cylinder->pt1,axis);
  cross_vec3	(dir,axis,n);

  ln			=	length_vec3(n);

  
  // Parallel? (?)
  if((ln<D3_EPSILON)&&(ln>-D3_EPSILON))
    return 0;

  normalize_vec3(n);

  d=fabs(dot_vec3(RC,n));

  if (d<=cylinder->radius)
  {
    cross_vec3(RC,axis,O);
    //TVector::cross(RC,cylinder._Axis,O);
    t=-dot_vec3(O,n)/ln;
    //TVector::cross(n,cylinder._Axis,O);
    cross_vec3(n,axis,O);
    normalize_vec3(O);
	s=fabs( sqrtf(cylinder->radiussq-d*d) / dot_vec3(dir,O));

	in=t-s;
    out=t+s;

    if (in<-D3_EPSILON)
    {
      if(out<-D3_EPSILON)
        return 0;
      else *lambda=out;
    } else if(out<-D3_EPSILON)
    {
      *lambda=in;
    } else if(in<out)
    {
      *lambda=in;
    } else
    {
      *lambda=out;
    }

    // Calculate intersection point
    copy_vec3(newPosition,org);
    newPosition[0]+=dir[0]*(*lambda);
    newPosition[1]+=dir[1]*(*lambda);
    newPosition[2]+=dir[2]*(*lambda);


	sub_vec3(newPosition,cylinder->pt1,HB);

    scale=dot_vec3(HB,axis);
    normal[0]=HB[0]-axis[0]*scale;
    normal[1]=HB[1]-axis[1]*scale;
    normal[2]=HB[2]-axis[2]*scale;
    normalize_vec3(normal);
    return 1;
  }

  return 0;
}


int cyl_intersect(const cylf_t *cyl,const float *raypos ,const float *raydir,float* t)
{
	vec3f_t	SxD;
	vec3f_t	OC,CO;
	vec3f_t	D;
	vec3f_t	OCxD;
	float	a,b,c,d;
	float	t0,t1;
	float	tcylmin,tcylmax,tcapmin,tcapmax;
	float	ttop,tbot;
	float	l=1000.0f;
	float	distSqrt;

	sub_vec3	(raypos		,cyl->pt1,OC);
	sub_vec3	(cyl->pt1	,raypos	 ,CO);
	sub_vec3	(cyl->pt2	,cyl->pt1,D);
	cross_vec3	(raydir		,D,SxD);
	cross_vec3	(OC			,D,OCxD);

	a = dot_vec3(SxD,SxD);
	b = 2.0f * dot_vec3(OCxD,SxD);
	c = dot_vec3(OCxD,OCxD) - cyl->radiussq;

	d = (b*b - 4*a*c);

	if (d < 0)return 0;
	distSqrt = sqrtf(d);
	//the solutions are
	t0 = (-b-distSqrt) / (2.0f*a);
	t1 = (-b+distSqrt) / (2.0f*a);
	//and

	tcylmin = min(t0, t1);
	tcylmax = max(t0, t1);

	//now, the intersection points with the bottom and top caps. I assume the cylinder has the point C as the centre of the cylinder, and the cylinder is of length l.

	ttop	= (dot_vec3(CO,D)+l/2)/ dot_vec3(raydir,D);
	tbot	= (dot_vec3(CO,D)-l/2)/ dot_vec3(raydir,D);


	tcapmin = min(ttop, tbot);
	tcapmax = max(ttop, tbot);

	if ((tcapmin > tcylmax) || (tcapmax < tcylmin) )
		return 0;

	*t=min(tcylmin,tcapmin);

	return 1;

	//the ray intersects at the overlapping interval between [tcylmin, tcylmax] and [tcapmin, tcapmax].
}



// Intersect segment S(t)=sa+t(sb-sa), 0<=t<=1 against cylinder specified by p, q and r
int IntersectSegmentCylinder(const vec3f_t sa, const vec3f_t sb, const cylf_t *cyl, float *t)
{
vec3f_t d,m,n;
float	md,dd,nd,nn,mn,a,k,c,b,discr;

sub_vec3(cyl->pt2,cyl->pt1,d);
sub_vec3(sa,cyl->pt1,m);
sub_vec3(sb,sa,n);

//x²+ z²- r²= 0
//q=p+d*t;

md = dot_vec3(m, d);
nd = dot_vec3(n, d);
dd = dot_vec3(d, d);
// Test if segment fully outside either endcap of cylinder
if (md < 0.0f && md + nd < 0.0f) return 0; // Segment outside ’p’ side of cylinder
if (md > dd && md + nd > dd) return 0; // Segment outside ’q’ side of cylinder
nn = dot_vec3(n, n);
mn = dot_vec3(m, n);

a = dd * nn-nd * nd;
k = dot_vec3(m, m) - cyl->radiussq;
c = dd * k - md * md;
if (fabs(a) < D3_EPSILON) 
{
	// Segment runs parallel to cylinder axis
	if (c > 0.0f) 
		return 0; // ’a’ and thus the segment lie outside cylinder
	// Now known that segment intersects cylinder; figure out how it intersects
	
	if (md < 0.0f) 
		*t = -mn / nn; // Intersect segment against ’p’ endcap
	else if (md > dd) 
		*t = (nd - mn) / nn; // Intersect segment against ’q’ endcap
	else *t = 0.0f; // ’a’ lies inside cylinder
	
	return 1;
}
b = dd * mn - nd * md;
discr = b * b - a * c;
if (discr < 0.0f) return 0; // No real roots; no intersection198 Chapter 5 Basic Primitive Tests

*t = (-b - sqrtf(discr)) / (a);

if (*t < 0.0f || *t > 1.0f) return 0; // Intersection lies outside segment

if (md + *t * nd < 0.0f) 
{
	// Intersection outside cylinder on ’p’ side
	if (nd <= 0.0f) return 0; // Segment pointing away from endcap
	*t = -md / nd;
	// Keep intersection if dot_vec3(S(t) - p, S(t) - p) <= r^2
	return k + 2 * *t * (mn + *t * nn) <= 0.0f;

} 
else if (md + *t * nd > dd) 
{
	// Intersection outside cylinder on ’q’ side
	if (nd >= 0.0f) return 0; // Segment pointing away from endcap
	*t = (dd - md) / nd;
	// Keep intersection if dot_vec3(S(t) - q, S(t) - q) <= r^2
	return k + dd - 2 * md + *t * (2 * (mn - nd) + *t * nn) <= 0.0f;
}
// Segment intersects cylinder between the endcaps; t is correct

return 1;
}

 int intcyl(const vec3f_t raybase, const vec3f_t raydir, const vec3f_t base, float radius, float *tVals)
 {
     int	 hit; // True if ray intersects cyl
     vec3f_t RC; // Ray base to cylinder base
     float   d; // Shortest distance between the ray and the cylinder
     float	 t, s; // Distances along the ray
     vec3f_t n, D, O;
     float	 ln;

	 //RC = raybase.subtract3(base);
	 //n = raycos.cross3(axis);

	 sub_vec3	(raybase,base,RC);
	 cross_vec3	(raydir,axis,n);

     
     // Ray is parallel to the cylinder's axis.
     if ((ln = length_vec3(n)) == 0.0)
     {
		 vec3f_t	sc_axis;

		 d = dot_vec3	(RC,axis);
		 scale_vec3_o	(axis,d,sc_axis);
		 sub_vec3		(RC,sc_axis,D);
         d	=length_vec3(D);
         
         tVals[0] = NEGATIVE_INFINITY;
         tVals[1] = POSITIVE_INFINITY;
         // True if ray is in cylinder.
         return (d <= radius);
     }
	 normalize_vec3(n);
     //n = n.normalize3();
	 d	 = fabs(dot_vec3(RC,n)); // Shortest distance.
     hit = (d <= radius);

     // If ray hits cylinder.
     if (hit)
     {
         cross_vec3		(RC,axis,O);
         t		  = -dot_vec3	(O,n) / ln;
		 cross_vec3		(n,axis,O);
		 normalize_vec3	(O);
         
         s		  = fabs (sqrtf(radius * radius - d * d)/dot_vec3(raydir,O));
         tVals[0] = t - s; // Entering distance.
         tVals[1] = t + s; // Exiting distance.
     }

     return hit;
 }

 int clipcyl(const vec3f_t raybase, const vec3f_t raydir, const vec3f_t bot, const vec3f_t top, const vec3f_t axis, float *tVals)
{
  float dc, dwb, dwt, tb, tt;
  float in, out; // Object intersection distances.

  in  = tVals[0];
  out = tVals[1];

  dc  = dot_vec3(axis,raydir);
  dwb = dot_vec3(axis,raybase) - dot_vec3(axis,bot);
  dwt = dot_vec3(axis,raybase) - dot_vec3(axis,top);

  // Ray is parallel to the cylinder end-caps.
  if (dc == 0.0)
  {
      if (dwb <= 0.0)
          return 0;
      if (dwt >= 0.0)
          return 0;
  }
  else
  {
      // Intersect the ray with the bottom end-cap.
      tb = -dwb / dc;
      // Intersect the ray with the top end-cap.
      tt = -dwt / dc;

      // Bottom is near cap, top is far cap.
      if (dc >= 0.0)
      {
          if (tb > out)
              return 0;
          if (tt < in)
              return 0;
          if (tb > in && tb < out)
              in = tb;
          if (tt > in && tt < out)
              out = tt;
      }
      // Bottom is far cap, top is near cap.
      else
      {
          if (tb < in)
              return 0;
          if (tt > out)
              return 0;
          if (tb > in && tb < out)
              out = tb;
          if (tt > in && tt < out)
              in = tt;
      }
  }

  tVals[0] = in;
  tVals[1] = out;
  return (in < out);
 }
#endif
