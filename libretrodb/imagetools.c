#include <stdint.h>
#include <string.h>
#include <formats/image.h>

void copy_rect(unsigned fb1_w, unsigned fb1_h, uint32_t* fb1_data, unsigned fb1_x, unsigned fb1_y, unsigned rect_w, unsigned rect_h, unsigned fb2_w, unsigned fb2_h, uint32_t* fb2_data, unsigned fb2_x, unsigned fb2_y)
{
   for(unsigned y = 0; y < rect_h; y++)
   {
      for(unsigned x = 0; x < rect_w; x++)
      {
         fb2_data[fb2_w * (fb2_y + y) + fb2_x + x] = fb1_data[fb1_w * (fb1_y + y) + fb1_x + x];
      }
   }
}

void simple_float_scale(unsigned w, unsigned h, uint32_t* data, unsigned out_w, unsigned out_h, uint32_t* output_buffer)
{
   for(unsigned y = 0; y < out_h; y++)
   {
      for(unsigned x = 0; x < out_w; x++)
      {
         double scale_x = (float)x / (float)out_w * (float)w;
         double scale_y = (float)y / (float)out_h * (float)h;
         unsigned pixel_scale_x = (int)scale_x;
         unsigned pixel_scale_y = (int)scale_y;
         
         //prevent float inaccuracy from reading undefined memory
         if (pixel_scale_x > w)
            pixel_scale_x = w;
         if (pixel_scale_y > h)
            pixel_scale_y = h;
         
         output_buffer[y * out_w + x] = data[pixel_scale_y * w + pixel_scale_x];
      }
   }
}

void get_file_thumbnail(const char* path, uint32_t* output_buffer, unsigned width, unsigned height)
{
   struct texture_image thumbnail_file;
   bool worked;
   
   worked = image_texture_load(&thumbnail_file, path);
   
   if (worked)
   {
      simple_float_scale(thumbnail_file.width, thumbnail_file.height, thumbnail_file.pixels, width, height, output_buffer);
      
      image_texture_free(&thumbnail_file);
   }
   else
   {
      //return null buffer
      memset(output_buffer, 0x00, width * height * sizeof(uint32_t));
   }
}
