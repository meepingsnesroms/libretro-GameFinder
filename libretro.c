#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "libretro.h"
#include "libretro_exported.h"
#include "libretrodb/libretrodb_tool_gui.h"

retro_log_printf_t log_cb;
retro_video_refresh_t video_cb;

#if 0
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
#endif

retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;


char database_path[PATH_MAX];
char thumbnails_path[PATH_MAX];

uint8_t  keyboard_keys[KEYBOARD_KEY_COUNT];
uint32_t framebuffer[320*240];

void libretro_log_printf(const char *fmt, ...)
{
   va_list va;
   va_start(va,fmt);
   char buff[4096];
   vsnprintf(buff, 4096, fmt, va);
   log_cb(RETRO_LOG_INFO, buff);
   va_end(va);
}

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   va_list va;

   (void)level;

   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

void retro_init(void)
{
   
}

void retro_deinit(void)
{

}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "Game Finder";
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
   info->library_version  = "v0.1" GIT_VERSION;
   info->need_fullpath    = true;
   info->valid_extensions = "rdb";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->timing.fps = 60.0;
   info->timing.sample_rate = 30000.0;

   info->geometry.base_width   = SCREEN_WIDTH;
   info->geometry.base_height  = SCREEN_HEIGHT;
   info->geometry.max_width    = SCREEN_WIDTH;
   info->geometry.max_height   = SCREEN_HEIGHT;
   info->geometry.aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;//square pixels, aspect ratio changes with screen size
}

void retro_set_environment(retro_environment_t cb)
{
   struct retro_log_callback logging;

   environ_cb = cb;

   if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
      log_cb = logging.log;
   else
      log_cb = fallback_log;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
#if 0
   audio_cb = cb;
#endif
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
#if 0
   audio_batch_cb = cb;
#endif
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_reset(void)
{
}

void retro_run(void)
{
   input_poll_cb();
   
   for (int j = 0; j < RETROK_LAST; j++)
      keyboard_keys[j] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, j) ? 1 : 0;
   
   libretro_db_gui_render();

   video_cb(framebuffer, 320, 240, 320 * sizeof(uint32_t));
}

bool retro_load_game(const struct retro_game_info *info)
{
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "%s\n", "XRGB8888 is not supported.");
      return false;
   }
   
   strcpy(database_path, info->path);
   
   if(!init_gui_db_tool())
      return false;
   
   return true;
}

void retro_unload_game(void)
{
   close_gui_db_tool();
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   (void)type;
   (void)info;
   (void)num;
   return false;
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *data_, size_t size)
{
   return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
   return false;
}

void *retro_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

