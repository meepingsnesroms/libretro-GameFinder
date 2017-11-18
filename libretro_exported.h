#ifndef LREXPORTED_H__
#define LREXPORTED_H__

#include <stdint.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define KEYBOARD_KEY_COUNT 323

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

extern char database_path[PATH_MAX];
extern char thumbnails_path[PATH_MAX];
//extern keys libretro_keys;
extern uint8_t  keyboard_keys[KEYBOARD_KEY_COUNT];
//extern uint8_t  last_frame_keyboard_keys[128];
extern uint32_t framebuffer[320*240];

extern void libretro_log_printf(const char *fmt, ...);

#endif

