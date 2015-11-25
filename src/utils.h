#include <pebble.h>
#pragma once
  

// used to pass bimap info to get/set pixel accurately  
typedef struct {
   uint8_t *bitmap_data;
   int bytes_per_row;
   GBitmapFormat bitmap_format;
   GColor *palette;
}  BitmapInfo;  

  
// set pixel color at given coordinates 
//void set_pixel(uint8_t *bitmap_data, int bytes_per_row, int y, int x, uint8_t color);

// get pixel color at given coordinates 
//uint8_t get_pixel(uint8_t *bitmap_data, int bytes_per_row, int y, int x);

void set_pixel(BitmapInfo bitmap_info, int y, int x, uint8_t color);
uint8_t get_pixel(BitmapInfo bitmap_info, int y, int x);