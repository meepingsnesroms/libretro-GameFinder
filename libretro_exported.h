#ifndef LREXPORTED_H__
#define LREXPORTED_H__

#include <stdint.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define KEYBOARD_KEY_COUNT 323

extern uint8_t  keyboard_keys[KEYBOARD_KEY_COUNT];
extern uint8_t  keyboard_keys_last_frame[KEYBOARD_KEY_COUNT];
extern uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

extern void libretro_log_printf(const char *fmt, ...);

#endif

