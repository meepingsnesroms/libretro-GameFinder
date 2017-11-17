#ifndef LREXPORTED_H__
#define LREXPORTED_H__

#include <stdint.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct
{
   bool up;
   bool down;
   bool left;
   bool right;
   bool a;
   bool b;
}keys;

extern char database_path[PATH_MAX];
extern char thumbnails_path[PATH_MAX];
extern keys libretro_keys;
extern uint32_t framebuffer[320*240];

extern void libretro_log_printf(const char *fmt, ...);

#endif

