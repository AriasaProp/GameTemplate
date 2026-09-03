#include "math/vector.h"
#include <math.h>

static const float epsilon = 1.0e-14f;

bool  ivec2_is0  (const ivec2 v) { return v.x && v.y; }
bool  ivec2_equal(const ivec2 a, const ivec2 b) { return a.x == b.x && a.y == b.y; }
ivec2 ivec2_addi (const ivec2 v, const int i) { return IVEC2(v.x + i, v.y + i); }
ivec2 ivec2_subi (const ivec2 v, const int i) { return IVEC2(v.x - i, v.y - i); }
ivec2 ivec2_muli (const ivec2 v, const int i) { return IVEC2(v.x * i, v.y * i); }
ivec2 ivec2_divi (const ivec2 v, const int i) { return IVEC2(v.x / i, v.y / i); }
ivec2 ivec2_add  (const ivec2 a, const ivec2 b) { return IVEC2(a.x + b.x, a.y + b.y); }
ivec2 ivec2_sub  (const ivec2 a, const ivec2 b) { return IVEC2(a.x - b.x, a.y - b.y); }
ivec2 ivec2_mul  (const ivec2 a, const ivec2 b) { return IVEC2(a.x * b.x, a.y * b.y); }
ivec2 ivec2_div  (const ivec2 a, const ivec2 b) { return IVEC2(a.x / b.x, a.y / b.y); }
void  ivec2_maddi(ivec2 *v, const int i) { v->x += i, v->y += i; }
void  ivec2_msubi(ivec2 *v, const int i) { v->x -= i, v->y -= i; }
void  ivec2_mmuli(ivec2 *v, const int i) { v->x *= i, v->y *= i; }
void  ivec2_mdivi(ivec2 *v, const int i) { v->x /= i, v->y /= i; }
void  ivec2_madd (ivec2 *a, const ivec2 b) { a->x += b.x, a->y += b.y; }
void  ivec2_msub (ivec2 *a, const ivec2 b) { a->x -= b.x, a->y -= b.y; }
void  ivec2_mmul (ivec2 *a, const ivec2 b) { a->x *= b.x, a->y *= b.y; }
void  ivec2_mdiv (ivec2 *a, const ivec2 b) { a->x /= b.x, a->y /= b.y; }

bool  uvec2_is0  (const uvec2 v) { return v.x && v.y; }
bool  uvec2_equal(const uvec2 a, const uvec2 b) { return a.x == b.x && a.y == b.y; }
uvec2 uvec2_addi (const uvec2 v, const uint i) { return UVEC2(v.x + i, v.y + i); }
uvec2 uvec2_subi (const uvec2 v, const uint i) { return UVEC2(v.x - i, v.y - i); }
uvec2 uvec2_muli (const uvec2 v, const uint i) { return UVEC2(v.x * i, v.y * i); }
uvec2 uvec2_divi (const uvec2 v, const uint i) { return UVEC2(v.x / i, v.y / i); }
uvec2 uvec2_add  (const uvec2 a, const uvec2 b) { return UVEC2(a.x + b.x, a.y + b.y); }
uvec2 uvec2_sub  (const uvec2 a, const uvec2 b) { return UVEC2(a.x - b.x, a.y - b.y); }
uvec2 uvec2_mul  (const uvec2 a, const uvec2 b) { return UVEC2(a.x * b.x, a.y * b.y); }
uvec2 uvec2_div  (const uvec2 a, const uvec2 b) { return UVEC2(a.x / b.x, a.y / b.y); }
void  uvec2_maddi(uvec2 *v, const uint i) { v->x += i, v->y += i; }
void  uvec2_msubi(uvec2 *v, const uint i) { v->x -= i, v->y -= i; }
void  uvec2_mmuli(uvec2 *v, const uint i) { v->x *= i, v->y *= i; }
void  uvec2_mdivi(uvec2 *v, const uint i) { v->x /= i, v->y /= i; }
void  uvec2_madd (uvec2 *a, const uvec2 b) { a->x += b.x, a->y += b.y; }
void  uvec2_msub (uvec2 *a, const uvec2 b) { a->x -= b.x, a->y -= b.y; }
void  uvec2_mmul (uvec2 *a, const uvec2 b) { a->x *= b.x, a->y *= b.y; }
void  uvec2_mdiv (uvec2 *a, const uvec2 b) { a->x /= b.x, a->y /= b.y; }

bool  vec2_is0   (const vec2 v) { return v.x < epsilon && v.y < epsilon; }
bool  vec2_equal (const vec2 a, const vec2 b) { return fabs(a.x - b.x) < epsilon && fabs(a.y - b.y) < epsilon; }
vec2  vec2_addf  (const vec2 v, const float f) { return VEC2(v.x + f, v.y + f); }
vec2  vec2_subf  (const vec2 v, const float f) { return VEC2(v.x - f, v.y - f); }
vec2  vec2_mulf  (const vec2 v, const float f) { return VEC2(v.x * f, v.y * f); }
vec2  vec2_divf  (const vec2 v, const float f) { return VEC2(v.x / f, v.y / f); }
vec2  vec2_add   (const vec2 a, const vec2 b) { return VEC2(a.x + b.x, a.y + b.y); }
vec2  vec2_sub   (const vec2 a, const vec2 b) { return VEC2(a.x - b.x, a.y - b.y); }
vec2  vec2_mul   (const vec2 a, const vec2 b) { return VEC2(a.x * b.x, a.y * b.y); }
vec2  vec2_div   (const vec2 a, const vec2 b) { return VEC2(a.x / b.x, a.y / b.y); }
vec2  vec2_diff  (const vec2 a, const vec2 b) { return VEC2(fabs(a.x - b.x), fabs(a.y - b.y)); }
vec2  vec2_norm  (const vec2 a) { return vec2_divf(a, vec2_dist(a));}
vec2  vec2_floor (const vec2 a) { return VEC2(floorf(a.x), floorf(a.y)); }
float vec2_dist  (const vec2 a, const vec2 b) { vec2 d = vec2_diff(a, b); return hypot(d.x, d.y); }
float vec2_dot   (const vec2 a, const vec2 b) { return a.x * b.x + a.y * b.y; }
float vec2_rad   (const vec2 a, const vec2 b) { return acos(vec2_dot(a,b) / (vec2_dist(a) * vec2_dist(b))); }
void  vec2_maddf (vec2 *v, const float f) { v->x += f, v->y += f; }
void  vec2_msubf (vec2 *v, const float f) { v->x -= f, v->y -= f; }
void  vec2_mmulf (vec2 *v, const float f) { v->x *= f, v->y *= f; }
void  vec2_mdivf (vec2 *v, const float f) { v->x /= f, v->y /= f; }
void  vec2_madd  (vec2 *a, const vec2 b) { a->x += b.x, a->y += b.y; }
void  vec2_msub  (vec2 *a, const vec2 b) { a->x -= b.x, a->y -= b.y; }
void  vec2_mmul  (vec2 *a, const vec2 b) { a->x *= b.x, a->y *= b.y; }
void  vec2_mdiv  (vec2 *a, const vec2 b) { a->x /= b.x, a->y /= b.y; }
void  vec2_mdiff (vec2 *a, const vec2 b) { a->x = fabs(a->x - b.x), a->y = fabs(a->y - b.y); }
void  vec2_mnorm (vec2 *a) { vec2_mdivf(a, vec2_dist(*a)); }
void  vec2_mfloor(vec2 *a) { a->x = floorf(a->x), a->y = floorf(a->y);}


bool  vec3_is0   (const vec3 v) { return v.x < epsilon && v.y < epsilon && v.z < epsilon; }
bool  vec3_equal (const vec3 a, const vec3 b) { return fabs(a.x - b.x) < epsilon && fabs(a.y - b.y) < epsilon && fabs(a.z - b.z) < epsilon; }
vec3  vec3_addf  (const vec3 a, const float f) { return VEC3(a.x + f, a.y + f, a.z + f); }
vec3  vec3_subf  (const vec3 a, const float f) { return VEC3(a.x - f, a.y - f, a.z - f); }
vec3  vec3_mulf  (const vec3 a, const float f) { return VEC3(a.x * f, a.y * f, a.z * f); }
vec3  vec3_divf  (const vec3 a, const float f) { return VEC3(a.x / f, a.y / f, a.z / f); }
vec3  vec3_add   (const vec3 a, const vec3 b) { return VEC3(a.x + b.x, a.y + b.y, a.z + b.z); }
vec3  vec3_sub   (const vec3 a, const vec3 b) { return VEC3(a.x - b.x, a.y - b.y, a.z - b.z); }
vec3  vec3_mul   (const vec3 a, const vec3 b) { return VEC3(a.x * b.x, a.y * b.y, a.z * b.z); }
vec3  vec3_div   (const vec3 a, const vec3 b) { return VEC3(a.x / b.x, a.y / b.y, a.z / b.z); }
vec3  vec3_diff  (const vec3 a, const vec3 b) { return VEC3(fabs(a.x - b.x), fabs(a.y - b.y), fabs(a.z - b.z)); }
vec3  vec3_crs   (const vec3 a, const vec3 b) { return VEC3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
vec3  vec3_norm  (const vec3 a) { return vec3_div(a, vec3_dist(a));}
vec3  vec3_floor (const vec3 a) { return VEC3(floorf(a.x), floorf(a.y), floorf(a.z)); }
float vec3_dist  (const vec3 a, const vec3 b) { vec3 d = vec3_diff(a, b); return sqrt(d.x * d.x + d.y * d.y + d.z * d.z); }
float vec3_dot   (const vec3 a, const vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
void  vec3_maddf (vec3 *v, const float f) { v->x += f, v->y += f, v->z += f; }
void  vec3_msubf (vec3 *v, const float f) { v->x -= f, v->y -= f, v->z -= f; }
void  vec3_mmulf (vec3 *v, const float f) { v->x *= f, v->y *= f, v->z *= f; }
void  vec3_mdivf (vec3 *v, const float f) { v->x /= f, v->y /= f, v->z /= f; }
void  vec3_madd  (vec3 *a, const vec3 b) { a->x += b.x, a->y += b.y, a->z += b.z; }
void  vec3_msub  (vec3 *a, const vec3 b) { a->x -= b.x, a->y -= b.y, a->z -= b.z; }
void  vec3_mmul  (vec3 *a, const vec3 b) { a->x *= b.x, a->y *= b.y, a->z *= b.z; }
void  vec3_mdiv  (vec3 *a, const vec3 b) { a->x /= b.x, a->y /= b.y, a->z /= b.z; }
void  vec3_mdiff (vec3 *a, const vec3 b) { a->x = fabs(a->x - b.x), a->y = fabs(a->y - b.y), a->z = fabs(a->z - b.z); }
void  vec3_mcrs  (vec3 *a, const vec3 b) { *a = VEC3(a->y * b.z - a->z * b.y, a->z * b.x - a->x * b.z, a->x * b.y - a->y * b.x); }
void  vec3_mnorm (vec3 *a) { vec3_mdiv(a, vec3_dist(*a));}
void  vec3_mfloor(vec3 *a) { a->x = floorf(a->x), a->y = floorf(a->y), a->z = floorf(a->z); }

bool  vec4_is0   (const vec4 v) { return v.x < epsilon && v.y < epsilon && v.z < epsilon && v.w < epsilon; }
bool  vec4_equal (const vec4 a, const vec4 b) { return fabs(a.x - b.x) < epsilon && fabs(a.y - b.y) < epsilon && fabs(a.z - b.z) < epsilon && fabs(a.w - b.w) < epsilon; }
vec4  vec4_addf  (const vec4 a, const float f) { return VEC4(a.x + f, a.y + f, a.z + f, a.w + f); }
vec4  vec4_subf  (const vec4 a, const float f) { return VEC4(a.x - f, a.y - f, a.z - f, a.w - f); }
vec4  vec4_mulf  (const vec4 a, const float f) { return VEC4(a.x * f, a.y * f, a.z * f, a.w * f); }
vec4  vec4_divf  (const vec4 a, const float f) { return VEC4(a.x / f, a.y / f, a.z / f, a.w / f); }
vec4  vec4_add   (const vec4 a, const vec4 b) { return VEC4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
vec4  vec4_sub   (const vec4 a, const vec4 b) { return VEC4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
vec4  vec4_mul   (const vec4 a, const vec4 b) { return VEC4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
vec4  vec4_div   (const vec4 a, const vec4 b) { return VEC4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
vec4  vec4_diff  (const vec4 a, const vec4 b) { return VEC4(fabs(a.x - b.x), fabs(a.y - b.y), fabs(a.z - b.z), fabs(a.w - b.w)); }
vec4  vec4_norm  (const vec4 a) { return vec4_div(a, vec4_dist(a));}
vec4  vec4_floor (const vec4 a) { return VEC4(floorf(a.x), floorf(a.y), floorf(a.z), floorf(a.w)); }
float vec4_dist  (const vec4 a, const vec4 b) { vec4 d = vec4_diff(a, b); return sqrt(d.x * d.x + d.y * d.y + d.z * d.z + d.w * d.w); }
float vec4_dot   (const vec4 a, const vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
void  vec4_maddf (vec4 *v, const float f) { v->x += f, v->y += f, v->z += f, v->w += f; }
void  vec4_msubf (vec4 *v, const float f) { v->x -= f, v->y -= f, v->z -= f, v->w -= f; }
void  vec4_mmulf (vec4 *v, const float f) { v->x *= f, v->y *= f, v->z *= f, v->w *= f; }
void  vec4_mdivf (vec4 *v, const float f) { v->x /= f, v->y /= f, v->z /= f, v->w /= f; }
void  vec4_madd  (vec4 *a, const vec4 b) { a->x += b.x, a->y += b.y, a->z += b.z, a->w += b.w; }
void  vec4_msub  (vec4 *a, const vec4 b) { a->x -= b.x, a->y -= b.y, a->z -= b.z, a->w -= b.w; }
void  vec4_mmul  (vec4 *a, const vec4 b) { a->x *= b.x, a->y *= b.y, a->z *= b.z, a->w *= b.w; }
void  vec4_mdiv  (vec4 *a, const vec4 b) { a->x /= b.x, a->y /= b.y, a->z /= b.z, a->w /= b.w; }
void  vec4_mdiff (vec4 *a, const vec4 b) { a->x = fabs(a->x - b.x), a->y = fabs(a->y - b.y), a->z = fabs(a->z - b.z), a->w = fabs(a->w - b.w); }
void  vec4_mnorm (vec4 *a) { vec4_mdiv(a, vec4_dist(*a));}
void  vec4_mfloor(vec4 *a) { a->x = floorf(a->x), a->y = floorf(a->y), a->z = floorf(a->z), a->w = floorf(a->w); }
