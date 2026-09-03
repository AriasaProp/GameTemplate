/* *****************************************************************************
 * vector.h v0.0.0000
 * 
 * vector object
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _VEC_MATH_INCLUDED_
#define _VEC_MATH_INCLUDED_

#include "common.h"

typedef union {
	uint u[2];
	PACKED(struct) { uint x, y; };
} uvec2;
typedef union {
	int u[2];
	PACKED(struct) { int x, y; };
} ivec2;
typedef union {
	float v[2];
	struct { float x, y; };
} vec2;
typedef union {
	float v[3];
	struct { float x, y, z; };
	struct { vec2 xy; float __pad1; };
	struct { float __pad2; vec2 yz; };
} vec3;
typedef union {
	float v[4];
	struct { float x, y, z, w; };
	struct { vec2 xy, zw; };
	struct { float __pad1; vec2 yz; float __pad2; };
	struct { vec3 xyz; float __pad3; };
	struct {  float __pad4; vec3 yzw; };
} vec4;

#define IVEC2_ZERO    CLIT(ivec2){0, 0}
#define IVEC2_ONE     CLIT(ivec2){1, 1}
#define IVEC2_I(F)    CLIT(ivec2){.x = (F), .y = (F)}
#define IVEC2(X,Y)    CLIT(ivec2){.x = (X), .y = (Y)}

#define UVEC2_ZERO    CLIT(uvec2){0, 0}
#define UVEC2_ONE     CLIT(uvec2){1, 1}
#define UVEC2_I(F)    CLIT(uvec2){.x = (F), .y = (F)}
#define UVEC2(X,Y)    CLIT(uvec2){.x = (X), .y = (Y)}

#define VEC2_ZERO    CLIT(vec2){0.0f, 0.0f}
#define VEC2_ONE     CLIT(vec2){1.0f, 1.0f}
#define VEC2_F(F)    CLIT(vec2){ (F),  (F)}
#define VEC2(X,Y)    CLIT(vec2){ (X),  (Y)}

#define VEC3_ZERO    CLIT(vec3){0.0f, 0.0f, 0.0f}
#define VEC3_ONE     CLIT(vec3){1.0f, 1.0f, 1.0f}
#define VEC3_F(F)    CLIT(vec3){ (F),  (F),  (F)}
#define VEC3(X,Y,Z)  CLIT(vec3){ (X),  (Y),  (Z)}

#define VEC4_ZERO     CLIT(vec4){0.0f, 0.0f, 0.0f, 0.0f}
#define VEC4_ONE      CLIT(vec4){1.0f, 1.0f, 1.0f, 1.0f}
#define VEC4_F(F)     CLIT(vec4){ (F),  (F),  (F),  (F)}
#define VEC4(X,Y,Z,W) CLIT(vec4){ (X),  (Y),  (Z),  (W)}

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool  ivec2_is0  (const ivec2);
bool  ivec2_equal(const ivec2, const ivec2);
ivec2 ivec2_addi (const ivec2, const int);
ivec2 ivec2_subi (const ivec2, const int);
ivec2 ivec2_muli (const ivec2, const int);
ivec2 ivec2_divi (const ivec2, const int);
ivec2 ivec2_add  (const ivec2, const ivec2);
ivec2 ivec2_sub  (const ivec2, const ivec2);
ivec2 ivec2_mul  (const ivec2, const ivec2);
ivec2 ivec2_div  (const ivec2, const ivec2);
void  ivec2_maddi(ivec2*, const int);
void  ivec2_msubi(ivec2*, const int);
void  ivec2_mmuli(ivec2*, const int);
void  ivec2_mdivi(ivec2*, const int);
void  ivec2_madd (ivec2*, const ivec2);
void  ivec2_msub (ivec2*, const ivec2);
void  ivec2_mmul (ivec2*, const ivec2);
void  ivec2_mdiv (ivec2*, const ivec2);

bool  uvec2_is0  (const uvec2);
bool  uvec2_equal(const uvec2, const uvec2);
uvec2 uvec2_addu (const uvec2, const uint);
uvec2 uvec2_subu (const uvec2, const uint);
uvec2 uvec2_mulu (const uvec2, const uint);
uvec2 uvec2_divu (const uvec2, const uint);
uvec2 uvec2_add  (const uvec2, const uvec2);
uvec2 uvec2_sub  (const uvec2, const uvec2);
uvec2 uvec2_mul  (const uvec2, const uvec2);
uvec2 uvec2_div  (const uvec2, const uvec2);
void  uvec2_maddu(uvec2*, const uint);
void  uvec2_msubu(uvec2*, const uint);
void  uvec2_mmulu(uvec2*, const uint);
void  uvec2_mdivu(uvec2*, const uint);
void  uvec2_madd (uvec2*, const uvec2);
void  uvec2_msub (uvec2*, const uvec2);
void  uvec2_mmul (uvec2*, const uvec2);
void  uvec2_mdiv (uvec2*, const uvec2);

bool  vec2_is0   (const vec2);
bool  vec2_equal (const vec2, const vec2);
vec2  vec2_addf  (const vec2, const float);
vec2  vec2_subf  (const vec2, const float);
vec2  vec2_mulf  (const vec2, const float);
vec2  vec2_divf  (const vec2, const float);
vec2  vec2_add   (const vec2, const vec2);
vec2  vec2_sub   (const vec2, const vec2);
vec2  vec2_mul   (const vec2, const vec2);
vec2  vec2_div   (const vec2, const vec2);
vec2  vec2_diff  (const vec2, const vec2);
vec2  vec2_norm  (const vec2);
vec2  vec2_floor (const vec2);
float vec2_dist  (const vec2, const vec2);
float vec2_dot   (const vec2, const vec2);
float vec2_rad   (const vec2, const vec2);
void  vec2_maddf (vec2*, const float);
void  vec2_msubf (vec2*, const float);
void  vec2_mmulf (vec2*, const float);
void  vec2_mdivf (vec2*, const float);
void  vec2_madd  (vec2*, const vec2);
void  vec2_msub  (vec2*, const vec2);
void  vec2_mmul  (vec2*, const vec2);
void  vec2_mdiv  (vec2*, const vec2);
void  vec2_mdiff (vec2*, const vec2);
void  vec2_mnorm (vec2*);
void  vec2_mfloor(vec2*);

bool  vec3_is0   (const vec3);
bool  vec3_equal (const vec3, const vec3);
vec3  vec3_addf  (const vec3, const float);
vec3  vec3_subf  (const vec3, const float);
vec3  vec3_mulf  (const vec3, const float);
vec3  vec3_divf  (const vec3, const float);
vec3  vec3_add   (const vec3, const vec3);
vec3  vec3_sub   (const vec3, const vec3);
vec3  vec3_mul   (const vec3, const vec3);
vec3  vec3_div   (const vec3, const vec3);
vec3  vec3_diff  (const vec3, const vec3);
vec3  vec3_crs   (const vec3, const vec3);
vec3  vec3_norm  (const vec3);
vec3  vec3_floor (const vec3);
float vec3_dist  (const vec3, const vec3);
float vec3_dot   (const vec3, const vec3);
void  vec3_maddf (vec3*, const float);
void  vec3_msubf (vec3*, const float);
void  vec3_mmulf (vec3*, const float);
void  vec3_mdivf (vec3*, const float);
void  vec3_madd  (vec3*, const vec3);
void  vec3_msub  (vec3*, const vec3);
void  vec3_mmul  (vec3*, const vec3);
void  vec3_mdiv  (vec3*, const vec3);
void  vec3_mdiff (vec3*, const vec3);
void  vec3_mcrs  (vec3*, const vec3);
void  vec3_mnorm (vec3*);
void  vec3_mfloor(vec3*);

bool  vec4_is0   (const vec4);
bool  vec4_equal (const vec4, const vec4);
vec4  vec4_addf  (const vec4, const float);
vec4  vec4_subf  (const vec4, const float);
vec4  vec4_mulf  (const vec4, const float);
vec4  vec4_divf  (const vec4, const float);
vec4  vec4_add   (const vec4, const vec4);
vec4  vec4_sub   (const vec4, const vec4);
vec4  vec4_mul   (const vec4, const vec4);
vec4  vec4_div   (const vec4, const vec4);
vec4  vec4_diff  (const vec4, const vec4);
vec4  vec4_norm  (const vec4);
vec4  vec4_floor (const vec4);
float vec4_dist  (const vec4, const vec4);
float vec4_dot   (const vec4, const vec4);
void  vec4_maddf (vec4*, const float);
void  vec4_msubf (vec4*, const float);
void  vec4_mmulf (vec4*, const float);
void  vec4_mdivf (vec4*, const float);
void  vec4_madd  (vec4*, const vec4);
void  vec4_msub  (vec4*, const vec4);
void  vec4_mmul  (vec4*, const vec4);
void  vec4_mdiv  (vec4*, const vec4);
void  vec4_mdiff (vec4*, const vec4);
void  vec4_mnorm (vec4*);
void  vec4_mfloor(vec4*);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_VEC_MATH_INCLUDED_
