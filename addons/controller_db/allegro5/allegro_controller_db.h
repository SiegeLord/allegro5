#ifndef __al_included_allegro5_allegro_controller_db_h
#define __al_included_allegro5_allegro_controller_db_h

#if (defined ALLEGRO_MINGW32) || (defined ALLEGRO_MSVC) || (defined ALLEGRO_BCC32)
	#ifndef ALLEGRO_STATICLINK
		#ifdef ALLEGRO_CONTROLLER_DB_SRC
			#define _ALLEGRO_CONTROLLER_DB_DLL __declspec(dllexport)
		#else
			#define _ALLEGRO_CONTROLLER_DB_DLL __declspec(dllimport)
		#endif
	#else
		#define _ALLEGRO_CONTROLLER_DB_DLL
	#endif
#endif

#if defined ALLEGRO_MSVC
	#define ALLEGRO_CONTROLLER_DB_FUNC(type, name, args)      _ALLEGRO_CONTROLLER_DB_DLL type __cdecl name args
#elif defined ALLEGRO_MINGW32
	#define ALLEGRO_CONTROLLER_DB_FUNC(type, name, args)      extern type name args
#elif defined ALLEGRO_BCC32
	#define ALLEGRO_CONTROLLER_DB_FUNC(type, name, args)      extern _ALLEGRO_CONTROLLER_DLL type name args
#else
	#define ALLEGRO_CONTROLLER_DB_FUNC      AL_FUNC
#endif  

#ifdef __cplusplus
extern "C" {
#endif

#include "allegro5/allegro.h"

/* Enum: ALLEGRO_CONTROLLER_AXIS
 */
enum ALLEGRO_CONTROLLER_AXIS
{
    ALLEGRO_CONTROLLER_AXIS_INVALID = -1,
    ALLEGRO_CONTROLLER_AXIS_LEFTX,
    ALLEGRO_CONTROLLER_AXIS_LEFTY,
    ALLEGRO_CONTROLLER_AXIS_RIGHTX,
    ALLEGRO_CONTROLLER_AXIS_RIGHTY,
    ALLEGRO_CONTROLLER_AXIS_TRIGGERLEFT,
    ALLEGRO_CONTROLLER_AXIS_TRIGGERRIGHT,
	ALLEGRO_CONTROLLER_AXIS_DPADX,
	ALLEGRO_CONTROLLER_AXIS_DPADY,
    ALLEGRO_CONTROLLER_AXIS_MAX
};

/* Enum: ALLEGRO_CONTROLLER_BUTTON
 */
enum ALLEGRO_CONTROLLER_BUTTON
{
    ALLEGRO_CONTROLLER_BUTTON_INVALID = -1,
    ALLEGRO_CONTROLLER_BUTTON_A,
    ALLEGRO_CONTROLLER_BUTTON_B,
    ALLEGRO_CONTROLLER_BUTTON_X,
    ALLEGRO_CONTROLLER_BUTTON_Y,
    ALLEGRO_CONTROLLER_BUTTON_BACK,
    ALLEGRO_CONTROLLER_BUTTON_GUIDE,
    ALLEGRO_CONTROLLER_BUTTON_START,
    ALLEGRO_CONTROLLER_BUTTON_LEFTSTICK,
    ALLEGRO_CONTROLLER_BUTTON_RIGHTSTICK,
    ALLEGRO_CONTROLLER_BUTTON_LEFTSHOULDER,
    ALLEGRO_CONTROLLER_BUTTON_RIGHTSHOULDER,
    ALLEGRO_CONTROLLER_BUTTON_MAX
};

#ifndef __cplusplus
typedef enum ALLEGRO_CONTROLLER_AXIS ALLEGRO_CONTROLLER_AXIS;
typedef enum ALLEGRO_CONTROLLER_BUTTON ALLEGRO_CONTROLLER_BUTTON;
#endif

ALLEGRO_CONTROLLER_DB_FUNC(void, al_controller_db_init, (void));  
ALLEGRO_CONTROLLER_DB_FUNC(int, al_register_controller_mapping, (const char *controller_string));  
ALLEGRO_CONTROLLER_DB_FUNC(int, al_load_controller_mapping, (const char *filename));  
ALLEGRO_CONTROLLER_DB_FUNC(bool, al_has_mapping_for_joystick, ( ALLEGRO_JOYSTICK *joystick));
ALLEGRO_CONTROLLER_DB_FUNC(ALLEGRO_CONTROLLER_BUTTON, al_get_controller_button_mapping, ( ALLEGRO_JOYSTICK *joystick, int button));
ALLEGRO_CONTROLLER_DB_FUNC(ALLEGRO_CONTROLLER_AXIS, al_get_controller_axis_mapping, ( ALLEGRO_JOYSTICK *joystick, int stick, int axis));

#ifdef __cplusplus
} /* End extern "C" */
#endif

#endif


/* vim: set sts=3 sw=3 et: */
