#include <stdint.h>

uint32_t* simple_float_scale(int w, int h, uint32_t* data, int out_w, int out_h)
{
   uint32_t* output_buffer = malloc(out_w * out_h * sizeof(uint32_t));
   
   if(!output_buffer)
      return NULL;
   
   for(int y = 0; y < out_h; y++)
   {
      for(int x = 0; x < out_w; x++)
      {
         float scale_x = (float)x / (float)out_w * (float)w;
         float scale_y = (float)y / (float)out_h * (float)h;
         
         output_buffer[y * out_w + x] = data[(int)scaley * w + (int)scalex];
      }
   }
   return output_buffer;
}
