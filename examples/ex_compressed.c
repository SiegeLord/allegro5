#include <stdio.h>
#include <stdlib.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_font.h"

#include "common.c"

typedef struct BITMAP_TYPE {
    ALLEGRO_BITMAP* bmp;
    ALLEGRO_BITMAP* clone;
    ALLEGRO_BITMAP* decomp;
    ALLEGRO_PIXEL_FORMAT format;
    const char* name;
} BITMAP_TYPE;

int main(int argc, char **argv)
{
   const char *filename;
   ALLEGRO_DISPLAY *display;
   ALLEGRO_TIMER *timer;
   ALLEGRO_EVENT_QUEUE *queue;
   ALLEGRO_FONT *font;
   ALLEGRO_BITMAP *bkg;
   bool redraw = true;
   int ii;
   int cur_bitmap = 0;
   #define NUM_BITMAPS 4
   BITMAP_TYPE bitmaps[NUM_BITMAPS] = {
      {NULL, NULL, NULL, ALLEGRO_PIXEL_FORMAT_RGBA_DXT1, "DXT1"},
      {NULL, NULL, NULL, ALLEGRO_PIXEL_FORMAT_RGBA_DXT3, "DXT3"},
      {NULL, NULL, NULL, ALLEGRO_PIXEL_FORMAT_RGBA_DXT5, "DXT5"},
      {NULL, NULL, NULL, ALLEGRO_PIXEL_FORMAT_ANY,       "Uncompressed"},
   };

   if (argc > 1) {
      filename = argv[1];
   }
   else {
      filename = "data/mysha.pcx";
   }

   if (!al_init()) {
      abort_example("Could not init Allegro.\n");
   }

   open_log();
    
   if (argc > 2) {
      al_set_new_display_adapter(atoi(argv[2]));
   }

   al_init_image_addon();
   al_init_font_addon();
   al_install_keyboard();

   display = al_create_display(640, 480);
   if (!display) {
      abort_example("Error creating display\n");
   }

   for (ii = 0; ii < NUM_BITMAPS; ii++) {
      double t0, t1;
      al_set_new_bitmap_format(bitmaps[ii].format);
      
      t0 = al_get_time();
      bitmaps[ii].bmp = al_load_bitmap(filename);
      t1 = al_get_time();
      
      if (!bitmaps[ii].bmp) {
         abort_example("%s not found or failed to load\n", filename);
      }
      log_printf("%s load time: %f sec\n", bitmaps[ii].name, t1 - t0);

      t0 = al_get_time();
      bitmaps[ii].clone = al_clone_bitmap(bitmaps[ii].bmp);
      t1 = al_get_time();

      if (!bitmaps[ii].clone) {
         abort_example("Couldn't clone %s\n", bitmaps[ii].name);
      }
      log_printf("%s clone time: %f sec\n", bitmaps[ii].name, t1 - t0);

      al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY);
      t0 = al_get_time();
      bitmaps[ii].decomp = al_clone_bitmap(bitmaps[ii].bmp);
      t1 = al_get_time();

      if (!bitmaps[ii].decomp) {
         abort_example("Couldn't decompress %s\n", bitmaps[ii].name);
      }
      log_printf("%s decompress time: %f sec\n", bitmaps[ii].name, t1 - t0);
   }

   bkg = al_load_bitmap("data/bkg.png");
   if (!bkg) {
      abort_example("data/bkg.png not found or failed to load\n");
   }

   al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY);
   font = al_create_builtin_font();
   timer = al_create_timer(1.0 / 30);
   queue = al_create_event_queue();
   al_register_event_source(queue, al_get_display_event_source(display));
   al_register_event_source(queue, al_get_keyboard_event_source());
   al_register_event_source(queue, al_get_timer_event_source(timer));
   al_start_timer(timer);

   while (1) {
      ALLEGRO_EVENT event;
      al_wait_for_event(queue, &event);
      switch (event.type) {
         case ALLEGRO_EVENT_DISPLAY_CLOSE:
            goto EXIT;
         case ALLEGRO_EVENT_TIMER:
            redraw = true;
            break;
         case ALLEGRO_EVENT_KEY_DOWN:
            switch (event.keyboard.keycode) {
               case ALLEGRO_KEY_LEFT:
                  cur_bitmap = (cur_bitmap - 1 + NUM_BITMAPS) % NUM_BITMAPS;
                  break;
               case ALLEGRO_KEY_RIGHT:
                  cur_bitmap = (cur_bitmap + 1) % NUM_BITMAPS;
                  break;
               case ALLEGRO_KEY_ESCAPE:
                  goto EXIT;
            }
            break;
      }
      if (redraw && al_is_event_queue_empty(queue)) {
         redraw = false;
         al_clear_to_color(al_map_rgb_f(0, 0, 0));
         al_draw_bitmap(bkg, 0, 0, 0);
         al_draw_textf(font, al_map_rgb_f(1, 1, 1), 5, 5, ALLEGRO_ALIGN_LEFT, "Format: %s", bitmaps[cur_bitmap].name);
         al_draw_bitmap(bitmaps[cur_bitmap].bmp, 0, 20, 0);
         al_draw_bitmap(bitmaps[cur_bitmap].clone, al_get_bitmap_width(bitmaps[cur_bitmap].bmp), 20, 0);
         al_draw_bitmap(bitmaps[cur_bitmap].decomp, 0, 20 + al_get_bitmap_height(bitmaps[cur_bitmap].bmp), 0);
         al_flip_display();
      }
   }
EXIT:

   al_destroy_bitmap(bkg);
   for (ii = 0; ii < NUM_BITMAPS; ii++) {
      al_destroy_bitmap(bitmaps[ii].bmp);
   }

   close_log(false);

   return 0;
}

/* vim: set sts=4 sw=4 et: */
