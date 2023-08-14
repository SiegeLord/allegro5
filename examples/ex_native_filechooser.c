#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_native_dialog.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
	ALLEGRO_DISPLAY * display = NULL;
	ALLEGRO_MENU * top_menu = NULL;
	ALLEGRO_MENU * menu = NULL;
	ALLEGRO_FONT * font = NULL;
	ALLEGRO_EVENT_QUEUE * queue = NULL;
	ALLEGRO_EVENT event;
	int height[3] = {0};
	int i;

	if(!al_init())
	{
		goto fail;
	}
	if(!al_install_keyboard())
	{
		goto fail;
	}
	if(!al_init_native_dialog_addon())
	{
		goto fail;
	}
	if(!al_init_font_addon())
	{
		goto fail;
	}
	al_set_new_display_flags(ALLEGRO_OPENGL);
	display = al_create_display(800, 600);
	if(!display)
	{
		goto fail;
	}
	height[0] = al_get_display_height(display);
	font = al_create_builtin_font();
	if(!font)
	{
		goto fail;
	}
	queue = al_create_event_queue();
	if(!queue)
	{
		goto fail;
	}
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_keyboard_event_source());
	menu = al_create_menu();
	if(!menu)
	{
		goto fail;
	}
	al_append_menu_item(menu, "Item 1", 0, 0, NULL, NULL);
	al_append_menu_item(menu, "Item 2", 1, 0, NULL, NULL);
	top_menu = al_create_menu();
	if(!top_menu)
	{
		goto fail;
	}
	al_append_menu_item(top_menu, "Main", 2, 0, NULL, menu);
	al_set_display_menu(display, top_menu);
	top_menu = NULL;

	while(1)
	{
		if (al_get_next_event(queue, &event))
		{
			if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			{
				break;
			}
			else if(event.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				height[1] = al_get_display_height(display);
				if(!top_menu)
				{
					top_menu = al_remove_display_menu(display);
				}
				else
				{
					al_set_display_menu(display, top_menu);
					top_menu = NULL;
				}
				height[2] = al_get_display_height(display);
			}
			else if(event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
			{
				al_acknowledge_resize(display);
				printf("Resize\n");
			}
			printf("%d\n", event.type);
			fflush(stdout);
		}
		al_rest(1.0 / 60.0);
		al_clear_to_color(al_map_rgb(0, 0, 0));
		for(i = 0; i < 3; i++)
		{
			al_draw_textf(font, al_map_rgb(255, 255, 255), 0, i * al_get_font_line_height(font), 0, "%d", height[i]);
		}
		al_flip_display();
	}
	al_destroy_font(font);
	al_destroy_event_queue(queue);
	al_destroy_display(display);

	return 0;

	fail:
	{
		if(top_menu)
		{
			al_destroy_menu(top_menu);
		}
		if(menu)
		{
			al_destroy_menu(menu);
		}
		if(queue)
		{
			al_destroy_event_queue(queue);
		}
		if(font)
		{
			al_destroy_font(font);
		}
		if(display)
		{
			al_destroy_display(display);
		}
		return -1;
	}
}