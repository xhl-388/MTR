#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h> 
#include <cmath>
#include <cstdio>

typedef struct {int width, height, channels; unsigned char *buffer;} image_t;

typedef struct window window_t;
typedef enum {KEY_A, KEY_D, KEY_S, KEY_W, KEY_SPACE, KEY_NUM} keycode_t;
typedef enum {BUTTON_L, BUTTON_R, BUTTON_NUM} button_t;
typedef struct {
    void (*key_callback)(window_t *window, keycode_t key, int pressed);
    void (*button_callback)(window_t *window, button_t button, int pressed);
    void (*scroll_callback)(window_t *window, float offset);
} callbacks_t;

typedef struct {float x, y, z, w;} vec4_t;
typedef struct {
    int width, height;
    vec4_t *colorbuffer;
    float *depthbuffer;
} framebuffer_t;


#define PATH_SIZE 2048

struct window {
    Window handle;
    XImage *ximage;
    image_t *surface;
    /* common data */
    int should_close;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    callbacks_t callbacks;
    void *userdata;
};

/* window related functions */

static Display *g_display = NULL;
static XContext g_context;

static void open_display(void);

unsigned char *private_get_pixel(image_t *image, int row, int col);

static Window create_window(const char *title, int width, int height);

image_t *image_create(int width, int height, int channels);

void image_release(image_t *image);

static void create_surface(int width, int height,
                           image_t **out_surface, XImage **out_ximage);

window_t *window_create(const char *title, int width, int height);

void window_destroy(window_t *window);

int window_should_close(window_t *window) ;

void window_set_userdata(window_t *window, void *userdata);

void *window_get_userdata(window_t *window);

static void present_surface(window_t *window);

void private_blit_image_bgr(image_t *src, image_t *dst);

void window_draw_image(window_t *window, image_t *image);
/* input related functions */

static void handle_key_event(window_t *window, int virtual_key, char pressed);

static void handle_button_event(window_t *window, int xbutton, char pressed);

static void handle_client_event(window_t *window, XClientMessageEvent event);

static void process_event(XEvent *event);

void input_poll_events(void);

int input_key_pressed(window_t *window, keycode_t key);

int input_button_pressed(window_t *window, button_t button);

void input_query_cursor(window_t *window, float *xpos, float *ypos);

void input_set_callbacks(window_t *window, callbacks_t callbacks);

void testKeyCallback(window_t *window, keycode_t key, int pressed);