#ifndef __al_included_allegro5_aintern_mouse_h
#define __al_included_allegro5_aintern_mouse_h

#include "allegro5/internal/aintern_driver.h"
#include "allegro5/internal/aintern_events.h"

#ifdef __cplusplus
   extern "C" {
#endif



typedef struct ALLEGRO_MOUSE_FLOAT_STATE
{
   /* Like ALLEGRO_MOUSE_STATE, but using floats. */
   float x;
   float y;
   float z;
   float w;
   float more_axes[ALLEGRO_MOUSE_MAX_EXTRA_AXES];
   int buttons;
   float pressure;
   struct ALLEGRO_DISPLAY *display;
} ALLEGRO_MOUSE_FLOAT_STATE;


typedef struct ALLEGRO_MOUSE_DRIVER
{
   int  msedrv_id;
   const char *msedrv_name;
   const char *msedrv_desc;
   const char *msedrv_ascii_name;
   AL_METHOD(bool, init_mouse, (void));
   AL_METHOD(void, exit_mouse, (void));
   AL_METHOD(ALLEGRO_MOUSE*, get_mouse, (void));
   AL_METHOD(unsigned int, get_mouse_num_buttons, (void));
   AL_METHOD(unsigned int, get_mouse_num_axes, (void));
   AL_METHOD(bool, set_mouse_xy, (ALLEGRO_DISPLAY *display, float x, float y));
   AL_METHOD(bool, set_mouse_axis, (int which, float value));
   AL_METHOD(void, get_mouse_state, (ALLEGRO_MOUSE_FLOAT_STATE *ret_state));
} ALLEGRO_MOUSE_DRIVER;


extern _AL_DRIVER_INFO _al_mouse_driver_list[];


struct ALLEGRO_MOUSE
{
   ALLEGRO_EVENT_SOURCE es;
};

extern void _al_make_int_mouse_event(ALLEGRO_EVENT *event);


#ifdef __cplusplus
   }
#endif

#endif

/* vi ts=8 sts=3 sw=3 et */
