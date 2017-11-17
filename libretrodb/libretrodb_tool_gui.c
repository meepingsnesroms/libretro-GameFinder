/* Copyright  (C) 2010-2017 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (libretrodb_tool.c).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#include <string/stdstring.h>

#include "libretrodb.h"
#include "rmsgpack_dom.h"
#include "../libretro_exported.h"
#include "../ugui/ugui.h"



#define MAX_OBJECTS 100
#define ITEM_LIST_ENTRYS 20
#define MAX_TEXT_ENTRY_SIZE 300

const char* query_types[3] = {
   "List all games",
   "Simple Query",
   "Text Query"
};


int rv;
libretrodb_t *db;
libretrodb_cursor_t *cur;
libretrodb_query_t *q;
struct rmsgpack_dom_value item;
const char *command, *path, *query_exp, *error;
bool last_db_call_success;


UG_GUI     fb_gui;
UG_WINDOW  fb_window;
UG_OBJECT  fb_objects[MAX_OBJECTS];

uint32_t   thumbnail_fb[60*60];
UG_IMAGE   game_thumbnail;

char       textbox_string[ITEM_LIST_ENTRYS][MAX_TEXT_ENTRY_SIZE];
UG_TEXTBOX list_entrys[ITEM_LIST_ENTRYS];


void write_pixel(UG_S16 x, UG_S16 y, UG_COLOR color)
{
   framebuffer[y * SCREEN_WIDTH + x] = color;
}

void window_callback(UG_MESSAGE* message)
{
   //do nothing
}

void get_names(char* query_exp)
{
   last_db_call_success = true;
   error = NULL;
   q = libretrodb_query_compile(db, query_exp, strlen(query_exp), &error);
   
   if (error)
   {
      libretro_log_printf("%s\n", error);
      last_db_call_success = false;
      return;
   }
   
   if ((rv = libretrodb_cursor_open(db, cur, q)) != 0)
   {
      libretro_log_printf("Could not open cursor: %s\n", strerror(-rv));
      last_db_call_success = false;
      return;
   }
   
   while (libretrodb_cursor_read_item(cur, &item) == 0)
   {
      if (item.type == RDT_MAP) //should always be true, but if false the program would segfault
      {
         unsigned i;
         for (i = 0; i < item.val.map.len; i++)
         {
            if (item.val.map.items[i].key.type == RDT_STRING && (strncmp(item.val.map.items[i].key.val.string.buff, "name", item.val.map.items[i].key.val.string.len) == 0))
            {
               rmsgpack_dom_value_print(&item.val.map.items[i].value);
               printf("\n");
            }
         }
      }
      
      rmsgpack_dom_value_free(&item);
   }
}

void find(char* query_exp)
{
   last_db_call_success = true;
   error = NULL;
   q = libretrodb_query_compile(db, query_exp, strlen(query_exp), &error);
   
   if (error)
   {
      libretro_log_printf("%s\n", error);
      last_db_call_success = false;
      return;
   }
   
   if ((rv = libretrodb_cursor_open(db, cur, q)) != 0)
   {
      libretro_log_printf("Could not open cursor: %s\n", strerror(-rv));
      last_db_call_success = false;
      return;
   }
   
   while (libretrodb_cursor_read_item(cur, &item) == 0)
   {
      /*
      rmsgpack_dom_value_print(&item);
      printf("\n");
      rmsgpack_dom_value_free(&item);
      */
   }
}

void list()
{
   last_db_call_success = true;
   
   if ((rv = libretrodb_cursor_open(db, cur, NULL)) != 0)
   {
      libretro_log_printf("Could not open cursor: %s\n", strerror(-rv));
      last_db_call_success = false;
      return;
   }
   
   while (libretrodb_cursor_read_item(cur, &item) == 0)
   {
      /*
      rmsgpack_dom_value_print(&item);
      printf("\n");
      rmsgpack_dom_value_free(&item);
      */
   }
}

#if 0

int main(int argc, char ** argv)
{
   int rv;
   libretrodb_t *db;
   libretrodb_cursor_t *cur;
   libretrodb_query_t *q;
   struct rmsgpack_dom_value item;
   const char *command, *path, *query_exp, *error;

   if (argc < 3)
   {
      printf("Usage: %s <db file> <command> [extra args...]\n", argv[0]);
      printf("Available Commands:\n");
      printf("\tlist\n");
      printf("\tcreate-index <index name> <field name>\n");
      printf("\tfind <query expression>\n");
      printf("\tget-names <query expression>\n");
      return 1;
   }

   command = argv[2];
   path    = argv[1];

   db  = libretrodb_new();
   cur = libretrodb_cursor_new();

   if (!db || !cur)
      goto error;

   if ((rv = libretrodb_open(path, db)) != 0)
   {
      printf("Could not open db file '%s': %s\n", path, strerror(-rv));
      goto error;
   }
   else if (memcmp(command, "list", 4) == 0)
   {
      if ((rv = libretrodb_cursor_open(db, cur, NULL)) != 0)
      {
         printf("Could not open cursor: %s\n", strerror(-rv));
         goto error;
      }

      if (argc != 3)
      {
         printf("Usage: %s <db file> list\n", argv[0]);
         goto error;
      }

      while (libretrodb_cursor_read_item(cur, &item) == 0)
      {
         rmsgpack_dom_value_print(&item);
         printf("\n");
         rmsgpack_dom_value_free(&item);
      }
   }
   else if (memcmp(command, "find", 4) == 0)
   {
      if (argc != 4)
      {
         printf("Usage: %s <db file> find <query expression>\n", argv[0]);
         goto error;
      }

      query_exp = argv[3];
      error = NULL;
      q = libretrodb_query_compile(db, query_exp, strlen(query_exp), &error);

      if (error)
      {
         printf("%s\n", error);
         goto error;
      }

      if ((rv = libretrodb_cursor_open(db, cur, q)) != 0)
      {
         printf("Could not open cursor: %s\n", strerror(-rv));
         goto error;
      }

      while (libretrodb_cursor_read_item(cur, &item) == 0)
      {
         rmsgpack_dom_value_print(&item);
         printf("\n");
         rmsgpack_dom_value_free(&item);
      }
   }
   else if (memcmp(command, "get-names", 9) == 0)
   {
      if (argc != 4)
      {
         printf("Usage: %s <db file> find-name <query expression>\n", argv[0]);
         goto error;
      }
      
      query_exp = argv[3];
      error = NULL;
      q = libretrodb_query_compile(db, query_exp, strlen(query_exp), &error);
      
      if (error)
      {
         printf("%s\n", error);
         goto error;
      }
      
      if ((rv = libretrodb_cursor_open(db, cur, q)) != 0)
      {
         printf("Could not open cursor: %s\n", strerror(-rv));
         goto error;
      }
      
      while (libretrodb_cursor_read_item(cur, &item) == 0)
      {
         if (item.type == RDT_MAP) //should always be true, but if false the program would segfault
         {
            unsigned i;
            for (i = 0; i < item.val.map.len; i++)
            {
               if (item.val.map.items[i].key.type == RDT_STRING && (strncmp(item.val.map.items[i].key.val.string.buff, "name", item.val.map.items[i].key.val.string.len) == 0))
               {
                  rmsgpack_dom_value_print(&item.val.map.items[i].value);
                  printf("\n");
               }
            }
         }

         rmsgpack_dom_value_free(&item);
      }
   }
   else if (memcmp(command, "create-index", 12) == 0)
   {
      const char * index_name, * field_name;

      if (argc != 5)
      {
         printf("Usage: %s <db file> create-index <index name> <field name>\n", argv[0]);
         goto error;
      }

      index_name = argv[3];
      field_name = argv[4];

      libretrodb_create_index(db, index_name, field_name);
   }
   else
   {
      printf("Unknown command %s\n", argv[2]);
      goto error;
   }

   libretrodb_close(db);

error:
   if (db)
      libretrodb_free(db);
   if (cur)
      libretrodb_cursor_free(cur);
   return 1;
}

#endif

void make_query_expression()
{
   
}

bool init_gui_db_tool()
{
   bool passed;
   uint8_t object_cnt;
   db  = libretrodb_new();
   cur = libretrodb_cursor_new();
   
   if (!db || !cur)
      return false;
   
   passed = UG_Init(&fb_gui, &write_pixel, SCREEN_WIDTH, SCREEN_HEIGHT);
   if (!passed)
      return false;
   
   for(int entrys = 0; entrys < ITEM_LIST_ENTRYS; entrys++)
   {
      fb_objects[]
      object_cnt++;
   }
   
   passed = UG_WindowCreate(&fb_window, fb_objects, MAX_OBJECTS, window_callback);
   if (!passed)
      return false;
   
   UG_FontSelect(&FONT_8X8);
   
   return true;
}

void close_gui_db_tool()
{
   libretrodb_close(db);
   libretrodb_free(db);
   libretrodb_cursor_free(cur);
}

void libretro_db_gui_render()
{
   
   //UG_Update();
}
