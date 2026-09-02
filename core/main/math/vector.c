#include "math/vector.h"


vec2 vec2_add(const vec2 a, const vec2 b) { return VECTOR2(a.x + b.x, a.y + b.y); }
vec3 vec3_add(const vec3 a, const vec3 b) { return VECTOR3(a.x + b.x, a.y + b.y, a.z + b.z); }
vec4 vec4_add(const vec4 a, const vec4 b) { return VECTOR4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }

vec2 vec2_sub(const vec2 a, const vec2 b) { return VECTOR2(a.x - b.x, a.y - b.y); }
vec3 vec3_sub(const vec3 a, const vec3 b) { return VECTOR3(a.x - b.x, a.y - b.y, a.z - b.z); }
vec4 vec4_sub(const vec4 a, const vec4 b) { return VECTOR4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }

vec2 vec2_diff(const vec2 a, const vec2 b) { return VECTOR2(imath_fabs(a.x - b.x), imath_fabs(a.y - b.y)); }
vec3 vec3_diff(const vec3 a, const vec3 b) { return VECTOR3(imath_fabs(a.x - b.x), imath_fabs(a.y - b.y), imath_fabs(a.z - b.z)); }
vec4 vec4_diff(const vec4 a, const vec4 b) { return VECTOR4(imath_fabs(a.x - b.x), imath_fabs(a.y - b.y), imath_fabs(a.z - b.z), imath_fabs(a.w - b.w)); }

vec2 vec2_mul(const vec2 a, const float f) { return VECTOR2(a.x * f, a.y * f); }
vec3 vec3_mul(const vec3 a, const float f) { return VECTOR3(a.x * f, a.y * f, a.z * f); }
vec4 vec4_mul(const vec4 a, const float f) { return VECTOR4(a.x * f, a.y * f, a.z * f, a.w * f); }

vec2 vec2_mulv(const vec2 a, const vec2 b) { return VECTOR2(a.x * b.x, a.y * b.y); }
vec3 vec3_mulv(const vec3 a, const vec3 b) { return VECTOR3(a.x * b.x, a.y * b.y, a.z * b.z); }
vec4 vec4_mulv(const vec4 a, const vec4 b) { return VECTOR4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }

vec2 vec2_div(const vec2 a, const float f) { return VECTOR2(a.x / f, a.y / f); }
vec3 vec3_div(const vec3 a, const float f) { return VECTOR3(a.x / f, a.y / f, a.z / f); }
vec4 vec4_div(const vec4 a, const float f) { return VECTOR4(a.x / f, a.y / f, a.z / f, a.w / f); }

float vec2_dist(const vec2 a, const vec2 b) { vec2 d = vec2_diff(a, b); return imath_hypot(d.x, d.y); }
float vec3_dist(const vec3 a, const vec3 b) { vec2 d = vec3_diff(a, b); return imath_len(d.v, 3); }
float vec4_dist(const vec4 a, const vec4 b) { vec2 d = vec4_diff(a, b); return imath_len(d.v, 4); }

vec2 vec2_norm(const vec2 a) { float d = vec2_dist(a); return vec2_div(a, d);}
vec3 vec3_norm(const vec3 a) { float d = vec3_dist(a); return vec3_div(a, d);}
vec4 vec4_norm(const vec4 a) { float d = vec4_dist(a); return vec4_div(a, d);}

float vec2_dot(const vec2 a, const vec2 b) { return a.x * b.x + a.y * b.y; }
float vec3_dot(const vec3 a, const vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float vec4_dot(const vec4 a, const vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

float vec2_rad(const vec2 a, const vec2 b) { return imath_acos(vec2_dot(a,b) / (vec2_dist(a) * vec2_dist(b))); }

