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
 *      SDL-compatible controller addon.
 *
 *      By Max Savenkov.
 *
 *      See readme.txt for copyright information.
 */

#include "allegro5/allegro.h"

#include "allegro5/allegro_controller_db.h"
#include "allegro5/joystick.h"
#include "allegro5/internal/aintern_joystick.h"
#include "allegro5/internal/aintern_vector.h"
#include "allegro5/internal/aintern_exitfunc.h"

ALLEGRO_DEBUG_CHANNEL("controller_db")

#if defined(ALLEGRO_WINDOWS)
   #define SDL_LIKE_PLATFORM_STRING "Windows"
#elif defined(ALLEGRO_MACOSX)
   #define SDL_LIKE_PLATFORM_STRING "Mac OS X"
#elif defined(ALLEGRO_UNIX)
   #define SDL_LIKE_PLATFORM_STRING "Linux"
#elif defined(ALLEGRO_IPHONE)
   #define SDL_LIKE_PLATFORM_STRING "iOS"
#elif defined(ALLEGRO_ANDROID)
   #define SDL_LIKE_PLATFORM_STRING "Android"
#elif defined(ALLEGRO_RASPBERRYPI)
   #define SDL_LIKE_PLATFORM_STRING "RaspberryPi" // Unsupported by SDL?
#else
   #define SDL_LIKE_PLATFORM_STRING "Universal"
#endif

#define PLATFORM_TAG "platform:"

#define MAX_REVERSE_MAPPINGS 20

typedef struct ALLEGRO_MAPPING
{
   ALLEGRO_JOYSTICK_GUID guid;

   bool initialized;

   // Forward mapping (button name to index)
   int axes[ALLEGRO_CONTROLLER_AXIS_MAX];
   int buttons_as_axes[ALLEGRO_CONTROLLER_AXIS_MAX];

   int buttons[ALLEGRO_CONTROLLER_BUTTON_MAX];
   int axes_as_buttons[ALLEGRO_CONTROLLER_BUTTON_MAX];

   // Reverse mapping (index to button name)
   ALLEGRO_CONTROLLER_AXIS reverse_axes[MAX_REVERSE_MAPPINGS];
   ALLEGRO_CONTROLLER_AXIS reverse_buttons_as_axes[MAX_REVERSE_MAPPINGS];

   ALLEGRO_CONTROLLER_BUTTON reverse_buttons[MAX_REVERSE_MAPPINGS];
   ALLEGRO_CONTROLLER_BUTTON reverse_axes_as_buttons[MAX_REVERSE_MAPPINGS];

   // Temporary storage for digital/analog indices until mapping is fully initialized
   ALLEGRO_CONTROLLER_AXIS digital_axes[MAX_REVERSE_MAPPINGS];
   ALLEGRO_CONTROLLER_AXIS analog_axes[MAX_REVERSE_MAPPINGS];
   
   ALLEGRO_CONTROLLER_BUTTON digital_axes_as_buttons[MAX_REVERSE_MAPPINGS];
   ALLEGRO_CONTROLLER_BUTTON analog_axes_as_buttons[MAX_REVERSE_MAPPINGS];
} ALLEGRO_MAPPING;

typedef struct CONTROLLERDB_STR
{
	const ALLEGRO_USTR *str;
	ALLEGRO_USTR_INFO   info;
} CONTROLLERDB_STR;

#define ALLEGRO_HAT_UP			0x01
#define ALLEGRO_HAT_RIGHT       0x02
#define ALLEGRO_HAT_DOWN        0x04
#define ALLEGRO_HAT_LEFT        0x08

static bool controller_db_initialized = false;
static _AL_VECTOR controller_mappings = _AL_VECTOR_INITIALIZER(ALLEGRO_MAPPING);

static ALLEGRO_MAPPING *_al_find_controller_mapping( ALLEGRO_JOYSTICK_GUID *guid )
{
   ALLEGRO_MAPPING *mapping = 0;
   unsigned int i;

   for (i = 0; i < _al_vector_size(&controller_mappings); i++) {
      mapping = _al_vector_ref(&controller_mappings, i);
      if (0 == memcmp(mapping->guid.data, guid->data, sizeof(guid->data))) {
         return mapping;
      }
   }

   return 0;
}

static ALLEGRO_MAPPING *_al_find_and_init_controller_mapping( ALLEGRO_JOYSTICK *joy )
{
	ALLEGRO_MAPPING *mapping;
	ALLEGRO_JOYSTICK_GUID *guid;

	guid = al_get_joystick_guid(joy);
	if ( !guid ) {
		return 0;
	}

	mapping = _al_find_controller_mapping(guid);
	if ( !mapping ) {
		return 0;
	}

	//al_lock_mutex(mapping_mutex);
	if ( !mapping->initialized )
	{
		int digital_idx = 0;
		int analog_idx = 0;
		int ii = 0;
		int sticks_num = al_get_joystick_num_sticks(joy);
		for (ii = 0; ii < sticks_num; ++ii)
		{
			if (( al_get_joystick_stick_flags( joy, ii ) & ALLEGRO_JOYFLAG_ANALOGUE ) == 0) {
				ALLEGRO_CONTROLLER_AXIS axis = mapping->digital_axes[digital_idx];
				ALLEGRO_CONTROLLER_BUTTON button = mapping->digital_axes_as_buttons[digital_idx];
				mapping->axes[axis] = ii;
				mapping->reverse_axes[ii] = axis;
				mapping->axes_as_buttons[button] = ii;
				mapping->reverse_axes_as_buttons[ii] = button;
				++digital_idx;
			}
			else {
				ALLEGRO_CONTROLLER_AXIS axis = mapping->analog_axes[analog_idx];
				ALLEGRO_CONTROLLER_BUTTON button = mapping->analog_axes_as_buttons[digital_idx];
				mapping->axes[axis] = ii;
				mapping->reverse_axes[ii] = axis;
				mapping->axes_as_buttons[button] = ii;
				mapping->reverse_axes_as_buttons[ii] = button;
				++analog_idx;
			}
		}
		mapping->initialized = true;
	}
	//sal_unlock_mutex(mapping_mutex);

	return mapping;
}


static bool _al_set_joystick_guid_from_string( ALLEGRO_JOYSTICK_GUID *guid, const ALLEGRO_USTR *guid_string )
{
   int32_t digitHigh=0, digitLow=0;
   int32_t chr;
   int pos = 0;
   int pos_guid = 0;
   bool first_of_pair = true;
   ALLEGRO_USTR_INFO info;
   const ALLEGRO_USTR *xinput_string;

   if ( !guid || !guid_string ) {
      return false;
   }

   xinput_string = al_ref_cstr( &info, "xinput" );

   if (al_ustr_equal( guid_string, xinput_string ))
   {
		memcpy( &guid->data[0], "xinput", sizeof("xinput") );
		return true;
   }

   while( ( chr = al_ustr_get_next( guid_string, &pos ) ) >= 0 ) {
      int32_t *pDigit = ( first_of_pair ? &digitHigh : &digitLow );
      first_of_pair = !first_of_pair;

      if ( chr >= '0' && chr <= '9' ) {
        *pDigit = ( chr - '0' );
      }
      else if ( chr >= 'a' && chr <= 'f' ) {
        *pDigit = ( chr - 'a' + 10 );
      }
      else if ( chr >= 'A' && chr <= 'F' ) {
        *pDigit = ( chr - 'A' + 10 );
      }
      else
        return false; /* Invalid character in string */

      /* pair if finished */
      if ( first_of_pair ) {
		 uint8_t value;
         if( pos_guid >= sizeof(guid->data) ) {
            return false; /* String is too long */
         }
         value = digitHigh * 16 + digitLow;
         guid->data[ pos_guid ] = value;
         ++pos_guid;
      }
   }

   if ( false == first_of_pair ) {
      return false; /* Invalid string, ends in the middle of a pair */
   }

   if ( pos_guid < sizeof( guid->data ) ) {
      return false; /* String is too short */
   }

   return true;
}

static const bool _al_get_guid_from_controller_string( const ALLEGRO_USTR *controller_string, CONTROLLERDB_STR *guid_str )
{
   int first_comma = al_ustr_find_chr(controller_string, 0, ',');
   if (first_comma < 0) {
       return false;
   }
    
   guid_str->str = al_ref_ustr(&guid_str->info, controller_string, 0, first_comma);
   return true;
}

static bool _al_get_name_from_controller_string( const ALLEGRO_USTR *controller_string, CONTROLLERDB_STR *name_str )
{
   int first_comma, second_comma;

   first_comma = al_ustr_find_chr(controller_string, 0, ',');
   if (first_comma < 0) {
       return false;
   }

   second_comma = al_ustr_find_chr(controller_string, first_comma+1, ',');
   if (second_comma < 0) {
       return false;
   }

   name_str->str = al_ref_ustr(&name_str->info, controller_string, first_comma+1, second_comma);
   return true;
}

static bool _al_get_mapping_from_controller_string( const ALLEGRO_USTR *controller_string, CONTROLLERDB_STR *mapping_string )
{
   int first_comma, second_comma, pos;

   first_comma = al_ustr_find_chr(controller_string, 0, ',');
   if (first_comma < 0) {
       return false;
   }

   second_comma = al_ustr_find_chr(controller_string, first_comma+1, ',');
   if (second_comma < 0) {
       return false;
   }

   pos = second_comma;
   if ( !al_ustr_next(controller_string, &pos) )
	   return false;
   
   mapping_string->str = al_ref_ustr(&mapping_string->info, controller_string, pos, al_ustr_size(controller_string) );
   return true;
}


static int _al_ustr_atoi(const ALLEGRO_USTR *str)
{
	int pos = 0;
	int value = 0;

	do
	{
		char chr = (char)al_ustr_get(str, pos);
		if ( chr < '0' || chr > '9' )
			return value;

		value *= 10;
		value += ( chr - '0' );
	}
	while( al_ustr_next(str, &pos) );

	return value;
}


static bool _al_parse_controller_button_type_and_index(const ALLEGRO_USTR *game_button, char *type, int *index)
{
	int pos = 0;
	const ALLEGRO_USTR *index_str;
	ALLEGRO_USTR_INFO info;

	*type = (char)al_ustr_get(game_button, pos);
	if ( !al_ustr_next(game_button, &pos) )
		return false;

	index_str = al_ref_ustr(&info, game_button, pos, al_ustr_size(game_button));

	*index = _al_ustr_atoi(index_str);

	return true;
}


static bool _al_parse_controller_hat(const ALLEGRO_USTR *game_button, int *index, int *direction)
{
	int index_start_pos=0, dir_start_pos=0;
	const ALLEGRO_USTR *index_str;
	const ALLEGRO_USTR *dir_str;
	int dot_pos;
	int delim_pos;

	if ( !al_ustr_next(game_button, &index_start_pos) ) /* Skip 'h' */
		return false;

	dot_pos = al_ustr_find_set_cstr( game_button, index_start_pos, "." );
	if ( dot_pos < 0 )
		return false;

	dir_start_pos = dot_pos;
	al_ustr_next(game_button, &dir_start_pos ); /* Skip '.' */

	delim_pos = al_ustr_find_set_cstr( game_button, dir_start_pos, ":" );
	if ( delim_pos < 0 )
		return false;

	index_str = al_ref_ustr(0, game_button, index_start_pos, dot_pos);
	dir_str = al_ref_ustr(0, game_button, dir_start_pos, delim_pos);

	*index = _al_ustr_atoi(index_str);
	*direction = _al_ustr_atoi(dir_str);

	return true;
}



typedef struct _AL_AXIS_MAPPING
{
	const char *name;
	ALLEGRO_CONTROLLER_AXIS axis;
} _AL_AXIS_MAPPING;

static const _AL_AXIS_MAPPING axis_strings[] = {
	{ "leftx",        ALLEGRO_CONTROLLER_AXIS_LEFTX },
    { "lefty",        ALLEGRO_CONTROLLER_AXIS_LEFTY },
    { "rightx",       ALLEGRO_CONTROLLER_AXIS_RIGHTX },
    { "righty",       ALLEGRO_CONTROLLER_AXIS_RIGHTY },
    { "lefttrigger",  ALLEGRO_CONTROLLER_AXIS_TRIGGERLEFT },
    { "righttrigger", ALLEGRO_CONTROLLER_AXIS_TRIGGERRIGHT },
	
	// Here, difference from SDL begins: allegro treats d-pad as an axis, not as a button
	
	{ "dpup",        ALLEGRO_CONTROLLER_AXIS_DPADY },
	{ "dpdown",      ALLEGRO_CONTROLLER_AXIS_DPADY },
	{ "dpleft",      ALLEGRO_CONTROLLER_AXIS_DPADX },
	{ "dpright",     ALLEGRO_CONTROLLER_AXIS_DPADX },
};

ALLEGRO_CONTROLLER_AXIS _al_get_axis_from_string(const ALLEGRO_USTR *string)
{
	int i=0;
	const int MAPPING_SIZE = sizeof(axis_strings)/sizeof(axis_strings[0]);
    if ( !string )
        return ALLEGRO_CONTROLLER_AXIS_INVALID;

    for ( i = 0; i < MAPPING_SIZE; ++i)
    {
		ALLEGRO_USTR_INFO info;
		const ALLEGRO_USTR *name_ustr = al_ref_cstr(&info, axis_strings[i].name);
		if ( al_ustr_equal(name_ustr, string))
			return axis_strings[i].axis;
    }
    return ALLEGRO_CONTROLLER_AXIS_INVALID;
}

static const char* button_strings[] = {
    "a",
    "b",
    "x",
    "y",
    "back",
    "guide",
    "start",
    "leftstick",
    "rightstick",
    "leftshoulder",
    "rightshoulder",
    NULL
};

/*
 * convert a string to its enum equivalent
 */
ALLEGRO_CONTROLLER_BUTTON _al_get_button_from_string(const ALLEGRO_USTR *string)
{
	int i=0;
	if ( !string )
		return ALLEGRO_CONTROLLER_BUTTON_INVALID;

	for ( i = 0; i < ALLEGRO_CONTROLLER_BUTTON_MAX; ++i)
	{
		ALLEGRO_USTR_INFO info;
		const ALLEGRO_USTR *name_ustr = al_ref_cstr(&info, button_strings[i]);
		if ( al_ustr_equal(name_ustr, string))
			return (ALLEGRO_CONTROLLER_BUTTON)i;
	}
	return ALLEGRO_CONTROLLER_BUTTON_INVALID;
}


static void _al_parse_controller_button(const ALLEGRO_USTR *game_button, const ALLEGRO_USTR *joy_button, ALLEGRO_MAPPING *mapping)
{
	ALLEGRO_CONTROLLER_BUTTON button;
	ALLEGRO_CONTROLLER_AXIS axis;
	char type = 0;
	int button_index = 0;

	if ( !_al_parse_controller_button_type_and_index(joy_button, &type, &button_index ) )
		return;

	button = _al_get_button_from_string(game_button);
	axis = _al_get_axis_from_string(game_button);
		
	if ( type == 'a' )
	{
		if ( button_index >= MAX_REVERSE_MAPPINGS )
		{
			//SDL_SetError("Axis index too large: %d", iSDLButton );
			return;
		}
		if ( axis != ALLEGRO_CONTROLLER_AXIS_INVALID )
		{
			mapping->analog_axes[ button_index ] = axis;
		}
		else if ( button != ALLEGRO_CONTROLLER_BUTTON_INVALID )
		{
			mapping->analog_axes_as_buttons[ button_index ] = button;
		}
		else
		{
			//SDL_assert( !"How did we get here?" );
		}
	}
	else if ( type == 'b' )
	{
		if ( button_index >= MAX_REVERSE_MAPPINGS )
		{
			//SDL_SetError("Button index too large: %d", iSDLButton );
			return;
		}
		if ( button != ALLEGRO_CONTROLLER_BUTTON_INVALID )
		{
			mapping->buttons[ button ] = button_index;
			mapping->reverse_buttons[ button_index ] = button;
		}
		else if ( axis != ALLEGRO_CONTROLLER_AXIS_INVALID )
		{
			mapping->buttons_as_axes[ axis ] = button_index;
			mapping->reverse_buttons_as_axes[ button_index ] = axis;
		}
		else
		{
			//SDL_assert( !"How did we get here?" );
		}
	}
	else if ( type == 'h' )
	{
		int digital_stick_index = 0, direction = 0, direction_index = 0;
		if ( !_al_parse_controller_hat(joy_button, &digital_stick_index, &direction) )
			return;

		if ( direction == ALLEGRO_HAT_LEFT || direction == ALLEGRO_HAT_RIGHT )
			direction_index = 0;
		else
			direction_index = 1;

		if ( button_index >= MAX_REVERSE_MAPPINGS )
		{
			//SDL_SetError("Axis index too large: %d", iSDLButton );
			return;
		}
		if ( axis != ALLEGRO_CONTROLLER_AXIS_INVALID )
		{
			mapping->digital_axes[ digital_stick_index + direction_index ] = axis;
		}
		else if ( button != ALLEGRO_CONTROLLER_BUTTON_INVALID )
		{
			mapping->digital_axes_as_buttons[ digital_stick_index + direction_index ] = button;
		}
		else
		{
			//SDL_assert( !"How did we get here?" );
		}
	} 	
}

static void _al_parse_controller_string(ALLEGRO_MAPPING *mapping, const ALLEGRO_USTR *mapping_string)
{
	int i;
	CONTROLLERDB_STR game_button;
	CONTROLLERDB_STR joy_button;
	bool reading_game_button = true;
	int str_pos = 0;
	int str_prev_pos = 0;
	int chr = 0;
	int button_start = 0;

	game_button.str = 0;
	joy_button.str = 0;

	mapping->initialized = false;

	for ( i=0; i < ALLEGRO_CONTROLLER_AXIS_MAX; ++i ) {
		mapping->axes[i] = -1;
		mapping->buttons_as_axes[i] = -1;
	}

	for ( i=0; i < ALLEGRO_CONTROLLER_BUTTON_MAX; ++i ) {
		mapping->buttons[i] = -1;
		mapping->axes_as_buttons[i] = -1;
	}

	for ( i=0; i < MAX_REVERSE_MAPPINGS; ++i ) {
		mapping->reverse_axes[i] = ALLEGRO_CONTROLLER_AXIS_INVALID;
		mapping->reverse_axes_as_buttons[i] = ALLEGRO_CONTROLLER_BUTTON_INVALID;
		mapping->reverse_buttons[i] = ALLEGRO_CONTROLLER_BUTTON_INVALID;
		mapping->reverse_buttons_as_axes[i] = ALLEGRO_CONTROLLER_AXIS_INVALID;
		mapping->analog_axes[i] = ALLEGRO_CONTROLLER_AXIS_INVALID;
		mapping->analog_axes_as_buttons[i] = ALLEGRO_CONTROLLER_AXIS_INVALID;
		mapping->digital_axes[i] = ALLEGRO_CONTROLLER_AXIS_INVALID;
		mapping->digital_axes_as_buttons[i] = ALLEGRO_CONTROLLER_AXIS_INVALID;
	}

	while( ( chr = al_ustr_get_next( mapping_string, &str_pos ) ) >= 0 ) {
		if ( chr == ':' )
		{
			reading_game_button = false;
			game_button.str = al_ref_ustr(&game_button.info, mapping_string, button_start, str_prev_pos );
			button_start = str_pos;
		}
		else if ( chr == ',' )
		{
			reading_game_button = true;
			joy_button.str = al_ref_ustr(&joy_button.info, mapping_string, button_start, str_prev_pos );
			button_start = str_pos;
			_al_parse_controller_button(game_button.str, joy_button.str, mapping);
		}
		str_prev_pos = str_pos;
	}
}

static bool _al_get_platform_from_controller_string( const ALLEGRO_USTR *controller_string, CONTROLLERDB_STR *platform_string )
{
	int start_pos, end_pos;
	
	start_pos = al_ustr_find_cstr(controller_string, 0, PLATFORM_TAG);
	if (start_pos < 0) {
		return false;
	}

	start_pos = al_ustr_find_chr(controller_string, start_pos, ':');
	if (start_pos < 0) {
		return false;
	}

	if (!al_ustr_next(controller_string, &start_pos)) {
		return false;
	}

	end_pos = al_ustr_find_chr(controller_string, start_pos, ',');
	if (end_pos < 0) {
		end_pos = al_ustr_size(controller_string);
	}

	platform_string->str = al_ref_ustr(&platform_string->info, controller_string, start_pos, end_pos );
	return true;
}

int al_register_controller_mapping( const char *controller_string )
{
    CONTROLLERDB_STR guid_string;
	CONTROLLERDB_STR name_string;
	CONTROLLERDB_STR mapping_string;
	CONTROLLERDB_STR platform_string;

    ALLEGRO_JOYSTICK_GUID joyGUID;
    ALLEGRO_MAPPING *mapping;
    const ALLEGRO_USTR *controller_ustr;
    ALLEGRO_USTR_INFO str_info;
	ALLEGRO_USTR_INFO platform_str_info;
	const ALLEGRO_USTR *platform_ustr = al_ref_cstr(&platform_str_info,SDL_LIKE_PLATFORM_STRING);

    controller_ustr = al_ref_cstr(&str_info, controller_string);

	if ( !_al_get_platform_from_controller_string(controller_ustr, &platform_string) ) {
		return -6;
	}
	if(!al_ustr_equal(platform_string.str, platform_ustr)) {
		return -7;
	}

    if (!_al_get_guid_from_controller_string(controller_ustr, &guid_string)) {
        return -1;
    }

    if ( false == _al_set_joystick_guid_from_string(&joyGUID, guid_string.str) ) {
        return -5;
    }
    
    if (!_al_get_name_from_controller_string(controller_ustr, &name_string)) {
        return -2;
    }

    if (!_al_get_mapping_from_controller_string(controller_ustr, &mapping_string)) {
        return -3;
    }

    mapping = _al_find_controller_mapping( &joyGUID );

    if (NULL == mapping) {
		mapping = _al_vector_alloc_back(&controller_mappings);
		if (NULL == mapping) {
			return -4;
		}
	}

	_al_parse_controller_string(mapping, mapping_string.str);

    memcpy(&mapping->guid.data[0], &joyGUID.data[0], sizeof(joyGUID.data));
    /*mapping->name = name;*/
    /*mapping->mapping = mappingString;*/
    return 0;
}

int al_load_controller_mapping( const char *filename )
{
	(void)filename;
	return -1;
}

bool al_has_mapping_for_joystick( ALLEGRO_JOYSTICK *joystick )
{
   ALLEGRO_JOYSTICK_GUID *guid;

   guid = al_get_joystick_guid(joystick);
   if ( !guid ) {
      return false;
   }

   return (0 != _al_find_controller_mapping(guid));
}

ALLEGRO_CONTROLLER_BUTTON al_get_controller_button_mapping( ALLEGRO_JOYSTICK *joystick, int button )
{
   ALLEGRO_MAPPING *mapping;

   if (button<0 || button>=MAX_REVERSE_MAPPINGS ){
      return ALLEGRO_CONTROLLER_BUTTON_INVALID;
   }

   mapping = _al_find_and_init_controller_mapping(joystick);
   if ( 0 == mapping ) {
      return ALLEGRO_CONTROLLER_BUTTON_INVALID;
   }

   if ( mapping->reverse_buttons[ button ] != ALLEGRO_CONTROLLER_BUTTON_INVALID )
	   return mapping->reverse_buttons[ button ];
   else if ( mapping->reverse_axes_as_buttons[ button ] != ALLEGRO_CONTROLLER_BUTTON_INVALID )
	   return mapping->reverse_axes_as_buttons[ button ];
   
   return ALLEGRO_CONTROLLER_BUTTON_INVALID;
}

ALLEGRO_CONTROLLER_AXIS al_get_controller_axis_mapping( ALLEGRO_JOYSTICK *joystick, int stick, int axis )
{
	ALLEGRO_MAPPING *mapping;

	if (stick+axis<0 || stick+axis>=MAX_REVERSE_MAPPINGS ){
		return ALLEGRO_CONTROLLER_AXIS_INVALID;
	}

	mapping = _al_find_and_init_controller_mapping(joystick);
	if ( 0 == mapping ) {
		return ALLEGRO_CONTROLLER_AXIS_INVALID;
	}

	if ( mapping->reverse_axes[ stick+axis ] != ALLEGRO_CONTROLLER_BUTTON_INVALID )
		return mapping->reverse_axes[ stick+axis ];
	else if ( mapping->reverse_buttons_as_axes[ stick+axis ] != ALLEGRO_CONTROLLER_BUTTON_INVALID )
		return mapping->reverse_buttons_as_axes[ stick+axis ];

	return ALLEGRO_CONTROLLER_AXIS_INVALID;
}

static void controller_db_shutdown(void)
{
	if (controller_db_initialized){
		_al_vector_free(&controller_mappings); 
		controller_db_initialized = false;
	}
}

void al_controller_db_init(void)
{
   controller_db_initialized = true;
   _al_add_exit_func(controller_db_shutdown, "controller_db_shutdown");
}

/* vim: set sts=3 sw=3 et: */
