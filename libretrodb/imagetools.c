#include <stdint.h>

void simple_float_scale(int w, int h, uint32_t* data, int out_w, int out_h, uint32_t* output_buffer)
{
   for(int y = 0; y < out_h; y++)
   {
      for(int x = 0; x < out_w; x++)
      {
         float scale_x = (float)x / (float)out_w * (float)w;
         float scale_y = (float)y / (float)out_h * (float)h;
         
         output_buffer[y * out_w + x] = data[(int)scaley * w + (int)scalex];
      }
   }
}
