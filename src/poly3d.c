/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      The 3d polygon rasteriser.
 *
 *      By Shawn Hargreaves.
 *
 *      Hicolor support added by Przemek Podsiadly. Complete support for
 *      all drawing modes in every color depth and MMX optimisations
 *      added by Calin Andrian. Subpixel and subtexel accuracy, triangle
 *      functions and speed enhancements by Bertrand Coconnier.
 *
 *      See readme.txt for copyright information.
 */


#include <limits.h>
#include <float.h>

#include "allegro.h"
#include "allegro/aintern.h"

#ifdef ALLEGRO_MMX_HEADER
   #include ALLEGRO_MMX_HEADER
#endif

#ifdef ALLEGRO_USE_C
   #undef ALLEGRO_MMX
#endif


#ifdef ALLEGRO_MMX

/* for use by iscan.s */
unsigned long _mask_mmx_15[] = { 0x03E0001F, 0x007C };
unsigned long _mask_mmx_16[] = { 0x07E0001F, 0x00F8 };

#endif

/*
 * Variable for future evolutions. _polygon_mode will be used to enable or
 * disable subpixel and subtexel accuracy or texture wrap.
 */
static int _polygon_mode = 1;
#define POLY_SUB_ACCURACY 1
#define POLY_TEX_WRAP 2

/* fceil :
 * Fixed point version of ceil().
 * Note that it returns an integer result (not a fixed one)
 */
#if (defined ALLEGRO_GCC) && (defined ALLEGRO_I386)

AL_INLINE(int, fceil, (fixed x),
{
   int result;

   asm (
      " addl $0xFFFF, %0 ;"	/* ceil () */
      " jns 0f ;"
      " jo 1f ;"

      "0:"
      " sarl $16, %0 ;"		/* convert to int */
      " jmp 2f ;"

      "1:"
      " movl %3, %0 ;"		/* on overflow, set errno */
      " movl %2, (%0) ;"
      " movl $0x7FFF, %0 ;"	/* and return large int */

      "2:"

   : "=r" (result)		/* result in a register */

   : "0" (x),			/* x in the output register */
     "i" (ERANGE),
     "m" (allegro_errno)

   : "%cc", "memory"		/* clobbers flags and errno */
   );

   return result;
})

#else

AL_INLINE(int, fceil, (fixed x),
{
   x += 0xFFFF;
   if (x >= 0x80000000) {
      *allegro_errno = ERANGE;
      return 0x7FFF;
   }

   return (x >> 16);
})

#endif

SCANLINE_FILLER _optim_alternative_drawer;

void _poly_scanline_dummy(unsigned long addr, int w, POLYGON_SEGMENT *info) { }



/* _fill_3d_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 3d 
 *  rasterising code, using fixed point vertex structures.
 */
void _fill_3d_edge_structure(POLYGON_EDGE *edge, AL_CONST V3D *v1, AL_CONST V3D *v2, int flags, BITMAP *bmp)
{
   int r1, r2, g1, g2, b1, b2;
   fixed h, step;

   /* swap vertices if they are the wrong way up */
   if (v2->y < v1->y) {
      AL_CONST V3D *vt;

      vt = v1;
      v1 = v2;
      v2 = vt;
   }

   /* set up screen rasterising parameters */
   edge->top = fceil(v1->y);
   edge->bottom = fceil(v2->y) - 1;
   h = v2->y - v1->y;
   step = (edge->top << 16) - v1->y;

   edge->dx = fdiv(v2->x - v1->x, h);
   edge->x = (_polygon_mode & POLY_SUB_ACCURACY ? v1->x + fmul(step, edge->dx) : v1->x);

   edge->prev = NULL;
   edge->next = NULL;
   edge->w = 0;

   if (flags & INTERP_FLAT) {
      /* clip edge */
      if (edge->top < bmp->ct) {
         edge->x += (bmp->ct - edge->top) * edge->dx;
	 edge->top = bmp->ct;
      }

      if (edge->bottom >= bmp->cb)
	 edge->bottom = bmp->cb - 1;

      return;
   }

   if (flags & INTERP_1COL) {
      /* single color shading interpolation */
      edge->dat.dc = fdiv(itofix(v2->c - v1->c), h);
      edge->dat.c = (_polygon_mode & POLY_SUB_ACCURACY ? itofix(v1->c) + fmul(step, edge->dat.dc) : itofix(v1->c));
   }

   if (flags & INTERP_3COL) {
      /* RGB shading interpolation */
      if (flags & COLOR_TO_RGB) {
	 int coldepth = bitmap_color_depth(bmp);
	 r1 = getr_depth(coldepth, v1->c);
	 r2 = getr_depth(coldepth, v2->c);
	 g1 = getg_depth(coldepth, v1->c);
	 g2 = getg_depth(coldepth, v2->c);
	 b1 = getb_depth(coldepth, v1->c);
	 b2 = getb_depth(coldepth, v2->c);
      } 
      else {
	 r1 = (v1->c >> 16) & 0xFF;
	 r2 = (v2->c >> 16) & 0xFF;
	 g1 = (v1->c >> 8) & 0xFF;
	 g2 = (v2->c >> 8) & 0xFF;
	 b1 = v1->c & 0xFF;
	 b2 = v2->c & 0xFF;
      }

      edge->dat.dr = fdiv(itofix(r2 - r1), h);
      edge->dat.dg = fdiv(itofix(g2 - g1), h);
      edge->dat.db = fdiv(itofix(b2 - b1), h);
      edge->dat.r = itofix(r1);
      edge->dat.g = itofix(g1);
      edge->dat.b = itofix(b1);
      if (_polygon_mode & POLY_SUB_ACCURACY) {
	 edge->dat.r += fmul(step, edge->dat.dr);
	 edge->dat.g += fmul(step, edge->dat.dg);
	 edge->dat.b += fmul(step, edge->dat.db);
      }
   }

   if (flags & INTERP_FIX_UV) {
      /* fixed point (affine) texture interpolation */
      edge->dat.du = fdiv(v2->u - v1->u, h);
      edge->dat.dv = fdiv(v2->v - v1->v, h);
      edge->dat.u = v1->u;
      edge->dat.v = v1->v;
      if (_polygon_mode & POLY_SUB_ACCURACY) {
	 edge->dat.u += fmul(step, edge->dat.du);
	 edge->dat.v += fmul(step, edge->dat.dv);
      }
   }

   if (flags & INTERP_Z) {
      float h1 = 65536. / h;
      float step_f = fixtof(step);

      /* Z (depth) interpolation */
      float z1 = 65536. / v1->z;
      float z2 = 65536. / v2->z;

      edge->dat.dz = (z2 - z1) * h1;
      edge->dat.z = (_polygon_mode & POLY_SUB_ACCURACY ? z1 + edge->dat.dz * step_f : z1);

      if (flags & INTERP_FLOAT_UV) {
	 /* floating point (perspective correct) texture interpolation */
	 float fu1 = v1->u * z1;
	 float fv1 = v1->v * z1;
	 float fu2 = v2->u * z2;
	 float fv2 = v2->v * z2;

	 edge->dat.dfu = (fu2 - fu1) * h1;
	 edge->dat.dfv = (fv2 - fv1) * h1;
	 edge->dat.fu = fu1;
	 edge->dat.fv = fv1;
         if (_polygon_mode & POLY_SUB_ACCURACY) {
	    edge->dat.fu += edge->dat.dfu * step_f;
	    edge->dat.fv += edge->dat.dfv * step_f;
         }
      }
   }

   /* clip edge */
   if (edge->top < bmp->ct) {
      int gap = bmp->ct - edge->top;
      edge->top = bmp->ct;
      edge->x += gap * edge->dx;
      _clip_polygon_segment(&(edge->dat),gap,flags);
   }

   if (edge->bottom >= bmp->cb)
      edge->bottom = bmp->cb - 1;
}



/* _fill_3d_edge_structure_f:
 *  Polygon helper function: initialises an edge structure for the 3d 
 *  rasterising code, using floating point vertex structures.
 */
void _fill_3d_edge_structure_f(POLYGON_EDGE *edge, AL_CONST V3D_f *v1, AL_CONST V3D_f *v2, int flags, BITMAP *bmp)
{
   int r1, r2, g1, g2, b1, b2;
   fixed h, step;
   float h_f, h1;

   /* swap vertices if they are the wrong way up */
   if (v2->y < v1->y) {
      AL_CONST V3D_f *vt;

      vt = v1;
      v1 = v2;
      v2 = vt;
   }

   /* set up screen rasterising parameters */
   edge->top = fceil(ftofix(v1->y));
   edge->bottom = fceil(ftofix(v2->y)) - 1;

   h1 = 1.0 / (v2->y - v1->y);
   h = ftofix(v2->y - v1->y);
   step = (edge->top << 16) - ftofix(v1->y);

   edge->dx = ftofix((v2->x - v1->x)  * h1);
   edge->x = (_polygon_mode & POLY_SUB_ACCURACY ? ftofix(v1->x) + fmul(step, edge->dx) : ftofix(v1->x));

   edge->prev = NULL;
   edge->next = NULL;
   edge->w = 0;

   if (flags & INTERP_FLAT) {
      /* clip edge */
      if (edge->top < bmp->ct) {
	 edge->x += (bmp->ct - edge->top) * edge->dx;
	 edge->top = bmp->ct;
      }

      if (edge->bottom >= bmp->cb)
	 edge->bottom = bmp->cb - 1;

      return;
   }

   if (flags & INTERP_1COL) {
      /* single color shading interpolation */
      edge->dat.dc = fdiv(itofix(v2->c - v1->c), h);
      edge->dat.c = (_polygon_mode & POLY_SUB_ACCURACY ? itofix(v1->c) + fmul(step, edge->dat.dc) : itofix(v1->c));
   }

   if (flags & INTERP_3COL) {
      /* RGB shading interpolation */
      if (flags & COLOR_TO_RGB) {
	 int coldepth = bitmap_color_depth(bmp);
	 r1 = getr_depth(coldepth, v1->c);
	 r2 = getr_depth(coldepth, v2->c);
	 g1 = getg_depth(coldepth, v1->c);
	 g2 = getg_depth(coldepth, v2->c);
	 b1 = getb_depth(coldepth, v1->c);
	 b2 = getb_depth(coldepth, v2->c);
      } 
      else {
	 r1 = (v1->c >> 16) & 0xFF;
	 r2 = (v2->c >> 16) & 0xFF;
	 g1 = (v1->c >> 8) & 0xFF;
	 g2 = (v2->c >> 8) & 0xFF;
	 b1 = v1->c & 0xFF;
	 b2 = v2->c & 0xFF;
      }

      edge->dat.dr = fdiv(itofix(r2 - r1), h);
      edge->dat.dg = fdiv(itofix(g2 - g1), h);
      edge->dat.db = fdiv(itofix(b2 - b1), h);
      edge->dat.r = itofix(r1);
      edge->dat.g = itofix(g1);
      edge->dat.b = itofix(b1);
      if (_polygon_mode & POLY_SUB_ACCURACY) {
	 edge->dat.r += fmul(step, edge->dat.dr);
	 edge->dat.g += fmul(step, edge->dat.dg);
	 edge->dat.b += fmul(step, edge->dat.db);
      }
   }

   if (flags & INTERP_FIX_UV) {
      /* fixed point (affine) texture interpolation */
      edge->dat.du = ftofix((v2->u - v1->u) * h1);
      edge->dat.dv = ftofix((v2->v - v1->v) * h1);
      edge->dat.u = ftofix(v1->u);
      edge->dat.v = ftofix(v1->v);
      if (_polygon_mode & POLY_SUB_ACCURACY) {
	 edge->dat.u += fmul(step, edge->dat.du);
	 edge->dat.v += fmul(step, edge->dat.dv);
      }
   }

   if (flags & INTERP_Z) {
      float step_f = fixtof(step);

      /* Z (depth) interpolation */
      float z1 = 1.0 / v1->z;
      float z2 = 1.0 / v2->z;

      edge->dat.dz = (z2 - z1) * h1;
      edge->dat.z = (_polygon_mode & POLY_SUB_ACCURACY ? z1 + edge->dat.dz * step_f : z1);

      if (flags & INTERP_FLOAT_UV) {
	 /* floating point (perspective correct) texture interpolation */
	 float fu1 = v1->u * z1 * 65536.;
	 float fv1 = v1->v * z1 * 65536.;
	 float fu2 = v2->u * z2 * 65536.;
	 float fv2 = v2->v * z2 * 65536.;

	 edge->dat.dfu = (fu2 - fu1) * h1;
	 edge->dat.dfv = (fv2 - fv1) * h1;
	 edge->dat.fu = fu1;
	 edge->dat.fv = fv1;
         if (_polygon_mode & POLY_SUB_ACCURACY) {
	    edge->dat.fu += edge->dat.dfu * step_f;
	    edge->dat.fv += edge->dat.dfv * step_f;
         }
      }
   }

   /* clip edge */
   if (edge->top < bmp->ct) {
      int gap = bmp->ct - edge->top;
      edge->top = bmp->ct;
      edge->x += gap * edge->dx;
      _clip_polygon_segment(&(edge->dat),gap,flags);
   }

   if (edge->bottom >= bmp->cb)
      edge->bottom = bmp->cb - 1;
}



/* _get_scanline_filler:
 *  Helper function for deciding which rasterisation function and 
 *  interpolation flags we should use for a specific polygon type.
 */
SCANLINE_FILLER _get_scanline_filler(int type, int *flags, POLYGON_SEGMENT *info, BITMAP *texture, BITMAP *bmp)
{
   typedef struct POLYTYPE_INFO 
   {
      SCANLINE_FILLER filler;
      SCANLINE_FILLER alternative;
   } POLYTYPE_INFO;

   int polytype_interp_pal[] = 
   {
      INTERP_FLAT,
      INTERP_1COL,
      INTERP_3COL,
      INTERP_FIX_UV,
      INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
      INTERP_FIX_UV,
      INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
      INTERP_FIX_UV | INTERP_1COL,
      INTERP_Z | INTERP_FLOAT_UV | INTERP_1COL | OPT_FLOAT_UV_TO_FIX,
      INTERP_FIX_UV | INTERP_1COL,
      INTERP_Z | INTERP_FLOAT_UV | INTERP_1COL | OPT_FLOAT_UV_TO_FIX
   };

   int polytype_interp_tc[] = 
   {
      INTERP_FLAT,
      INTERP_3COL | COLOR_TO_RGB,
      INTERP_3COL,
      INTERP_FIX_UV,
      INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
      INTERP_FIX_UV,
      INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
      INTERP_FIX_UV | INTERP_1COL,
      INTERP_Z | INTERP_FLOAT_UV | INTERP_1COL | OPT_FLOAT_UV_TO_FIX,
      INTERP_FIX_UV | INTERP_1COL,
      INTERP_Z | INTERP_FLOAT_UV | INTERP_1COL | OPT_FLOAT_UV_TO_FIX
   };

   #ifdef ALLEGRO_COLOR8
   static POLYTYPE_INFO polytype_info8[] =
   {
      {  _poly_scanline_dummy,          NULL },
      {  _poly_scanline_gcol8,          NULL },
      {  _poly_scanline_grgb8,          NULL },
      {  _poly_scanline_atex8,          NULL },
      {  _poly_scanline_ptex8,          _poly_scanline_atex8 },
      {  _poly_scanline_atex_mask8,     NULL },
      {  _poly_scanline_ptex_mask8,     _poly_scanline_atex_mask8 },
      {  _poly_scanline_atex_lit8,      NULL },
      {  _poly_scanline_ptex_lit8,      _poly_scanline_atex_lit8 },
      {  _poly_scanline_atex_mask_lit8, NULL },
      {  _poly_scanline_ptex_mask_lit8, _poly_scanline_atex_mask_lit8 }
   };

   #ifdef ALLEGRO_MMX
   static POLYTYPE_INFO polytype_info8x[] =
   {
      {  NULL,                    NULL },
      {  NULL,                    NULL },
      {  _poly_scanline_grgb8x,   NULL },
      {  NULL,                    NULL },
      {  NULL,                    NULL },
      {  NULL,                    NULL },
      {  NULL,                    NULL },
      {  NULL,                    NULL },
      {  NULL,                    NULL },
      {  NULL,                    NULL },
      {  NULL,                    NULL }
   };

   static POLYTYPE_INFO polytype_info8d[] =
   {
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL },
      {  NULL,                           NULL }
   };
   #endif
   #endif

   #ifdef ALLEGRO_COLOR16
   static POLYTYPE_INFO polytype_info15[] =
   {
      {  _poly_scanline_dummy,           NULL },
      {  _poly_scanline_grgb15,          NULL },
      {  _poly_scanline_grgb15,          NULL },
      {  _poly_scanline_atex16,          NULL },
      {  _poly_scanline_ptex16,          _poly_scanline_atex16 },
      {  _poly_scanline_atex_mask15,     NULL },
      {  _poly_scanline_ptex_mask15,     _poly_scanline_atex_mask15 },
      {  _poly_scanline_atex_lit15,      NULL },
      {  _poly_scanline_ptex_lit15,      _poly_scanline_atex_lit15 },
      {  _poly_scanline_atex_mask_lit15, NULL },
      {  _poly_scanline_ptex_mask_lit15, _poly_scanline_atex_mask_lit15 }
   };

   #ifdef ALLEGRO_MMX
   static POLYTYPE_INFO polytype_info15x[] =
   {
      {  NULL,                            NULL },
      {  _poly_scanline_grgb15x,          NULL },
      {  _poly_scanline_grgb15x,          NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  _poly_scanline_atex_lit15x,      NULL },
      {  _poly_scanline_ptex_lit15x,      _poly_scanline_atex_lit15x },
      {  _poly_scanline_atex_mask_lit15x, NULL },
      {  _poly_scanline_ptex_mask_lit15x, _poly_scanline_atex_mask_lit15x }
   };

   static POLYTYPE_INFO polytype_info15d[] =
   {
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  _poly_scanline_ptex_lit15d,      _poly_scanline_atex_lit15x },
      {  NULL,                            NULL },
      {  _poly_scanline_ptex_mask_lit15d, _poly_scanline_atex_mask_lit15x }
   };
   #endif

   static POLYTYPE_INFO polytype_info16[] =
   {
      {  _poly_scanline_dummy,           NULL },
      {  _poly_scanline_grgb16,          NULL },
      {  _poly_scanline_grgb16,          NULL },
      {  _poly_scanline_atex16,          NULL },
      {  _poly_scanline_ptex16,          _poly_scanline_atex16 },
      {  _poly_scanline_atex_mask16,     NULL },
      {  _poly_scanline_ptex_mask16,     _poly_scanline_atex_mask16 },
      {  _poly_scanline_atex_lit16,      NULL },
      {  _poly_scanline_ptex_lit16,      _poly_scanline_atex_lit16 },
      {  _poly_scanline_atex_mask_lit16, NULL },
      {  _poly_scanline_ptex_mask_lit16, _poly_scanline_atex_mask_lit16 }
   };

   #ifdef ALLEGRO_MMX
   static POLYTYPE_INFO polytype_info16x[] =
   {
      {  NULL,                            NULL },
      {  _poly_scanline_grgb16x,          NULL },
      {  _poly_scanline_grgb16x,          NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  _poly_scanline_atex_lit16x,      NULL },
      {  _poly_scanline_ptex_lit16x,      _poly_scanline_atex_lit16x },
      {  _poly_scanline_atex_mask_lit16x, NULL },
      {  _poly_scanline_ptex_mask_lit16x, _poly_scanline_atex_mask_lit16x }
   };

   static POLYTYPE_INFO polytype_info16d[] =
   {
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  _poly_scanline_ptex_lit16d,      _poly_scanline_atex_lit16x },
      {  NULL,                            NULL },
      {  _poly_scanline_ptex_mask_lit16d, _poly_scanline_atex_mask_lit16x }
   };
   #endif
   #endif

   #ifdef ALLEGRO_COLOR24
   static POLYTYPE_INFO polytype_info24[] =
   {
      {  _poly_scanline_dummy,           NULL },
      {  _poly_scanline_grgb24,          NULL },
      {  _poly_scanline_grgb24,          NULL },
      {  _poly_scanline_atex24,          NULL },
      {  _poly_scanline_ptex24,          _poly_scanline_atex24 },
      {  _poly_scanline_atex_mask24,     NULL },
      {  _poly_scanline_ptex_mask24,     _poly_scanline_atex_mask24 },
      {  _poly_scanline_atex_lit24,      NULL },
      {  _poly_scanline_ptex_lit24,      _poly_scanline_atex_lit24 },
      {  _poly_scanline_atex_mask_lit24, NULL },
      {  _poly_scanline_ptex_mask_lit24, _poly_scanline_atex_mask_lit24 }
   };

   #ifdef ALLEGRO_MMX
   static POLYTYPE_INFO polytype_info24x[] =
   {
      {  NULL,                            NULL },
      {  _poly_scanline_grgb24x,          NULL },
      {  _poly_scanline_grgb24x,          NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  _poly_scanline_atex_lit24x,      NULL },
      {  _poly_scanline_ptex_lit24x,      _poly_scanline_atex_lit24x },
      {  _poly_scanline_atex_mask_lit24x, NULL },
      {  _poly_scanline_ptex_mask_lit24x, _poly_scanline_atex_mask_lit24x }
   };

   static POLYTYPE_INFO polytype_info24d[] =
   {
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  _poly_scanline_ptex_lit24d,      _poly_scanline_atex_lit24x },
      {  NULL,                            NULL },
      {  _poly_scanline_ptex_mask_lit24d, _poly_scanline_atex_mask_lit24x }
   };
   #endif
   #endif

   #ifdef ALLEGRO_COLOR32
   static POLYTYPE_INFO polytype_info32[] =
   {
      {  _poly_scanline_dummy,           NULL },
      {  _poly_scanline_grgb32,          NULL },
      {  _poly_scanline_grgb32,          NULL },
      {  _poly_scanline_atex32,          NULL },
      {  _poly_scanline_ptex32,          _poly_scanline_atex32 },
      {  _poly_scanline_atex_mask32,     NULL },
      {  _poly_scanline_ptex_mask32,     _poly_scanline_atex_mask32 },
      {  _poly_scanline_atex_lit32,      NULL },
      {  _poly_scanline_ptex_lit32,      _poly_scanline_atex_lit32 },
      {  _poly_scanline_atex_mask_lit32, NULL },
      {  _poly_scanline_ptex_mask_lit32, _poly_scanline_atex_mask_lit32 }
   };

   #ifdef ALLEGRO_MMX
   static POLYTYPE_INFO polytype_info32x[] =
   {
      {  NULL,                            NULL },
      {  _poly_scanline_grgb32x,          NULL },
      {  _poly_scanline_grgb32x,          NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  _poly_scanline_atex_lit32x,      NULL },
      {  _poly_scanline_ptex_lit32x,      _poly_scanline_atex_lit32x },
      {  _poly_scanline_atex_mask_lit32x, NULL },
      {  _poly_scanline_ptex_mask_lit32x, _poly_scanline_atex_mask_lit32x }
   };

   static POLYTYPE_INFO polytype_info32d[] =
   {
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  NULL,                            NULL },
      {  _poly_scanline_ptex_lit32d,      _poly_scanline_atex_lit32x },
      {  NULL,                            NULL },
      {  _poly_scanline_ptex_mask_lit32d, _poly_scanline_atex_mask_lit32x }
   };
   #endif
   #endif

   int *interpinfo;
   POLYTYPE_INFO *typeinfo;

   #ifdef ALLEGRO_MMX
   POLYTYPE_INFO *typeinfo_mmx, *typeinfo_3d;
   #endif

   switch (bitmap_color_depth(bmp)) {

      #ifdef ALLEGRO_COLOR8

	 case 8:
	    interpinfo = polytype_interp_pal;
	    typeinfo = polytype_info8;
	 #ifdef ALLEGRO_MMX
	    typeinfo_mmx = polytype_info8x;
	    typeinfo_3d = polytype_info8d;
	 #endif
	    break;

      #endif

      #ifdef ALLEGRO_COLOR16

	 case 15:
	    interpinfo = polytype_interp_tc;
	    typeinfo = polytype_info15;
	 #ifdef ALLEGRO_MMX
	    typeinfo_mmx = polytype_info15x;
	    typeinfo_3d = polytype_info15d;
	 #endif
	    break;

	 case 16:
	    interpinfo = polytype_interp_tc;
	    typeinfo = polytype_info16;
	 #ifdef ALLEGRO_MMX
	    typeinfo_mmx = polytype_info16x;
	    typeinfo_3d = polytype_info16d;
	 #endif
	    break;

      #endif

      #ifdef ALLEGRO_COLOR24

	 case 24:
	    interpinfo = polytype_interp_tc;
	    typeinfo = polytype_info24;
	 #ifdef ALLEGRO_MMX
	    typeinfo_mmx = polytype_info24x;
	    typeinfo_3d = polytype_info24d;
	 #endif
	    break;

      #endif

      #ifdef ALLEGRO_COLOR32

	 case 32:
	    interpinfo = polytype_interp_tc;
	    typeinfo = polytype_info32;
	 #ifdef ALLEGRO_MMX
	    typeinfo_mmx = polytype_info32x;
	    typeinfo_3d = polytype_info32d;
	 #endif
	    break;

      #endif

      default:
	 return NULL;
   }

   type = MID(0, type, POLYTYPE_MAX-1);
   *flags = interpinfo[type];

   if (texture) {
      info->texture = texture->line[0];
      info->umask = texture->w - 1;
      info->vmask = texture->h - 1;
      info->vshift = 0;
      while ((1 << info->vshift) < texture->w)
	 info->vshift++;
   }
   else {
      info->texture = NULL;
      info->umask = info->vmask = info->vshift = 0;
   }

   info->seg = bmp->seg;
   bmp_select(bmp);

   #ifdef ALLEGRO_MMX
   if ((cpu_mmx) && (typeinfo_mmx[type].filler)) {
      if ((cpu_3dnow) && (typeinfo_3d[type].filler)) {
	 _optim_alternative_drawer = typeinfo_3d[type].alternative;
	 return typeinfo_3d[type].filler;
      }
      _optim_alternative_drawer = typeinfo_mmx[type].alternative;
      return typeinfo_mmx[type].filler;
   }
   #endif

   _optim_alternative_drawer = typeinfo[type].alternative;

   return typeinfo[type].filler;
}



/* _clip_polygon_segment:
 *  Updates interpolation state values when skipping several places, eg.
 *  clipping the first part of a scanline.
 */
void _clip_polygon_segment(POLYGON_SEGMENT *info, int gap, int flags)
{
   if (flags & INTERP_1COL)
      info->c += info->dc * gap;

   if (flags & INTERP_3COL) {
      info->r += info->dr * gap;
      info->g += info->dg * gap;
      info->b += info->db * gap;
   }

   if (flags & INTERP_FIX_UV) {
      info->u += info->du * gap;
      info->v += info->dv * gap;
   }

   if (flags & INTERP_Z) {
      info->z += info->dz * gap;

      if (flags & INTERP_FLOAT_UV) {
	 info->fu += info->dfu * gap;
	 info->fv += info->dfv * gap;
      }
   }
}



/* draw_polygon_segment: 
 *  Polygon helper function to fill a scanline. Calculates deltas for 
 *  whichever values need interpolating, clips the segment, and then calls
 *  the lowlevel scanline filler.
 */
static void draw_polygon_segment(BITMAP *bmp, int ytop, int ybottom, POLYGON_EDGE *e1, POLYGON_EDGE *e2, SCANLINE_FILLER drawer, int flags, int color, POLYGON_SEGMENT *info)
{
   int x, y, w, gap;
   fixed step;
   POLYGON_SEGMENT *s1 = &(e1->dat);
   POLYGON_SEGMENT *s2 = &(e2->dat);
   AL_CONST SCANLINE_FILLER save_drawer = drawer;

   /* for each scanline in the polygon... */
   for (y=ytop; y<=ybottom; y++) {
      x = fceil(e1->x);
      w = fceil(e2->x) - x;
      step = (x << 16) - e1->x;
      drawer = save_drawer;

      if ((flags & INTERP_FLAT) && (drawer == _poly_scanline_dummy)) {
         if (w != 0)
	      hline(bmp, x, y, x+w-1, color);
      }
      else {
/*
 *  Nasty trick :
 *  In order to avoid divisions by zero, w is set to -1. This way s1 and s2
 *  are still being updated but the scanline is not drawn since w < 0.
 */
	 if (w == 0)
	    w = -1;
/*
 *  End of nasty trick.
 */

	 if (flags & INTERP_1COL) {
	    info->dc = (s2->c - s1->c) / w;
	    info->c = (_polygon_mode & POLY_SUB_ACCURACY ? s1->c + fmul(step, info->dc) : s1->c);
	    s1->c += s1->dc;
	    s2->c += s2->dc;
	 }

	 if (flags & INTERP_3COL) {
	    info->dr = (s2->r - s1->r) / w;
	    info->dg = (s2->g - s1->g) / w;
	    info->db = (s2->b - s1->b) / w;
	    info->r = s1->r;
	    info->g = s1->g;
	    info->b = s1->b;

	    if (_polygon_mode & POLY_SUB_ACCURACY) {
	       info->r += fmul(step, info->dr);
	       info->g += fmul(step, info->dg);
	       info->b += fmul(step, info->db);
	    }

	    s1->r += s1->dr;
	    s2->r += s2->dr;
	    s1->g += s1->dg;
	    s2->g += s2->dg;
	    s1->b += s1->db;
	    s2->b += s2->db;
	 }

	 if (flags & INTERP_FIX_UV) {
	    info->du = (s2->u - s1->u) / w;
	    info->dv = (s2->v - s1->v) / w;
	    info->u = s1->u;
	    info->v = s1->v;

	    if (_polygon_mode & POLY_SUB_ACCURACY) {
	       info->u += fmul(step, info->du);
	       info->v += fmul(step, info->dv);
	    }

	    s1->u += s1->du;
	    s2->u += s2->du;
	    s1->v += s1->dv;
	    s2->v += s2->dv;
	 }

	 if (flags & INTERP_Z) {
	    float step_f = fixtof(step);
	    float w1 = 1.0 / w;

	    info->dz = (s2->z - s1->z) * w1;
	    info->z = (_polygon_mode & POLY_SUB_ACCURACY ? s1->z + info->dz * step_f : s1->z);
	    s1->z += s1->dz;
	    s2->z += s2->dz;

	    if (flags & INTERP_FLOAT_UV) {
	       info->dfu = (s2->fu - s1->fu) * w1;
	       info->dfv = (s2->fv - s1->fv) * w1;
	       info->fu = s1->fu;
	       info->fv = s1->fv;

	       if (_polygon_mode & POLY_SUB_ACCURACY) {
		  info->fu += info->dfu * step_f;
		  info->fv += info->dfv * step_f;
	       }

	       s1->fu += s1->dfu;
	       s2->fu += s2->dfu;
	       s1->fv += s1->dfv;
	       s2->fv += s2->dfv;
	    }
	 }

	 if ((w > 0) && (x+w > bmp->cl) && (x < bmp->cr)) {
	    if (x < bmp->cl) {
	       gap = bmp->cl - x;
	       x = bmp->cl;
	       w -= gap;
	       _clip_polygon_segment(info, gap, flags);
	    }

	    if (x+w > bmp->cr)
	       w = bmp->cr - x;

	    if ((flags & OPT_FLOAT_UV_TO_FIX) && (info->dz == 0)) {
	       info->u = info->fu / info->z;
	       info->v = info->fv / info->z;
	       info->du = info->dfu / info->z;
	       info->dv = info->dfv / info->z;
	       drawer = _optim_alternative_drawer;
	    }

	    drawer(bmp_write_line(bmp, y) + x * BYTES_PER_PIXEL(bitmap_color_depth(bmp)), w, info);
	 }
      }

      e1->x += e1->dx;
      e2->x += e2->dx;
   }
}



/* do_polygon3d:
 *  Helper function for rendering 3d polygon, used by both the fixed point
 *  and floating point drawing functions.
 */
static void do_polygon3d(BITMAP *bmp, int top, int bottom, POLYGON_EDGE *left_edge, SCANLINE_FILLER drawer, int flags, int color, POLYGON_SEGMENT *info)
{
   int x, w, ytop, ybottom;
   #ifdef ALLEGRO_DOS
      int old87 = 0;
   #endif
   POLYGON_EDGE *right_edge;

   /* set fpu to single-precision, truncate mode */
   #ifdef ALLEGRO_DOS
      if (flags & (INTERP_Z | INTERP_FLOAT_UV))
	 old87 = _control87(PC_24 | RC_CHOP, MCW_PC | MCW_RC);
   #endif

   acquire_bitmap(bmp);

   if (left_edge->prev != left_edge->next) {
      if (left_edge->prev->top == top)
	 left_edge = left_edge->prev;
   }
   else {
      if ((left_edge->x + left_edge->dx) > (left_edge->next->x + left_edge->next->dx))
	 left_edge = left_edge->prev;
   }

   right_edge = left_edge->next;

   ytop = top;
   while (ytop <= bottom) {
      if (right_edge->bottom <= left_edge->bottom)
	 ybottom = right_edge->bottom;
      else
	 ybottom = left_edge->bottom;

      /* fill the scanline */
      draw_polygon_segment(bmp, ytop, ybottom, left_edge, right_edge, drawer, flags, color, info);

      /* update edges */
      if (ybottom >= left_edge->bottom)
	 left_edge = left_edge->prev;
      if (ybottom >= right_edge->bottom)
	 right_edge = right_edge->next;

      ytop = ybottom + 1;
   }

   bmp_unwrite_line(bmp);
   release_bitmap(bmp);

   /* reset fpu mode */
   #ifdef ALLEGRO_DOS
      if (flags & (INTERP_Z | INTERP_FLOAT_UV))
	 _control87(old87, MCW_PC | MCW_RC);
   #endif
}



/* polygon3d:
 *  Draws a 3d polygon in the specified mode. The vertices parameter should
 *  be followed by that many pointers to V3D structures, which describe each
 *  vertex of the polygon.
 */
void polygon3d(BITMAP *bmp, int type, BITMAP *texture, int vc, V3D *vtx[])
{
   int c;
   int flags;
   int top = INT_MAX;
   int bottom = INT_MIN;
   int v1y, v2y, test_normal;
   int cinit, cincr, cend;
   V3D *v1, *v2;
   POLYGON_EDGE *edge, *edge0, *start_edge;
   POLYGON_EDGE *list_edges = NULL;
   POLYGON_SEGMENT info;
   SCANLINE_FILLER drawer;

   if (vc < 3)
      return;

   /* set up the drawing mode */
   drawer = _get_scanline_filler(type, &flags, &info, texture, bmp);
   if (!drawer)
      return;

   /* allocate some space for the active edge table */
   _grow_scratch_mem(sizeof(POLYGON_EDGE) * vc);
   start_edge= edge0 = edge = (POLYGON_EDGE *)_scratch_mem;

   /* determine whether the vertices are given in clockwise order or not */

   test_normal = fixtoi(vtx[2]->y-vtx[1]->y)*fixtoi(vtx[1]->x-vtx[0]->x)-fixtoi(vtx[1]->y-vtx[0]->y)*fixtoi(vtx[2]->x-vtx[1]->x);

   if (test_normal < 0) {
      /* vertices are given in counterclockwise order */
      v2 = vtx[0];
      cinit = vc - 1;
      cincr = -1;
      cend = -1 ;
   }
   else {
      /* vertices are given in clockwise order */
      v2 = vtx[vc-1];
      cinit = 0;
      cincr = 1;
      cend = vc;
   }

   /* fill the double-linked list of edges in clockwise order */
   for (c=cinit; c!=cend; c+=cincr) {
      v1 = v2;
      v2 = vtx[c];
      v1y = fceil(v1->y);
      v2y = fceil(v2->y);

      if ((v1y != v2y) && 
	  ((v1y >= bmp->ct) || (v2y >= bmp->ct)) &&
	  ((v1y < bmp->cb) || (v2y < bmp->cb))) {

	 _fill_3d_edge_structure(edge, v1, v2, flags, bmp);

         if (edge->bottom >= edge->top) {
	    if (edge->top < top) {
	       top=edge->top;
	       start_edge = edge;
	    }

	    if (edge->bottom > bottom)
	       bottom = edge->bottom;

	    if (list_edges) {
	       list_edges->next = edge;
	       edge->prev = list_edges;
	    }

	    list_edges = edge;
	    edge++;
         }
      }
   }

   if (list_edges) {
      /* close the double-linked list */
      edge0->prev = --edge;
      edge->next = edge0;

      /* render the polygon */
      do_polygon3d(bmp, top, bottom, start_edge, drawer, flags, vtx[0]->c, &info);
   }
}



/* polygon3d_f:
 *  Floating point version of polygon3d().
 */
void polygon3d_f(BITMAP *bmp, int type, BITMAP *texture, int vc, V3D_f *vtx[])
{
   int c;
   int flags;
   int top = INT_MAX;
   int bottom = INT_MIN;
   int v1y, v2y;
   int cinit, cincr, cend;
   V3D_f *v1, *v2;
   POLYGON_EDGE *edge, *edge0, *start_edge;
   POLYGON_EDGE *list_edges = NULL;
   POLYGON_SEGMENT info;
   SCANLINE_FILLER drawer;

   if (vc < 3)
      return;

   /* set up the drawing mode */
   drawer = _get_scanline_filler(type, &flags, &info, texture, bmp);
   if (!drawer)
      return;

   /* allocate some space for the active edge table */
   _grow_scratch_mem(sizeof(POLYGON_EDGE) * vc);
   start_edge = edge0 = edge = (POLYGON_EDGE *)_scratch_mem;

   /* determine whether the vertices are given in clockwise order or not */

   if (polygon_z_normal_f(vtx[0], vtx[1], vtx[2]) < 0) {
      /* vertices are given in counterclockwise order */
      v2 = vtx[0];
      cinit = vc - 1;
      cincr = -1;
      cend = -1 ;
   }
   else {
      /* vertices are given in clockwise order */
      v2 = vtx[vc-1];
      cinit = 0;
      cincr = 1;
      cend = vc;
   }

   /* fill the double-linked list of edges in clockwise order */
   for (c=cinit; c!=cend; c+=cincr) {
      v1 = v2;
      v2 = vtx[c];
      v1y = fceil(ftofix(v1->y));
      v2y = fceil(ftofix(v2->y));

      if ((v1y != v2y) && 
	  ((v1y >= bmp->ct) || (v2y >= bmp->ct)) &&
	  ((v1y < bmp->cb) || (v2y < bmp->cb))) {

	 _fill_3d_edge_structure_f(edge, v1, v2, flags, bmp);

         if (edge->bottom >= edge->top) {
	    if (edge->top < top) {
	       top=edge->top;
	       start_edge = edge;
	    }

	    if (edge->bottom > bottom)
	       bottom = edge->bottom;

	    if (list_edges) {
	       list_edges->next = edge;
	       edge->prev = list_edges;
	    }

	    list_edges = edge;
	    edge++;
         }
      }
   }

   if (list_edges) {
      /* close the double-linked list */
      edge0->prev = --edge;
      edge->next = edge0;

      /* render the polygon */
      do_polygon3d(bmp, top, bottom, start_edge, drawer, flags, vtx[0]->c, &info);
   }
}



/* draw_triangle_part:
 *  Triangle helper function to fill a triangle part. Computes interpolation,
 *  clips the segment, and then calls the lowlevel scanline filler.
 */
static void draw_triangle_part(BITMAP *bmp, int ytop, int ybottom, POLYGON_EDGE *left_edge, POLYGON_EDGE *right_edge, SCANLINE_FILLER drawer, int flags, int color, POLYGON_SEGMENT *info)
{
   int x, y, w;
   int gap;
   AL_CONST int test_optim = (flags & OPT_FLOAT_UV_TO_FIX) && (info->dz == 0);
   fixed step;
   POLYGON_SEGMENT *s1 = &(left_edge->dat);

   for (y=ytop; y<=ybottom; y++) {
      x = fceil(left_edge->x);
      w = fceil(right_edge->x) - x;
      step = (x << 16) - left_edge->x;

      if (flags & INTERP_FLAT)
	 hline(bmp, x, y, x+w, color);

      else {
	 if (flags & INTERP_1COL)
	    info->c = (_polygon_mode & POLY_SUB_ACCURACY ? s1->c + fmul(step, info->dc) : s1->c);
         s1->c += s1->dc;

	 if (flags & INTERP_3COL) {
	    info->r = s1->r;
	    info->g = s1->g;
	    info->b = s1->b;

	    if (_polygon_mode & POLY_SUB_ACCURACY) {
	       info->r += fmul(step, info->dr);
	       info->g += fmul(step, info->dg);
	       info->b += fmul(step, info->db);
	    }

	    s1->r += s1->dr;
	    s1->g += s1->dg;
	    s1->b += s1->db;
	 }

	 if (flags & INTERP_FIX_UV) {
	    info->u = s1->u;
	    info->v = s1->v;

	    if (_polygon_mode & POLY_SUB_ACCURACY) {
	       info->u += fmul(step, info->du);
	       info->v += fmul(step, info->dv);
	    }

	    s1->u += s1->du;
	    s1->v += s1->dv;
	 }

	 if (flags & INTERP_Z) {
	    float step_f = fixtof(step);

	    info->z = (_polygon_mode & POLY_SUB_ACCURACY ? s1->z + info->dz * step_f : s1->z);
	    s1->z += s1->dz;

	    if (flags & INTERP_FLOAT_UV) {
	       info->fu = s1->fu;
	       info->fv = s1->fv;

	       if (_polygon_mode & POLY_SUB_ACCURACY) {
		  info->fu += info->dfu * step_f;
		  info->fv += info->dfv * step_f;
	       }

	       s1->fu += s1->dfu;
	       s1->fv += s1->dfv;
	    }
	 }

	 if ((w > 0) && (x+w > bmp->cl) && (x < bmp->cr)) {
	    if (x < bmp->cl) {
	       gap = bmp->cl - x;
	       x = bmp->cl;
	       w -= gap;
	       _clip_polygon_segment(info, gap, flags);
	    }

	    if (x+w > bmp->cr)
	       w = bmp->cr - x;

	    if (test_optim) {
	       info->u = info->fu / info->z;
	       info->v = info->fv / info->z;
	       info->du = info->dfu / info->z;
	       info->dv = info->dfv / info->z;
	       drawer = _optim_alternative_drawer;
	    }

	    drawer(bmp_write_line(bmp, y) + x * BYTES_PER_PIXEL(bitmap_color_depth(bmp)), w, info);
	 }
      }

      left_edge->x += left_edge->dx;
      right_edge->x += right_edge->dx;
   }
}



/* _triangle_deltas
 * Triangle3d helper function to calculate the deltas. (For triangles,
 * deltas are constant over the whole triangle).
 */
static void _triangle_deltas(BITMAP *bmp, fixed w, POLYGON_SEGMENT *s1, POLYGON_SEGMENT *info, AL_CONST V3D *v, int flags)
{
   if (flags & INTERP_1COL)
      info->dc = fdiv(s1->c - itofix(v->c), w);

   if (flags & INTERP_3COL) {
      int r, g, b;

      if (flags & COLOR_TO_RGB) {
	 int coldepth = bitmap_color_depth(bmp);
	 r = getr_depth(coldepth, v->c);
	 g = getg_depth(coldepth, v->c);
	 b = getb_depth(coldepth, v->c);
      }
      else {
	 r = (v->c >> 16) & 0xFF;
	 g = (v->c >> 8) & 0xFF;
	 b = v->c & 0xFF;
      }

      info->dr = fdiv(s1->r - itofix(r), w);
      info->dg = fdiv(s1->g - itofix(g), w);
      info->db = fdiv(s1->b - itofix(b), w);
   }

   if (flags & INTERP_FIX_UV) {
      info->du = fdiv(s1->u - v->u, w);
      info->dv = fdiv(s1->v - v->v, w);
   }

   if (flags & INTERP_Z) {
      float w1 = 65536. / w;

      /* Z (depth) interpolation */
      float z1 = 65536. / v->z;

      info->dz = (s1->z - z1) * w1;

      if (flags & INTERP_FLOAT_UV) {
	 /* floating point (perspective correct) texture interpolation */
	 float fu1 = v->u * z1;
	 float fv1 = v->v * z1;

	 info->dfu = (s1->fu - fu1) * w1;
	 info->dfv = (s1->fv - fv1) * w1;
      }
   }
}


/* _triangle_deltas_f
 * Floating point version of _triangle_deltas().
 */
static void _triangle_deltas_f(BITMAP *bmp, fixed w, POLYGON_SEGMENT *s1, POLYGON_SEGMENT *info, AL_CONST V3D_f *v, int flags)
{
   if (flags & INTERP_1COL)
      info->dc = fdiv(s1->c - itofix(v->c), w);

   if (flags & INTERP_3COL) {
      int r, g, b;

      if (flags & COLOR_TO_RGB) {
	 int coldepth = bitmap_color_depth(bmp);
	 r = getr_depth(coldepth, v->c);
	 g = getg_depth(coldepth, v->c);
	 b = getb_depth(coldepth, v->c);
      }
      else {
	 r = (v->c >> 16) & 0xFF;
	 g = (v->c >> 8) & 0xFF;
	 b = v->c & 0xFF;
      }

      info->dr = fdiv(s1->r - itofix(r), w);
      info->dg = fdiv(s1->g - itofix(g), w);
      info->db = fdiv(s1->b - itofix(b), w);
   }

   if (flags & INTERP_FIX_UV) {
      info->du = fdiv(s1->u - ftofix(v->u), w);
      info->dv = fdiv(s1->v - ftofix(v->v), w);
   }

   if (flags & INTERP_Z) {
      float w1 = 65536. / w;

      /* Z (depth) interpolation */
      float z1 = 1.0 / v->z;

      info->dz = (s1->z - z1) * w1;

      if (flags & INTERP_FLOAT_UV) {
	 /* floating point (perspective correct) texture interpolation */
	 float fu1 = v->u * z1 * 65536.;
	 float fv1 = v->v * z1 * 65536.;

	 info->dfu = (s1->fu - fu1) * w1;
	 info->dfv = (s1->fv - fv1) * w1;
      }
   }
}


/* _clip_polygon_segment_fixed:
 *  Fixed point version of _clip_polygon_segment().
 */
static void _clip_polygon_segment_fixed(POLYGON_SEGMENT *info, fixed gap, int flags)
{
   if (flags & INTERP_1COL)
      info->c += fmul(info->dc, gap);

   if (flags & INTERP_3COL) {
      info->r += fmul(info->dr, gap);
      info->g += fmul(info->dg, gap);
      info->b += fmul(info->db, gap);
   }

   if (flags & INTERP_FIX_UV) {
      info->u += fmul(info->du, gap);
      info->v += fmul(info->dv, gap);
   }

   if (flags & INTERP_Z) {
      float gap_f = fixtof(gap);

      info->z += info->dz * gap_f;

      if (flags & INTERP_FLOAT_UV) {
	 info->fu += info->dfu * gap_f;
	 info->fv += info->dfv * gap_f;
      }
   }
}



/* triangle3d:
 *  Draws a 3d triangle.
 */
void triangle3d(BITMAP *bmp, int type, BITMAP *texture, V3D *v1, V3D *v2, V3D *v3)
{
   int flags;

   #ifdef ALLEGRO_DOS
      int old87 = 0;
   #endif

   int color = v1->c;
   int test_normal;
   int y1, y2, y3;
   V3D *vt1, *vt2, *vt3;
   POLYGON_EDGE edge1, edge2;
   POLYGON_EDGE *left_edge, *right_edge;
   POLYGON_SEGMENT info;
   SCANLINE_FILLER drawer;

   left_edge = right_edge = NULL;

   /* set up the drawing mode */
   drawer = _get_scanline_filler(type, &flags, &info, texture, bmp);
   if (!drawer)
      return;

   /* sort the vertices so that vt1->y <= vt2->y <= vt3->y */
   if (v1->y > v2->y) {
      vt1 = v2;
      vt2 = v1;
   }
   else {
      vt1 = v1;
      vt2 = v2;
   }

   if (vt1->y > v3->y) {
      vt3 = vt1;
      vt1 = v3;
   }
   else
      vt3 = v3;

   if (vt2->y > vt3->y) {
      V3D* vtemp = vt2;
      vt2 = vt3;
      vt3 = vtemp;
   }

   y1 = fceil(vt1->y);
   y2 = fceil(vt2->y);
   y3 = fceil(vt3->y);

   /* do 3D triangle*/
   if ((y1 < bmp->cb) && (y3 >= bmp->ct) && (y1 != y3)) {
      /* set fpu to single-precision, truncate mode */
      #ifdef ALLEGRO_DOS
	 if (flags & (INTERP_Z | INTERP_FLOAT_UV))
	    old87 = _control87(PC_24 | RC_CHOP, MCW_PC | MCW_RC);
      #endif

      acquire_bitmap(bmp);

      _fill_3d_edge_structure(&edge1, vt1, vt3, flags, bmp);

      test_normal = (y3-y2)*fixtoi(vt2->x-vt1->x)-(y2-y1)*fixtoi(vt3->x-vt2->x);

      if (test_normal < 0) {
	 left_edge = &edge2;
	 right_edge = &edge1;
      }
      else {
	 left_edge = &edge1;
	 right_edge = &edge2;
      }

      /* calculate deltas */
      if (!(flags & INTERP_FLAT)) {
	 fixed w, h;
	 POLYGON_SEGMENT s1 = edge1.dat;

	 h = vt2->y - (edge1.top << 16);
	 _clip_polygon_segment_fixed(&s1, h, flags);

	 w = edge1.x + fmul(h, edge1.dx) - vt2->x;
	 if (w != 0) _triangle_deltas(bmp, w, &s1, &info, vt2, flags);
      }

      /* draws part between y1 and y2 */
      if ((y1 != y2) && (y2 >= bmp->ct)) {
	 _fill_3d_edge_structure(&edge2, vt1, vt2, flags, bmp);

	 draw_triangle_part(bmp, edge1.top, edge2.bottom, left_edge, right_edge, drawer, flags, color, &info);
      }

      /* draws part between y2 and y3 */
      if ((y2 != y3) && (y2 < bmp->cb)) {
	 _fill_3d_edge_structure(&edge2, vt2, vt3, flags, bmp);

	 draw_triangle_part(bmp, edge2.top, edge1.bottom, left_edge, right_edge, drawer, flags, color, &info);
      }

      bmp_unwrite_line(bmp);
      release_bitmap(bmp);

      /* reset fpu mode */
      #ifdef ALLEGRO_DOS
	 if (flags & (INTERP_Z | INTERP_FLOAT_UV))
	    _control87(old87, MCW_PC | MCW_RC);
      #endif
   }
}



/* triangle3d_f:
 *  Draws a 3d triangle.
 */
void triangle3d_f(BITMAP *bmp, int type, BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3)
{
   int flags;

   #ifdef ALLEGRO_DOS
      int old87 = 0;
   #endif

   int color = v1->c;
   int y1, y2, y3;
   V3D_f *vt1, *vt2, *vt3;
   POLYGON_EDGE edge1, edge2;
   POLYGON_EDGE *left_edge, *right_edge;
   POLYGON_SEGMENT info;
   SCANLINE_FILLER drawer;

   left_edge = right_edge = NULL;

   /* set up the drawing mode */
   drawer = _get_scanline_filler(type, &flags, &info, texture, bmp);
   if (!drawer)
      return;

   /* sort the vertices so that vt1->y <= vt2->y <= vt3->y */
   if (v1->y > v2->y) {
      vt1 = v2;
      vt2 = v1;
   }
   else {
      vt1 = v1;
      vt2 = v2;
   }

   if (vt1->y > v3->y) {
      vt3 = vt1;
      vt1 = v3;
   }
   else
      vt3 = v3;

   if (vt2->y > vt3->y) {
      V3D_f* vtemp = vt2;
      vt2 = vt3;
      vt3 = vtemp;
   }

   y1 = fceil(ftofix(vt1->y));
   y2 = fceil(ftofix(vt2->y));
   y3 = fceil(ftofix(vt3->y));

   /* do 3D triangle*/
   if ((y1 < bmp->cb) && (y3 >= bmp->ct) && (y1 != y3)) {
      /* set fpu to single-precision, truncate mode */
      #ifdef ALLEGRO_DOS
	 if (flags & (INTERP_Z | INTERP_FLOAT_UV))
	    old87 = _control87(PC_24 | RC_CHOP, MCW_PC | MCW_RC);
      #endif

      acquire_bitmap(bmp);

      _fill_3d_edge_structure_f(&edge1, vt1, vt3, flags, bmp);

      /* calculate deltas */
      if (!(flags & INTERP_FLAT)) {
	 fixed w, h;
	 POLYGON_SEGMENT s1 = edge1.dat;

	 h = ftofix(vt2->y) - (edge1.top << 16);
	 _clip_polygon_segment_fixed(&s1, h, flags);

	 w = edge1.x + fmul(h, edge1.dx) - ftofix(vt2->x);
	 if (w != 0) _triangle_deltas_f(bmp, w, &s1, &info, vt2, flags);
      }

      if (polygon_z_normal_f(vt1, vt2, vt3) < 0) {
	 left_edge = &edge2;
	 right_edge = &edge1;
      }
      else {
	 left_edge = &edge1;
	 right_edge = &edge2;
      }

      /* draws part between y1 and y2 */
      if ((y1 != y2) && (y2 >= bmp->ct)) {
	 _fill_3d_edge_structure_f(&edge2, vt1, vt2, flags, bmp);

	 draw_triangle_part(bmp, edge1.top, edge2.bottom, left_edge, right_edge, drawer, flags, color, &info);
      }

      /* draws part between y2 and y3 */
      if ((y2 != y3) && (y2 < bmp->cb)) {
	 _fill_3d_edge_structure_f(&edge2, vt2, vt3, flags, bmp);

	 draw_triangle_part(bmp, edge2.top, edge1.bottom, left_edge, right_edge, drawer, flags, color, &info);
      }

      bmp_unwrite_line(bmp);
      release_bitmap(bmp);

      /* reset fpu mode */
      #ifdef ALLEGRO_DOS
	 if (flags & (INTERP_Z | INTERP_FLOAT_UV))
	    _control87(old87, MCW_PC | MCW_RC);
      #endif
   }
}



/* quad3d:
 *  Draws a 3d quad.
 */
void quad3d(BITMAP *bmp, int type, BITMAP *texture, V3D *v1, V3D *v2, V3D *v3, V3D *v4)
{
   #if (defined ALLEGRO_GCC) && (defined ALLEGRO_I386)

      /* dodgy assumption alert! See comments for triangle() */
      polygon3d(bmp, type, texture, 4, &v1);

   #else

      V3D *vertex[4];

      vertex[0] = v1;
      vertex[1] = v2;
      vertex[2] = v3;
      vertex[3] = v4;
      polygon3d(bmp, type, texture, 4, vertex);

   #endif
}



/* quad3d_f:
 *  Draws a 3d quad.
 */
void quad3d_f(BITMAP *bmp, int type, BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3, V3D_f *v4)
{
   #if (defined ALLEGRO_GCC) && (defined ALLEGRO_I386)

      /* dodgy assumption alert! See comments for triangle() */
      polygon3d_f(bmp, type, texture, 4, &v1);

   #else

      V3D_f *vertex[4];

      vertex[0] = v1;
      vertex[1] = v2;
      vertex[2] = v3;
      vertex[3] = v4;
      polygon3d_f(bmp, type, texture, 4, vertex);

   #endif
}

