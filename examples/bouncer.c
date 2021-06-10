#include <stdio.h>
#include <allegro5/allegro.h>

#ifndef GL_ES_VERSION_2_0
#include <GLES2/gl2.h>
#endif
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <gbm.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const float FPS = 60;
int SCREEN_W = 640;
int SCREEN_H = 480;
const int BOUNCER_SIZE = 32;

int main(int argc, char **argv) {
    // printf("Here 1\n");
    // fflush(stdout);
    //       int drm_fd = open("/dev/dri/card1", O_RDWR);
    //       //int drm_fd = open("/dev/dri/renderD128", O_RDWR);
    //       struct gbm_device *dev;
    //       dev = gbm_create_device(drm_fd);
    //       printf("drm_fd: %d dev: %p\n", drm_fd, dev);
    //     	eglGetDisplay(dev);//(void *)gbm->dev);
    //     	printf("Here\n");
    //     	fflush(stdout);
    // return 0;
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *bouncer = NULL;
    float bouncer_x = SCREEN_W / 2.0 - BOUNCER_SIZE / 2.0;
    float bouncer_y = SCREEN_H / 2.0 - BOUNCER_SIZE / 2.0;
    float bouncer_dx = -4.0, bouncer_dy = 4.0;
    bool redraw = true;

    if(!al_init()) {
        printf("failed to initialize allegro!\n");
        return -1;
    }

    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        printf("failed to create timer!\n");
        return -1;
    }

    al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_PROGRAMMABLE_PIPELINE);

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
        printf("failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }
    if (!al_install_keyboard()) {
        printf("failed to install keyboard!\n");
        return -1;
    }
    SCREEN_W = al_get_display_width(display);
    SCREEN_H = al_get_display_height(display);
    printf("%d %d\n", al_get_display_width(display), al_get_display_height(display));
    //for (int i = 0; i < 500; i++) {
    //        al_clear_to_color(al_map_rgb_f(1., 0.5, 0.5));
    //        al_flip_display();
    //}
    //al_clear_to_color(al_map_rgb_f(1., 0.5, 0.5));
    //al_flip_display();
    //al_clear_to_color(al_map_rgb_f(1., 0.5, 0.5));
    //al_flip_display();
    //printf("Done... resting.\n");
    //al_rest(3.5);
    //return 0;
        
    bouncer = al_create_bitmap(BOUNCER_SIZE, BOUNCER_SIZE);
    if(!bouncer) {
        printf("failed to create bouncer bitmap!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    al_set_target_bitmap(bouncer);

    al_clear_to_color(al_map_rgb(255, 0, 255));

    al_set_target_bitmap(al_get_backbuffer(display));

    event_queue = al_create_event_queue();
    if(!event_queue) {
        printf("failed to create event_queue!\n");
        al_destroy_bitmap(bouncer);
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));

    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(0,0,0));

    al_flip_display();

    al_start_timer(timer);

    while(al_get_time() < 10.0) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_TIMER) {
            if(bouncer_x < 0 || bouncer_x > SCREEN_W - BOUNCER_SIZE) {
                bouncer_dx = -bouncer_dx;
            }
            if(bouncer_y < 0 || bouncer_y > SCREEN_H - BOUNCER_SIZE) {
                bouncer_dy = -bouncer_dy;
            }

            bouncer_x += bouncer_dx;
            bouncer_y += bouncer_dy;

            redraw = true;
        }
        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            printf("Key down %x %x\n", ev.keyboard.keycode, ALLEGRO_KEY_ESCAPE);
            if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
               break;
            }
        }
        else {
            if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                break;
            }
        }

        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;

            al_clear_to_color(al_map_rgb(0,0,0));

            al_draw_bitmap(bouncer, bouncer_x, bouncer_y, 0);

            al_flip_display();
        }
    }

    al_destroy_bitmap(bouncer);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
