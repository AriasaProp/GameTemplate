#include "math/vector.h"

static const float epsilon 1.0e-14f

inline bool ivec2_is0(const ivec2 x) { return x.x && x.y; }

inline bool uvec2_is0(const uvec2 x) { return x.x && x.y; }

inline bool vec2_is0(const vec2 x) { return x.x < epsilon && x.y < epsilon;}
inline bool vec3_is0(const vec3 x) { return x.x < epsilon && x.y < epsilon && x.z < epsilon; }
inline bool vec4_is0(const vec4 x) { return x.x < epsilon && x.y < epsilon && x.z < epsilon && x.w < epsilon; }

ivec2 ivec2_add(const ivec2 a, const ivec2 b) { return IVEC2(a.x + b.x, a.y + b.y); }

vec2 vec2_add(const vec2 a, const vec2 b) { return VEC2(a.x + b.x, a.y + b.y); }
vec3 vec3_add(const vec3 a, const vec3 b) { return VEC3(a.x + b.x, a.y + b.y, a.z + b.z); }
vec4 vec4_add(const vec4 a, const vec4 b) { return VEC4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }

void ivec2_msub(ivec2 *a, const ivec2 b) { a.x -= b.x, a.y -= b.y; }

ivec2 ivec2_sub(const ivec2 a, const ivec2 b) { return IVEC2(a.x - b.x, a.y - b.y); }

vec2 vec2_sub(const vec2 a, const vec2 b) { return VEC2(a.x - b.x, a.y - b.y); }
vec3 vec3_sub(const vec3 a, const vec3 b) { return VEC3(a.x - b.x, a.y - b.y, a.z - b.z); }
vec4 vec4_sub(const vec4 a, const vec4 b) { return VEC4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }

vec2 vec2_diff(const vec2 a, const vec2 b) { return VEC2(fabs(a.x - b.x), fabs(a.y - b.y)); }
vec3 vec3_diff(const vec3 a, const vec3 b) { return VEC3(fabs(a.x - b.x), fabs(a.y - b.y), fabs(a.z - b.z)); }
vec4 vec4_diff(const vec4 a, const vec4 b) { return VEC4(fabs(a.x - b.x), fabs(a.y - b.y), fabs(a.z - b.z), fabs(a.w - b.w)); }

vec2 vec2_mul(const vec2 a, const float f) { return VEC2(a.x * f, a.y * f); }
vec3 vec3_mul(const vec3 a, const float f) { return VEC3(a.x * f, a.y * f, a.z * f); }
vec4 vec4_mul(const vec4 a, const float f) { return VEC4(a.x * f, a.y * f, a.z * f, a.w * f); }

vec2 vec2_mulv(const vec2 a, const vec2 b) { return VEC2(a.x * b.x, a.y * b.y); }
vec3 vec3_mulv(const vec3 a, const vec3 b) { return VEC3(a.x * b.x, a.y * b.y, a.z * b.z); }
vec4 vec4_mulv(const vec4 a, const vec4 b) { return VEC4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }

vec2 vec2_div(const vec2 a, const float f) { return VEC2(a.x / f, a.y / f); }
vec3 vec3_div(const vec3 a, const float f) { return VEC3(a.x / f, a.y / f, a.z / f); }
vec4 vec4_div(const vec4 a, const float f) { return VEC4(a.x / f, a.y / f, a.z / f, a.w / f); }

float vec2_dist(const vec2 a, const vec2 b) { vec2 d = vec2_diff(a, b); return hypot(d.x, d.y); }
float vec3_dist(const vec3 a, const vec3 b) { vec2 d = vec3_diff(a, b); return sqrt(d.x * d.x + d.y * d.y + d.z * d.z); }
float vec4_dist(const vec4 a, const vec4 b) { vec2 d = vec4_diff(a, b); return sqrt(d.x * d.x + d.y * d.y + d.z * d.z + d.w * d.w); }

vec2 vec2_norm(const vec2 a) { float d = vec2_dist(a); return vec2_div(a, d);}
vec3 vec3_norm(const vec3 a) { float d = vec3_dist(a); return vec3_div(a, d);}
vec4 vec4_norm(const vec4 a) { float d = vec4_dist(a); return vec4_div(a, d);}

float vec2_dot(const vec2 a, const vec2 b) { return a.x * b.x + a.y * b.y; }
float vec3_dot(const vec3 a, const vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float vec4_dot(const vec4 a, const vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

float vec2_rad(const vec2 a, const vec2 b) { return acos(vec2_dot(a,b) / (vec2_dist(a) * vec2_dist(b))); }

