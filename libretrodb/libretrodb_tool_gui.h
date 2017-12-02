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

enum propertys
{
   NAME = 0,
   RELEASE_YEAR,
   RELEASE_MONTH,
   DEVELOPER,
   GENRE,
   
   
   LAST_PROPERTY = INT_MAX
};

typedef struct
{
   int  game_year[2];//start<->end years, leave the same to search one year
   int  game_month[2];////start<->end months, leave the same to search one month
   char name[200];
   char genre[100];
   char developer[100];
   char platforms[100];
   //char tags[500];//no tags yet
}simple_query_expression;//set propertys to -1 to ignore, for strings set first letter to NULL terminator '\0'

extern char database_path[PATH_MAX_LENGTH];

extern bool init_gui_db_tool();
extern void close_gui_db_tool();
extern void libretro_db_gui_render();

#endif
