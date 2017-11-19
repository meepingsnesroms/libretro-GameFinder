#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "libretro.h"
#include "libretro_exported.h"
#include "libretrodb/libretrodb_tool_gui.h"

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;

#if 0
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
#endif

static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;


char database_path[PATH_MAX];
char thumbnails_path[PATH_MAX];

uint8_t  keyboard_keys[KEYBOARD_KEY_COUNT];
uint8_t  keyboard_keys_last_frame[KEYBOARD_KEY_COUNT];
uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

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
   struct retro_log_callback log;
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = fallback_log;
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
   environ_cb = cb;
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

void update_input()
{
   int i;
   
   input_poll_cb();
   
   memcpy(keyboard_keys_last_frame, keyboard_keys, KEYBOARD_KEY_COUNT);
   
   //numbers
   for (i = RETROK_0; i <= RETROK_9; i++)
      keyboard_keys[i] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, i) ? 1 : 0;
   
   //letters
   for (i = RETROK_a; i <= RETROK_z; i++)
      keyboard_keys[i] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, i) ? 1 : 0;
   
   //arrows
   keyboard_keys[RETROK_UP] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_UP) ? 1 : 0;
   keyboard_keys[RETROK_DOWN] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_DOWN) ? 1 : 0;
   keyboard_keys[RETROK_LEFT] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_LEFT) ? 1 : 0;
   keyboard_keys[RETROK_RIGHT] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_RIGHT) ? 1 : 0;
   
   //actions
   keyboard_keys[RETROK_RETURN] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_RETURN) ? 1 : 0;
   keyboard_keys[RETROK_DELETE] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_DELETE) ? 1 : 0;
   
   //modifiers
   keyboard_keys[RETROK_LSHIFT] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_LSHIFT) ? 1 : 0;
   keyboard_keys[RETROK_RSHIFT] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_RSHIFT) ? 1 : 0;
   
   //symbols
   keyboard_keys[RETROK_LEFTBRACKET] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_LEFTBRACKET) ? 1 : 0;
   keyboard_keys[RETROK_RIGHTBRACKET] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_RIGHTBRACKET) ? 1 : 0;
   keyboard_keys[RETROK_COLON] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_COLON) ? 1 : 0;
   keyboard_keys[RETROK_QUOTE] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_QUOTE) ? 1 : 0;
   keyboard_keys[RETROK_UNDERSCORE] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_UNDERSCORE) ? 1 : 0;
   
   for(i = 0; i < KEYBOARD_KEY_COUNT; i++)
   {
      if (keyboard_keys[i] && !keyboard_keys_last_frame[i])
         printf("Key %d was pressed and released.\n", i);
   }
}

void retro_run(void)
{
   update_input();
   
   libretro_db_gui_render();

   video_cb(framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH * sizeof(uint32_t));
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
   
   memset(keyboard_keys, 0, KEYBOARD_KEY_COUNT);
   
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

