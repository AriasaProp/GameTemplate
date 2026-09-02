/* *****************************************************************************
 * color.h v0.0.0000
 * 
 * color object
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _COLOR_MATH_INCLUDED_
#define _COLOR_MATH_INCLUDED_

#include "common.h"


typedef PACKED(struct) { float r, g, b; } rgbf;
typedef union {
	PACKED(struct) { float r, g, b, a; };
	PACKED(struct) { rgbf rgb; float __pad_alpha_; };
} rgbaf;
typedef struct { float h, s, v; } hsv;
typedef union {
	PACKED(struct) { float h, s, v, a; };
	PACKED(struct) { hsv rgb; float __pad_alpha_; };
} hsva;
typedef struct { ubyte r, g, b; } rgb_u;
typedef union {
	uint32 u;
	PACKED(struct) { ubyte r, g, b, a; };
	PACKED(struct) { rgb_u rgb; ubyte __pad_alpha_; };
} rgba_u;

#define FRGBA_ZERO  CLIT(rgbaf){.r = 0.f, .g = 0.f, .b = 0.f, .a = 0.f}
#define FRGBA_BLACK CLIT(rgbaf){.r = 0.f, .g = 0.f, .b = 0.f, .a = 1.f}
#define FRGBA_RED   CLIT(rgbaf){.r = 1.f, .g = 0.f, .b = 0.f, .a = 1.f}
#define FRGBA_GREEN CLIT(rgbaf){.r = 0.f, .g = 1.f, .b = 0.f, .a = 1.f}
#define FRGBA_BLUE  CLIT(rgbaf){.r = 0.f, .g = 0.f, .b = 1.f, .a = 1.f}

#define URGBA_ZERO  CLIT(rgba_u){.u = 0x00000000}
#define URGBA_BLACK CLIT(rgba_u){.u = 0x000000ff}
#define URGBA_RED   CLIT(rgba_u){.u = 0xff0000ff}
#define URGBA_GREEN CLIT(rgba_u){.u = 0x00ff00ff}
#define URGBA_BLUE  CLIT(rgba_u){.u = 0x0000ffff}

hsva color_rgbaf_hsva(rgbaf);
hsva color_rgbf_hsva (rgbf,float);
hsva color_hsv_hsva  (hsv,float);
hsv  color_rgbf_hsv  (rgbf);

rgbaf color_hsva_rgbaf(hsva);
rgbaf color_hsv_rgbaf (hsv,float);
rgbaf color_rgbf_rgbaf(rgbf,float);
rgbf  color_hsv_rgbf  (hsv);


#endif // _COLOR_MATH_INCLUDED_