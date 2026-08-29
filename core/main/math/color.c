/* *****************************************************************************
 * color.c v0.0.0000
 * 
 * color object
 * 
 * 
 * 
 * *****************************************************************************/


#include "math/color.h"

hsva color_rgbaf_hsva(rgbaf x) {
  return CLIT(hsva){.hsv = color_rgbf_hsv(x.rgb), .a = x.a};
}
hsva color_rgbf_hsva(rgbf x, float a) {
  return CLIT(hsva){.hsv = color_rgbf_hsv(x), .a = a};
}
hsva color_hsv_hsva(hsv x,float a) {
  return CLIT(hsva){.hsv = x, .a = a};
}
hsv color_rgbf_hsv(rgbf x) {
  float r = fmax(fmin(x.r, 1.f), 0.f);
  float g = fmax(fmin(x.g, 1.f), 0.f);
  float b = fmax(fmin(x.b, 1.f), 0.f);
  float v = fmax(r,fmax(g,b));
  float c = v - fmin(r,fmin(g,b));
  hsv ret;
  ret.v = v;
  ret.s = (v > 0.f) ? 1.f - fmin(r,fmin(g,b))/v : 0.f;
  if (v == r) ret.h = 60.0f * (g - b) / c;
  else if (v == g) ret.h = 60.0f * (b - r) / c;
  else if (v == b) ret.h = 60.0f * (r - g) / c;
  else ret.h = 0;
  return ret;
}

rgbaf color_hsva_rgbaf(hsva x) {
  return CLIT(rgbaf){.hsv = color_hsv_rgbf(x.hsv), .a = x.a};
}
rgbaf color_hsv_rgbaf(hsv x,float a) {
  return CLIT(rgbaf){.rgb = color_hsv_rgbf(x), .a = a};
}
rgbaf color_rgbf_rgbaf(rgbf x,float a) {
  return CLIT(rgbaf){.rgb = x, .a = a};
}
rgbf color_hsv_rgbf(hsv x) {
  float d = fmodf(x.h/60.f ,6.f);
  float v = fmax(0.f, fmin(1.f, x.v));
  float s = 1.f - fmax(0.f, fmin(1.f, x.s));
  return CLIT(rgbf){
    .r = v * fmax(fmin(-1.f + fabs(d - 3.f),1.f),s),
    .g = v * fmax(fmin( 2.f - fabs(d - 2.f),1.f),s),
    .b = v * fmax(fmin( 2.f - fabs(d - 4.f),1.f),s),
  };
}
