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

#define IVEC2_INIT(F) CLIT(ivec2){.x = (F), .y = (F)}
#define IVEC2(X,Y)    CLIT(ivec2){.x = (X), .y = (Y)}

#define UVEC2_INIT(F) CLIT(uvec2){.x = (F), .y = (F)}
#define UVEC2(X,Y)    CLIT(uvec2){.x = (X), .y = (Y)}

#define VEC2_ZERO    CLIT(vec2){0.0f, 0.0f}
#define VEC2_ONE     CLIT(vec2){1.0f, 1.0f}
#define VEC2_INIT(F) CLIT(vec2){ (F),  (F)}
#define VEC2(X,Y)    CLIT(vec2){ (X),  (Y)}

#define VEC3_ZERO    CLIT(vec3){0.0f, 0.0f, 0.0f}
#define VEC3_ONE     CLIT(vec3){1.0f, 1.0f, 1.0f}
#define VEC3_INIT(F) CLIT(vec3){ (F),  (F),  (F)}
#define VEC3(X,Y,Z)  CLIT(vec3){ (X),  (Y),  (Z)}

#define VEC4_ZERO     CLIT(vec4){0.0f, 0.0f, 0.0f, 0.0f}
#define VEC4_ONE      CLIT(vec4){1.0f, 1.0f, 1.0f, 1.0f}
#define VEC4_INIT(F)  CLIT(vec4){ (F),  (F),  (F),  (F)}
#define VEC4(X,Y,Z,W) CLIT(vec4){ (X),  (Y),  (Z),  (W)}

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool ivec2_is0(const ivec2);

bool uvec2_is0(const uvec2);

bool vec2_is0(const vec2);
bool vec3_is0(const vec3);
bool vec4_is0(const vec4);

ivec2 ivec2_add(const ivec2, const ivec2);

vec2 vec2_add(const vec2, const vec2);
vec3 vec3_add(const vec3, const vec3);
vec4 vec4_add(const vec4, const vec4);

ivec2 ivec2_sub(const ivec2, const ivec2);

void ivec2_msub(ivec2 *, const ivec2);

vec2 vec2_sub(const vec2, const vec2);
vec3 vec3_sub(const vec3, const vec3);
vec4 vec4_sub(const vec4, const vec4);

vec2 vec2_diff(const vec2, const vec2);
vec3 vec3_diff(const vec3, const vec3);
vec4 vec4_diff(const vec4, const vec4);

vec2 vec2_mul(const vec2, const float);
vec3 vec3_mul(const vec3, const float);
vec4 vec4_mul(const vec4, const float);

vec2 vec2_mulv(const vec2, const vec2);
vec3 vec3_mulv(const vec3, const vec3);
vec4 vec4_mulv(const vec4, const vec4);

vec2 vec2_div(const vec2, const float);
vec3 vec3_div(const vec3, const float);
vec4 vec4_div(const vec4, const float);

float vec2_dist(const vec2, const vec2);
float vec3_dist(const vec3, const vec3);
float vec4_dist(const vec4, const vec4);

vec2 vec2_norm(const vec2);
vec3 vec3_norm(const vec3);
vec4 vec4_norm(const vec4);

float vec2_dot(const vec2, const vec2);
float vec3_dot(const vec3, const vec3);
float vec4_dot(const vec4, const vec4);

float vec2_rad(const vec2, const vec2);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_VEC_MATH_INCLUDED_
