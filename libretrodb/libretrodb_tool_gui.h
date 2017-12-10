#ifndef __LIBRETRODBTOOLGUI_H__
#define __LIBRETRODBTOOLGUI_H__

#include <limits.h>
#include <retro_miscellaneous.h>
#include "../libretro_exported.h"

enum ui_window
{
   NO_ACTION = 0,
   MAIN_MENU,
   LIST_ALL_GAMES,
   SIMPLE_QUERY,
   TEXT_QUERY,
   
   PICK_PARAM,
   SHOW_GAME,
   SHOW_GAME_PROPERTY,
   
   NO_MATCHING_GAMES,
   LAST_WINDOW = INT_MAX
};

enum query_propertys
{
   QUERY_START = 0;
   NAME,
   RELEASE_START_YEAR,
   RELEASE_END_YEAR,
   RELEASE_START_MONTH,
   RELEASE_END_MONTH,
   DEVELOPER,
   GENRE,
   
   TAGS,
   
   QUERY_END
};

extern char database_path[PATH_MAX_LENGTH];

extern bool init_gui_db_tool();
extern void close_gui_db_tool();
extern void libretro_db_gui_render();

#endif
