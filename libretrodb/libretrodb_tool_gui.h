#ifndef __LIBRETRODBTOOLGUI_H__
#define __LIBRETRODBTOOLGUI_H__

#include <limits.h>

enum db_command
{
   LIST_ALL_GAMES = 0,
   SIMPLE_QUERY,
   TEXT_QUERY,
   
   LAST_COMMAND = INT_MAX
};

typedef struct
{
   int game_year;
   int game_month;
   char name[200];
   char genre[100];
   char developer[100];
   //char tags[500];//no tags yet
}simple_query_expression;//set propertys to -1 to ignore, for strings set first letter to NULL terminator '\0'

extern bool init_gui_db_tool();
extern void close_gui_db_tool();
extern void libretro_db_gui_render();

#endif
