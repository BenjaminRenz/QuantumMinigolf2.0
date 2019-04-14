#ifndef MAP_CAMERA_PLANE_INCLUDED
#define MAP_CAMERA_PLANE_INCLUDED

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif
#include "linmath.h"
/*CalibPoints are in the format
    D     C
    ┌─────┐
    │     │
    │     │
    └─────┘
    A     B
While the first and lower left points is A, then B... so
CalibPointsIn={Ax,Ay,Bx,By,...}
*/

typedef vec3 mat3x3[3];
static inline void mat3x3_identity(mat3x3 M) //Done
{
	int i, j;
	for(i=0; i<3; ++i)
		for(j=0; j<3; ++j)
			M[i][j] = i==j ? 1.f : 0.f;
}
static inline void mat3x3_dup(mat3x3 M, mat3x3 N)
{
	int i, j;
	for(i=0; i<3; ++i)
		for(j=0; j<3; ++j)
			M[i][j] = N[i][j];
}
static inline void mat3x3_mul(mat3x3 M, mat3x3 a, mat3x3 b) //Done
{
	mat3x3 temp;
	int k, r, c;
	for(c=0; c<3; ++c) for(r=0; r<3; ++r) {
		temp[c][r] = 0.f;
		for(k=0; k<3; ++k)
			temp[c][r] += a[k][r] * b[c][k];
	}
	mat3x3_dup(M, temp);
}
static inline void mat3x3_translate(mat3x3 T, float x, float y) //Done
{
	mat3x3_identity(T);
	T[2][0] = x;
	T[2][1] = y;
}
static inline void mat3x3_mul_vec3(vec3 r, mat3x3 M, vec3 v)
{
	int i, j;
	for(j=0; j<3; ++j) {
		r[j] = 0.f;
		for(i=0; i<3; ++i)
			r[j] += M[i][j] * v[i];
	}
}
static inline void mat3x3_rotate_delayed_Z(mat3x3 Q, mat3x3 M, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	mat3x3 R = {
		{   c,   s, 0.f},
		{  -s,   c, 0.f},
		{ 0.f, 0.f, 1.f}
	};
	mat3x3_mul(Q, R, M);
}

void camera_perspec_calibrating(mat3x3 CalibMatrixOut,float* CalibPointsIn);
void camera_perspec_map_point(vec2 MappedVecOut,mat3x3 CalibrationMatrixIn,vec2 CameraBrightSpotIn);

#endif // MAP_CAMERA_PLANE_INCLUDED
