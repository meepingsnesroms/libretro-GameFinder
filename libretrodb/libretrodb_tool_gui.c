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
#include <retro_miscellaneous.h>
#include <file/file_path.h>

#include "libretrodb.h"
#include "rmsgpack_dom.h"
#include "libretrodb_tool_gui.h"
#include "imagetools.h"
#include "../libretro_exported.h"
#include "../libretro.h"
#include "../ugui/ugui.h"


#define FONT_WIDTH   8
#define FONT_HEIGHT  8
#define FONT_SPACING 0

#define MAX_OBJECTS          100
#define CONSOLE_MESSAGE_SIZE 200
#define KEYBOARD_STRING_SIZE 10000
#define QUERY_STRING_SIZE    KEYBOARD_STRING_SIZE

#define TEXTBOX_PIXEL_MARGIN 1 //add 1 pixel border around text
#define TEXTBOX_PIXEL_WIDTH  (SCREEN_WIDTH / 4 * 3) //3/4 of the screen
#define TEXTBOX_PIXEL_HEIGHT (FONT_HEIGHT + (TEXTBOX_PIXEL_MARGIN * 2))
#define TEXTBOX_DEFAULT_COLOR C_WHITE
#define TEXTBOX_DEFAULT_TEXT_COLOR C_DEEP_SKY_BLUE
#define TEXTBOX_CURSOR_COLOR C_SKY_BLUE
#define TEXTBOX_CURSOR_TEXT_COLOR C_WHITE

#define ITEM_LIST_ENTRYS (SCREEN_HEIGHT / TEXTBOX_PIXEL_HEIGHT - 1)
#define ITEM_STRING_SIZE (TEXTBOX_PIXEL_WIDTH / (FONT_WIDTH + FONT_SPACING))


//paths
char database_path[PATH_MAX_LENGTH];
char assets_path[PATH_MAX_LENGTH];

//db access
int rv;
libretrodb_t *db;
libretrodb_cursor_t *cur;
libretrodb_query_t *q;
struct rmsgpack_dom_value item;
const char *command, *path, *error;
bool last_db_call_success;

//screen
uint32_t   backup_fb[SCREEN_WIDTH * SCREEN_HEIGHT];
UG_GUI     fb_gui;
UG_WINDOW  fb_window;
UG_OBJECT  fb_objects[MAX_OBJECTS];

//thumbnail
uint32_t*  thumbnail;
int        thumbnail_width;
int        thumbnail_height;

//list items
char       textbox_string[ITEM_LIST_ENTRYS][ITEM_STRING_SIZE];
UG_TEXTBOX list_entrys[ITEM_LIST_ENTRYS];
int        list_index_action[ITEM_LIST_ENTRYS];
int        list_index_action_param[ITEM_LIST_ENTRYS];
int64_t    list_length;
int64_t    selected_entry;
char       selected_entry_full_name[PATH_MAX_LENGTH];
int        back_button_action;
int64_t    (*list_handler)(int64_t index);

//keyboard
bool       typing_mode;
char       console_message[CONSOLE_MESSAGE_SIZE];
char       kbd_str[KEYBOARD_STRING_SIZE];
int        kbd_str_index;
void       (*process_kbd_string)();

//query
bool       use_query;
char       game_list_query[QUERY_STRING_SIZE];

//simple query
//also uses query fields above
int        query_section;//what property in the query is being entered
char       query_name[1000];
int        query_start_year;
int        query_end_year;
int        query_start_month;
int        query_end_month;
char       query_genre[100];
char       query_developer[100];
char       query_tags[1000];
char       query_platform[100];//used if you use a merged list with games for all platforms


static void write_pixel(UG_S16 x, UG_S16 y, UG_COLOR color)
{
   framebuffer[y * SCREEN_WIDTH + x] = color;
}

static void make_path_from_name(char* path, char* game_name)
{
   int total_filename_length;
   
   total_filename_length =  strlen(assets_path);
   total_filename_length += strlen(game_name);
   total_filename_length += strlen(".png");
   
   if (total_filename_length <= PATH_MAX_LENGTH)
   {
      strcpy(path, assets_path);
      strcat(path, game_name);
      strcat(path, ".png");
   }
   else
   {
      path[0] = '\0';
   }
}

static void draw_thumbnail(char* path)
{
   get_file_thumbnail(path, thumbnail, thumbnail_width, thumbnail_height);
   copy_rect(thumbnail_width, thumbnail_height, thumbnail, 0/*fb1 start x*/, 0/*fb1 start y*/, thumbnail_width, thumbnail_height, SCREEN_WIDTH, SCREEN_HEIGHT, framebuffer, TEXTBOX_PIXEL_WIDTH, 0/*fb2 start y*/);
}

static void window_callback(UG_MESSAGE* message)
{
   //do nothing
}

static int64_t list_games(int64_t index)
{
   int64_t total_items = 0;
   int64_t items_returned = 0;
   
   last_db_call_success = true;
   
   if (use_query)
   {
      error = NULL;
      q = libretrodb_query_compile(db, game_list_query, strlen(game_list_query), &error);
      
      if (error)
      {
         libretro_log_printf("%s\n", error);
         last_db_call_success = false;
         return 0;
      }
      
      if ((rv = libretrodb_cursor_open(db, cur, q)) != 0)
      {
         libretro_log_printf("Could not open cursor: %s\n", strerror(-rv));
         last_db_call_success = false;
         return 0;
      }
   }
   else
   {
      if ((rv = libretrodb_cursor_open(db, cur, NULL)) != 0)
      {
         libretro_log_printf("Could not open cursor: %s\n", strerror(-rv));
         last_db_call_success = false;
         return 0;
      }
   }
   
   while (libretrodb_cursor_read_item(cur, &item) == 0)
   {
      if (item.type == RDT_MAP) //should always be true, but if false the program would segfault
      {
         for (int64_t i = 0; i < item.val.map.len; i++)
         {
            if (item.val.map.items[i].key.type == RDT_STRING && (strncmp(item.val.map.items[i].key.val.string.buff, "name", item.val.map.items[i].key.val.string.len) == 0))
            {
               if (index > 0)//skip elements till proper index
               {
                  index--;
               }
               else {
                  if (items_returned < ITEM_LIST_ENTRYS)
                  {
                     int null_term = ITEM_STRING_SIZE - 1;
                     if (null_term > item.val.map.items[i].value.val.string.len)
                        null_term = item.val.map.items[i].value.val.string.len;
                     
                     strncpy(textbox_string[items_returned], item.val.map.items[i].value.val.string.buff, null_term);
                     textbox_string[items_returned][null_term] = '\0';//strncpy does not null terminate strings that exceed buffer size
                     UG_TextboxSetText(&fb_window, items_returned, textbox_string[items_returned]);
                     list_index_action[items_returned] = SHOW_GAME;
                     
                     if (items_returned == selected_entry % ITEM_LIST_ENTRYS)//TODO: make define for page number
                     {
                        char thumbnail_path[PATH_MAX_LENGTH];
                        
                        if (item.val.map.items[i].value.val.string.len < PATH_MAX_LENGTH)
                        {
                           strncpy(selected_entry_full_name, item.val.map.items[i].value.val.string.buff, item.val.map.items[i].value.val.string.len);
                           selected_entry_full_name[item.val.map.items[i].value.val.string.len] = '\0';
                        }
                        else
                        {
                           //this variable is only good if it can store the full name, so theres no point in storing a partial name
                           selected_entry_full_name[0] = '\0';
                        }
                        
                        make_path_from_name(thumbnail_path, selected_entry_full_name);
                        draw_thumbnail(thumbnail_path);
                     }
                     
                     items_returned++;
                  }
               }
               
               total_items++;
               break;
            }
         }
      }
      
      rmsgpack_dom_value_free(&item);
   }
   
   while (items_returned < ITEM_LIST_ENTRYS)
   {
      //fill end of list with empty strings
      textbox_string[items_returned][0] = '\0';
      UG_TextboxSetText(&fb_window, items_returned, textbox_string[items_returned]);
      list_index_action[items_returned] = NO_ACTION;
      items_returned++;
   }
   
   libretrodb_cursor_close(cur);//cursor_close also frees the query if present
   return total_items;
}

static void set_main_window()
{
   strcpy(textbox_string[0], "List all games");
   strcpy(textbox_string[1], "Simple Query");
   strcpy(textbox_string[2], "Text Query");
   list_index_action[0] = LIST_ALL_GAMES;
   list_index_action[1] = SIMPLE_QUERY;
   list_index_action[2] = TEXT_QUERY;
   back_button_action   = NO_ACTION;
   
   //Needed to refresh the textbox, even though it is just setting the same string
   UG_TextboxSetText(&fb_window, 0, textbox_string[0]);
   UG_TextboxSetText(&fb_window, 1, textbox_string[1]);
   UG_TextboxSetText(&fb_window, 2, textbox_string[2]);
   
   list_length = 3;
   selected_entry = 0;
   
   //set cursor color
   UG_TextboxSetForeColor(&fb_window, 0, TEXTBOX_CURSOR_TEXT_COLOR);
   UG_TextboxSetBackColor(&fb_window, 0, TEXTBOX_CURSOR_COLOR);
   
   UG_Update();
}

static void run_text_query()
{
   memcpy(framebuffer, backup_fb, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));//restore list mode framebuffer
   back_button_action = MAIN_MENU;
   use_query = true;
   strcpy(game_list_query, kbd_str);
   list_handler = list_games;
   selected_entry = 0;//put cursor at top of the list
   list_games(0);
}

static void set_text_query()
{
   typing_mode = true;
   kbd_str[0] = '\0';
   kbd_str_index = 0;
   process_kbd_string = run_text_query;
   
   strcpy(console_message, "Enter your text query:\n");
   
   memcpy(backup_fb, framebuffer, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
   UG_ConsoleSetArea(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
   UG_ConsoleClear();
   UG_ConsolePutString(console_message);
}

static void build_query_from_params()
{
   int full_query_length = 0;
   bool had_former_entry = false;//used to place commas after entrys
   
   full_query_length += strlen("{");
   if (query_name[0] != '\0')
   {
      full_query_length += strlen("'name':glob('*");
      full_query_length += strlen(query_name);
      full_query_length += strlen("*')");
      had_former_entry = true;
   }
   
   //TODO: add other query types
   
   full_query_length += strlen("}");
   
   
   if (full_query_length < QUERY_STRING_SIZE)
   {
      //query length is known to be safe, now build query
      had_former_entry = false;
      game_list_query[0] = '\0';
      strcat(game_list_query, "{");
      if (query_name[0] != '\0')
      {
         strcat(game_list_query, "'name':glob('*");
         strcat(game_list_query, query_name);
         strcat(game_list_query, "*')");
         had_former_entry = true;
      }
      
      //TODO: add other query types
      
      strcat(game_list_query, "}");
   }
}

static void process_simple_query()
{
   //this function is called after each question and when the query is finished
   switch (query_section)
   {
      case QUERY_START:
         typing_mode = true;
         kbd_str[0] = '\0';
         kbd_str_index = 0;
         
         strcpy(console_message, "Enter full or partial game name, value is case sensitive:\n");
         
         //render first frame
         memcpy(backup_fb, framebuffer, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
         UG_ConsoleSetArea(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
         UG_ConsoleClear();
         UG_ConsolePutString(console_message);
         
         query_section++;
         break;
      case NAME:
         if (strlen(kbd_str) < 1000)
         {
            strcpy(query_name, kbd_str);
         }
         else
         {
            query_name[0] = '\0';
         }
         
         //stay in typing mode for the next section
         typing_mode = true;
         kbd_str[0] = '\0';
         kbd_str_index = 0;
         
         query_section++;
         break;
      default:
      case QUERY_END:
         //build text query from simple query and run it
         memcpy(framebuffer, backup_fb, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));//restore list mode framebuffer
         back_button_action = MAIN_MENU;
         use_query = true;
         build_query_from_params();
         list_handler = list_games;
         selected_entry = 0;//put cursor at top of the list
         list_games(0);
         break;
   }
}

static void set_simple_query()
{
   process_kbd_string = process_simple_query;
   query_section = QUERY_START;
   
   process_simple_query();
}

bool init_gui_db_tool()
{
   int passed;
   path = database_path;
   db   = libretrodb_new();
   cur  = libretrodb_cursor_new();
   
   if (!db || !cur)
      return false;
   
   if ((rv = libretrodb_open(path, db)) != 0)
   {
      libretro_log_printf("Could not open db file '%s': %s\n", path, strerror(-rv));
      return false;
   }
   
   UG_Init(&fb_gui, &write_pixel, SCREEN_WIDTH, SCREEN_HEIGHT);
   UG_FontSelect(&FONT_8X8);
   
   passed = UG_WindowCreate(&fb_window, fb_objects, MAX_OBJECTS, window_callback);
   if (passed != UG_RESULT_OK)
      return false;
   
   UG_WindowSetStyle(&fb_window, WND_STYLE_HIDE_TITLE | WND_STYLE_2D);
   UG_WindowSetForeColor(&fb_window, TEXTBOX_DEFAULT_TEXT_COLOR);
   UG_WindowSetBackColor(&fb_window, TEXTBOX_DEFAULT_COLOR);
   
   UG_ConsoleSetForecolor(C_WHITE);
   UG_ConsoleSetBackcolor(C_DEEP_SKY_BLUE);
   
   int new_textbox_y = 0;
   for (int entry = 0; entry < ITEM_LIST_ENTRYS; entry++)
   {
      UG_TextboxCreate(&fb_window, &list_entrys[entry], entry, 0/*x start*/, new_textbox_y, TEXTBOX_PIXEL_WIDTH - 1/*x end*/, new_textbox_y + TEXTBOX_PIXEL_HEIGHT - 1);
      UG_TextboxSetAlignment(&fb_window, entry, ALIGN_CENTER);
      UG_TextboxSetText(&fb_window, entry, textbox_string[entry]);
      UG_TextboxShow(&fb_window, entry);
      new_textbox_y += TEXTBOX_PIXEL_HEIGHT;
   }
   
   
   //resolve assets directory
   char database_name[PATH_MAX_LENGTH];
   strcpy(database_name, database_path);
   path_remove_extension(database_name);
   char* asset_dir_name = find_last_slash(database_name);
   asset_dir_name++;//dont count the found slash
   
   fill_pathname_basedir(assets_path, database_path, PATH_MAX_LENGTH);
   fill_pathname_slash(assets_path, PATH_MAX_LENGTH);
   strncat(assets_path, "assets", PATH_MAX_LENGTH - strlen(assets_path));
   fill_pathname_slash(assets_path, PATH_MAX_LENGTH);
   strncat(assets_path, asset_dir_name, PATH_MAX_LENGTH - strlen(assets_path));
   fill_pathname_slash(assets_path, PATH_MAX_LENGTH);
   strncat(assets_path, "Named_Boxarts", PATH_MAX_LENGTH - strlen(assets_path));
   fill_pathname_slash(assets_path, PATH_MAX_LENGTH);
   
   
   //thumbnails
   thumbnail_width  = SCREEN_WIDTH - TEXTBOX_PIXEL_WIDTH;
   thumbnail_height = SCREEN_HEIGHT;
   
   thumbnail = malloc(thumbnail_width * thumbnail_height * sizeof(uint32_t));
   if (!thumbnail)
   {
      return false;
   }
   
   typing_mode = false;
   selected_entry_full_name[0] = '\0';
   
   set_main_window();
   UG_WindowShow(&fb_window);
   
   return true;
}

void close_gui_db_tool()
{
   UG_WindowDelete(&fb_window);
   if (thumbnail)
      free(thumbnail);
   
   libretrodb_close(db);
   if (db)
      libretrodb_free(db);
   if (cur)
      libretrodb_cursor_free(cur);
}

static void run_action(int action)
{
   libretro_log_printf("GUI Action:%d\n", action);
   
   switch (action)
   {
      case LIST_ALL_GAMES:
         back_button_action = MAIN_MENU;
         use_query = false;
         list_handler = list_games;
         selected_entry = 0;//put cursor at top of the list
         list_length = list_games(0);
         break;
      case TEXT_QUERY:
         set_text_query();
         break;
      case SIMPLE_QUERY:
         set_simple_query();
         break;
      case SHOW_GAME:
         break;
      case SHOW_GAME_PROPERTY:
         break;
      case NO_ACTION:
         break;
   }
}

static void typing_mode_frame()
{
   int kbd_key = 0;
   
   for (int i = 0; i < KEYBOARD_KEY_COUNT; i++)
   {
      if (keyboard_keys[i] && !keyboard_keys_last_frame[i])
      {
         libretro_log_printf("Key %d pressed\n", i);
         kbd_key = i;
      }
   }
   
   if (kbd_key >= 32/*space*/ && kbd_key <= 126)
   {
      if (kbd_str_index < KEYBOARD_STRING_SIZE - 1/*null terminator*/)
      {
         if (keyboard_keys[RETROK_LSHIFT] || keyboard_keys[RETROK_RSHIFT])
         {
            if (kbd_key >= 'a' && kbd_key <= 'z')
            {
               //make uppercase letter
               kbd_key -= 32;
            }
            else if (kbd_key >= '[' && kbd_key <= ']')
            {
               //swap special chars
               kbd_key += 32;
            }
            else if (kbd_key == ';')
            {
               kbd_key = ':';
            }
            else if (kbd_key == '1')
            {
               kbd_key = '!';
            }
            else if (kbd_key == '2')
            {
               kbd_key = '@';
            }
            else if (kbd_key == '3')
            {
               kbd_key = '#';
            }
            else if (kbd_key == '4')
            {
               kbd_key = '$';
            }
            else if (kbd_key == '5')
            {
               kbd_key = '%';
            }
            else if (kbd_key == '6')
            {
               kbd_key = '^';
            }
            else if (kbd_key == '7')
            {
               kbd_key = '&';
            }
            else if (kbd_key == '8')
            {
               kbd_key = '*';
            }
            else if (kbd_key == '9')
            {
               kbd_key = '(';
            }
            else if (kbd_key == '0')
            {
               kbd_key = ')';
            }
         }
         
         kbd_str[kbd_str_index] = kbd_key;
         kbd_str[kbd_str_index + 1] = '\0';
         kbd_str_index++;
         
         UG_ConsoleClear();
         UG_ConsolePutString(console_message);
         UG_ConsolePutString(kbd_str);
      }
   }
   else if (kbd_key == RETROK_RETURN)
   {
      //send string
      typing_mode = false;//typing_mode must be disabled before process_kbd_string() to give the function the option to hold the gui in typing mode
      
      process_kbd_string();
      
      //restore gui mode
      if (!typing_mode)
      {
         memcpy(framebuffer, backup_fb, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
      }
   }
   else if (kbd_key == RETROK_BACKSPACE/*delete*/)
   {
      if (kbd_str_index > 0)
      {
         kbd_str_index--;
         kbd_str[kbd_str_index] = '\0';
         
         UG_ConsoleClear();
         UG_ConsolePutString(console_message);
         UG_ConsolePutString(kbd_str);
      }
   }
}

static void list_mode_frame()
{
   int64_t page;
   int64_t index;
   static int64_t last_index = 0;
   static int64_t last_page  = 0;
   
   if (keyboard_keys[RETROK_UP] && !keyboard_keys_last_frame[RETROK_UP])
   {
      if(selected_entry - 1 >= 0)
      {
         selected_entry--;
      }
   }
   
   if (keyboard_keys[RETROK_DOWN] && !keyboard_keys_last_frame[RETROK_DOWN])
   {
      if(selected_entry + 1 < list_length)
      {
         selected_entry++;
      }
   }
   
   if (keyboard_keys[RETROK_LEFT] && !keyboard_keys_last_frame[RETROK_LEFT] && list_length > ITEM_LIST_ENTRYS)
   {
      if(selected_entry - ITEM_LIST_ENTRYS >= 0)
      {
         selected_entry -= ITEM_LIST_ENTRYS;//flip the page
      }
      else
      {
         selected_entry = 0;
      }
   }
   
   if (keyboard_keys[RETROK_RIGHT] && !keyboard_keys_last_frame[RETROK_RIGHT] && list_length > ITEM_LIST_ENTRYS)
   {
      if(selected_entry + ITEM_LIST_ENTRYS < list_length)
      {
         selected_entry += ITEM_LIST_ENTRYS;//flip the page
      }
      else
      {
         selected_entry = list_length - 1;
      }
   }
   
   page  = selected_entry / ITEM_LIST_ENTRYS;
   index = selected_entry % ITEM_LIST_ENTRYS;
   
   if (page != last_page || index != last_index && list_handler)
   {
      list_length = list_handler(page * ITEM_LIST_ENTRYS);
   }
   
   if (keyboard_keys[RETROK_RETURN] && !keyboard_keys_last_frame[RETROK_RETURN])
   {
      //select
      run_action(list_index_action[index]);
   }
   
   if (keyboard_keys[RETROK_BACKSPACE] && !keyboard_keys_last_frame[RETROK_BACKSPACE])
   {
      //go back
      run_action(back_button_action);
   }
   
   if (keyboard_keys[RETROK_TAB] && !keyboard_keys_last_frame[RETROK_TAB])
   {
      //skip parameter
   }
   
   if (index != last_index)
   {
      //update item colors
      UG_TextboxSetForeColor(&fb_window, last_index, TEXTBOX_DEFAULT_TEXT_COLOR);
      UG_TextboxSetBackColor(&fb_window, last_index, TEXTBOX_DEFAULT_COLOR);
      
      UG_TextboxSetForeColor(&fb_window, index, TEXTBOX_CURSOR_TEXT_COLOR);
      UG_TextboxSetBackColor(&fb_window, index, TEXTBOX_CURSOR_COLOR);
   }
   
   last_index = index;
   last_page  = page;
   
   UG_Update();
}

void libretro_db_gui_render()
{
   if (typing_mode)
   {
      typing_mode_frame();
   }
   else
   {
      list_mode_frame();
   }
}
