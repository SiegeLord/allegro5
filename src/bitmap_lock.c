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
 *      Bitmap locking routines.
 *
 *      See LICENSE.txt for copyright information.
 */


#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_pixels.h"


/* Function: al_lock_bitmap_region
 */
ALLEGRO_LOCKED_REGION *al_lock_bitmap_region(ALLEGRO_BITMAP *bitmap,
   int x, int y, int width, int height, int format, int flags)
{
   ALLEGRO_LOCKED_REGION *lr;
   int bitmap_format = al_get_bitmap_format(bitmap);
   int bitmap_flags = al_get_bitmap_flags(bitmap);
   int block_width = al_get_pixel_block_width(bitmap_format);
   int xc, yc, wc, hc;
   ASSERT(x >= 0);
   ASSERT(y >= 0);
   ASSERT(width >= 0);
   ASSERT(height >= 0);
   ASSERT(!_al_pixel_format_is_compressed(format));

   /* For sub-bitmaps */
   if (bitmap->parent) {
      x += bitmap->xofs;
      y += bitmap->yofs;
      bitmap = bitmap->parent;
   }

   if (bitmap->locked)
      return NULL;

   if (!(bitmap_flags & ALLEGRO_MEMORY_BITMAP) &&
         !(flags & ALLEGRO_LOCK_READONLY))
      bitmap->dirty = true;

   ASSERT(x+width <= bitmap->w);
   ASSERT(y+height <= bitmap->h);

   xc = (x / block_width) * block_width;
   yc = (y / block_width) * block_width;
   wc = _al_get_least_multiple(x + width, block_width) - xc;
   hc = _al_get_least_multiple(y + height, block_width) - yc;

   bitmap->lock_x = xc;
   bitmap->lock_y = yc;
   bitmap->lock_w = wc;
   bitmap->lock_h = hc;
   bitmap->lock_flags = flags;

   if (flags == ALLEGRO_LOCK_WRITEONLY
         && (xc != x || yc != y || wc != width || hc != height)) {
      /* Unaligned write-only access requires that we fill in the padding
       * from the texture.
       * XXX: In principle, this could be done more efficiently. */
      flags = ALLEGRO_LOCK_READWRITE;
   }

   if (bitmap_flags & ALLEGRO_MEMORY_BITMAP) {
      int f = _al_get_real_pixel_format(al_get_current_display(), format);
      if (f < 0) {
         return NULL;
      }
      ASSERT(bitmap->memory);
      if (format == ALLEGRO_PIXEL_FORMAT_ANY || bitmap_format == format || bitmap_format == f) {
         bitmap->locked_region.data = bitmap->memory
            + bitmap->pitch * yc + xc * al_get_pixel_size(bitmap_format);
         bitmap->locked_region.format = bitmap_format;
         bitmap->locked_region.pitch = bitmap->pitch;
         bitmap->locked_region.pixel_size = al_get_pixel_size(bitmap_format);
      }
      else {
         bitmap->locked_region.pitch = al_get_pixel_size(f) * wc;
         bitmap->locked_region.data = al_malloc(bitmap->locked_region.pitch*hc);
         bitmap->locked_region.format = f;
         bitmap->locked_region.pixel_size = al_get_pixel_size(f);
         if (!(bitmap->lock_flags & ALLEGRO_LOCK_WRITEONLY)) {
            _al_convert_bitmap_data(
               bitmap->memory, bitmap_format, bitmap->pitch,
               bitmap->locked_region.data, f, bitmap->locked_region.pitch,
               xc, yc, 0, 0, wc, hc);
         }
      }
      lr = &bitmap->locked_region;
   }
   else {
      lr = bitmap->vt->lock_region(bitmap, xc, yc, wc, hc, format, flags);
      if (!lr) {
         return NULL;
      }
   }

   /* Fixup the data pointer for unaligned access */
   lr->data = (char*)lr->data + (xc - x) * lr->pixel_size + (yc - y) * lr->pitch;

   bitmap->locked = true;

   return lr;
}


/* Function: al_lock_bitmap
 */
ALLEGRO_LOCKED_REGION *al_lock_bitmap(ALLEGRO_BITMAP *bitmap,
   int format, int flags)
{
   return al_lock_bitmap_region(bitmap, 0, 0, bitmap->w, bitmap->h, format, flags);
}


/* Function: al_unlock_bitmap
 */
void al_unlock_bitmap(ALLEGRO_BITMAP *bitmap)
{
   int bitmap_format = al_get_bitmap_format(bitmap);
   /* For sub-bitmaps */
   if (bitmap->parent) {
      bitmap = bitmap->parent;
   }

   if (!(al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP)) {
      bitmap->vt->unlock_region(bitmap);
   }
   else {
      if (bitmap->locked_region.format != 0 && bitmap->locked_region.format != bitmap_format) {
         if (!(bitmap->lock_flags & ALLEGRO_LOCK_READONLY)) {
            _al_convert_bitmap_data(
               bitmap->locked_region.data, bitmap->locked_region.format, bitmap->locked_region.pitch,
               bitmap->memory, bitmap_format, bitmap->pitch,
               0, 0, bitmap->lock_x, bitmap->lock_y, bitmap->lock_w, bitmap->lock_h);
         }
         al_free(bitmap->locked_region.data);
      }
   }

   bitmap->locked = false;
}


/* Function: al_is_bitmap_locked
 */
bool al_is_bitmap_locked(ALLEGRO_BITMAP *bitmap)
{
   return bitmap->locked;
}


/* vim: set ts=8 sts=3 sw=3 et: */
