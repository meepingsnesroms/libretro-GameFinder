#ifndef IMAGETOOLS_H__
#define IMAGETOOLS_H__

#include <stdint.h>

#include <string>

void copy_rect(unsigned fb1_w, unsigned fb1_h, uint32_t* fb1_data, unsigned fb1_x, unsigned fb1_y, unsigned rect_w, unsigned rect_h, unsigned fb2_w, unsigned fb2_h, uint32_t* fb2_data, unsigned fb2_x, unsigned fb2_y);
void simple_float_scale(unsigned w, unsigned h, uint32_t* data, unsigned out_w, unsigned out_h, uint32_t* output_buffer);
void get_file_thumbnail(std::string path, uint32_t* output_buffer, unsigned width, unsigned height);

#endif
