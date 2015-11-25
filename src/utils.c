#include <pebble.h>
#include "utils.h"
  

/*
  // set pixel color at given coordinates 
void set_pixel(uint8_t *bitmap_data, int bytes_per_row, int y, int x, uint8_t color) {
  
  uint8_t (*fb_a)[bytes_per_row] = (uint8_t (*)[bytes_per_row])bitmap_data;
  
  #ifdef PBL_COLOR 
    fb_a[y][x] = color; // in Basalt - simple set entire byte
  #else
    fb_a[y][x / 8] ^= (-color ^ fb_a[y][x / 8]) & (1 << (x % 8)); // in Applite - set the bit
  #endif
}




// get pixel color at given coordinates 
uint8_t get_pixel(uint8_t *bitmap_data, int bytes_per_row, int y, int x) {
  
  uint8_t (*fb_a)[bytes_per_row] = (uint8_t (*)[bytes_per_row])bitmap_data;
  
  #ifdef PBL_COLOR
    return fb_a[y][x]; // in Basalt - simple get entire byte
  #else
    return (fb_a[y][x / 8] >> (x % 8)) & 1; // in Applite - get the bit
  #endif
}
*/
// set pixel color at given coordinates 
void set_pixel(BitmapInfo bitmap_info, int y, int x, uint8_t color) {
  
#ifdef PBL_PLATFORM_BASALT  
  if (bitmap_info.bitmap_format == GBitmapFormat1BitPalette) { // for 1bit palette bitmap on Basalt --- verify if it needs to be different
     bitmap_info.bitmap_data[y*bitmap_info.bytes_per_row + x / 8] ^= (-color ^ bitmap_info.bitmap_data[y*bitmap_info.bytes_per_row + x / 8]) & (1 << (x % 8)); 
#else
  if (bitmap_info.bitmap_format == GBitmapFormat1Bit) { // for 1 bit bitmap on Aplite  --- verify if it needs to be different
     bitmap_info.bitmap_data[y*bitmap_info.bytes_per_row + x / 8] ^= (-color ^ bitmap_info.bitmap_data[y*bitmap_info.bytes_per_row + x / 8]) & (1 << (x % 8)); 
#endif
  } else { // othersise (assuming GBitmapFormat8Bit) going byte-wise
     bitmap_info.bitmap_data[y*bitmap_info.bytes_per_row + x] = color;
  }
      
}

// get pixel color at given coordinates 
uint8_t get_pixel(BitmapInfo bitmap_info, int y, int x) {

#ifdef PBL_PLATFORM_BASALT  
  if (bitmap_info.bitmap_format == GBitmapFormat1BitPalette) { // for 1bit palette bitmap on Basalt shifting left to get correct bit
    return (bitmap_info.bitmap_data[y*bitmap_info.bytes_per_row + x / 8] << (x % 8)) & 128;
#else
  if (bitmap_info.bitmap_format == GBitmapFormat1Bit) { // for 1 bit bitmap on Aplite - shifting right to get bit
    return (bitmap_info.bitmap_data[y*bitmap_info.bytes_per_row + x / 8] >> (x % 8)) & 1;
#endif
  } else {  // othersise (assuming GBitmapFormat8Bit) going byte-wise
    return bitmap_info.bitmap_data[y*bitmap_info.bytes_per_row + x]; 
  }
  
}