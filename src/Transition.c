#include <pebble.h>
#include "utils.h"

  
  
static Window *s_main_window;  

static Layer *transition_layer;

static GBitmap *source_bitmap = NULL;
static GBitmap *dest_bitmap = NULL;
//static GBitmap *temp_bitmap = NULL;

// Transition timers
int32_t  trans_start, now, elapsed;

// Set frame rate (in fps)
int frame_rate = 25; 

// Transition settings
int trans_effect = 0;         // No of Transition effect to use
bool trans_fadein = true;     // True for fade-in, False for fade out
int32_t trans_length = 500;   // Amount of time transition should take in milliseconds
uint16_t trans_percent;          // How far through the transition we are based on time now minus time started
bool trans_running = false;   // Whether a transition is currently animating


// Function to return the timestamp in milliseconds
static int32_t get_ms_time(){
  
  time_t sec1;
  uint16_t ms1;
  int32_t t1;

  time_ms(&sec1, &ms1);

  t1 = (int32_t)1000*(int32_t)sec1 + (int32_t)ms1;

  return t1;
}


// Main transition drawing function
static void update_transition_layer(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  uint16_t width = bounds.size.w;
  uint16_t height = bounds.size.h;

  if (trans_percent > 100){
    trans_percent = 100;
    trans_running = false;
  }
  
  uint16_t draw_percent;
  
  if (trans_fadein){
    draw_percent = trans_percent;
  }
  else{
    draw_percent = 100 - trans_percent;
  };
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, width, height), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  // Choose which effect to draw
  switch(trans_effect){
  
    case 0:
    // Horizontal wipe
    graphics_fill_rect(ctx, GRect(0, 0, width * draw_percent/100, height), 0, GCornerNone);
    break;
  
    case 1:
    // Vertical wipe
    graphics_fill_rect(ctx, GRect(0, 0, width, height * draw_percent/100), 0, GCornerNone);
    break;
     
    case 2:
    // Iris wipe
    // CHeck we need to draw something as was leaving 1 pixel in center when fading out
    if(draw_percent >= 1){
      
      //1.414 is used to calculate size of circle to cover given rect
      graphics_fill_circle(ctx, GPoint(width/2,height/2), (height*draw_percent/200)*1414/1000);
    }  
    break;
  
    case 3:
    // Zoom Square
    graphics_fill_rect(ctx, GRect(width/2-width*draw_percent/200, height/2-height*draw_percent/200, width*draw_percent/100, height*draw_percent/100), 0, GCornerNone);
    break;
      
    case 4:
    // Horizontal Barn Door 
    graphics_fill_rect(ctx, GRect(0, 0, width * draw_percent/200, height), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(width - width * draw_percent/200, 0, width, height), 0, GCornerNone);
    break;
    
    case 5:
    // Vertical Barn Door 
    graphics_fill_rect(ctx, GRect(0, 0, width, height * draw_percent/200), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(0, height - height * draw_percent/200, width, height), 0, GCornerNone);
    break;
    
    case 6:
    // Four Corners
    graphics_fill_rect(ctx, GRect(0, 0, width * draw_percent/200, height * draw_percent/200), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(width - width * draw_percent/200, 0, width * draw_percent/200 , height * draw_percent/200), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(0, height - height * draw_percent/200, width * draw_percent/200, height * draw_percent/200), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(width - width * draw_percent/200, height - height * draw_percent/200, width * draw_percent/200 , height * draw_percent/200), 0, GCornerNone);
    
  }
  
  // Capture the frame buffer
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  
  // Put the data into an array
  BitmapInfo fb_bitmap_info;
  fb_bitmap_info.bitmap_data =  gbitmap_get_data(fb);
  fb_bitmap_info.bytes_per_row = gbitmap_get_bytes_per_row(fb);
  fb_bitmap_info.bitmap_format = gbitmap_get_format(fb);

  // get the source data as an array
  BitmapInfo source_bitmap_info;
  source_bitmap_info.bitmap_data =  gbitmap_get_data(source_bitmap);
  source_bitmap_info.bytes_per_row = gbitmap_get_bytes_per_row(source_bitmap);
  source_bitmap_info.bitmap_format = gbitmap_get_format(source_bitmap);
#ifdef PBL_SDK_3
  source_bitmap_info.palette = gbitmap_get_palette(source_bitmap);
#endif
  
  // get the dest data as an array
  BitmapInfo dest_bitmap_info;
  dest_bitmap_info.bitmap_data =  gbitmap_get_data(dest_bitmap);
  dest_bitmap_info.bytes_per_row = gbitmap_get_bytes_per_row(dest_bitmap);
  dest_bitmap_info.bitmap_format = gbitmap_get_format(dest_bitmap);
#ifdef PBL_SDK_3
  dest_bitmap_info.palette = gbitmap_get_palette(dest_bitmap);
#endif
  
  
  // looping thru screen coordinates
  for (int y=0; y<height; y++){
    for (int x=0; x<width; x++){
          
      switch (get_pixel(fb_bitmap_info, y, x)) {
          // if it's a white pixel (both in Aplite & basalt - display destination)
          case 1:
          case 255:
            set_pixel(fb_bitmap_info, y, x, get_pixel(dest_bitmap_info, y, x));
            break;
          // otherwise - display source
          default:
            set_pixel(fb_bitmap_info, y, x, get_pixel(source_bitmap_info, y, x));            
            break;
        }
    }
  }
  
  // Release the frame buffer so it can be drawn
  graphics_release_frame_buffer(ctx, fb);
  
}



static void start_new_transition();

static void trans_timer_handler(void *context) {

  // Get the time now
  now = get_ms_time();
  
  // Get how many milliseconds from trans start to now
  elapsed = now - trans_start;
  
  //int e = elapsed;
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "ELAPSED: %d", e);
  
  // Calculate what percentage we are through the context
  trans_percent = elapsed * 100 / trans_length;
 
  // Force the layer to update
  layer_mark_dirty(transition_layer);
  
  // if transition is less than 100% register the next call based on frame rate 
  //if (trans_percent < 100){
  if (trans_running){
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Scheduling next run...");
    
    // Schedule the next animation callback
    app_timer_register(1000/frame_rate, trans_timer_handler, NULL);
    
  }
  else{
    
    // Display current image for 2 seconds
    psleep(1500);
    
    // For DEMO - Swap images, choose another number and do another transition
    //temp_bitmap = dest_bitmap;
    //dest_bitmap = source_bitmap;
    //source_bitmap = temp_bitmap;
    //temp_bitmap = NULL;
    
    /*
    // Choose a random transition effect
    int r = rand() % 7;
      
    // Make sure we don't get the same number two times in a roe
    while (r == trans_effect){
      r = rand() % 7;
    }
    */
    
    // Update the transition effect to the new effect number
    //trans_effect = r;
    if (!trans_fadein){trans_effect += 1;}  // Only increase the effect once we've faded out
    if (trans_effect > 6){ trans_effect = 0;}
    
    // Change the transition direction
    trans_fadein = trans_fadein ? false : true;
    
    // Animation has completed
    trans_running = false;
    
    // Start the next transition
    start_new_transition();

  }
  
}


static void start_new_transition(){
  
    // If no animation is running then start another one
    if (!trans_running){
       
      // Set initial percent to zero
      trans_percent = 0;
    
      // Get the start time in milliseconds 
      trans_start = get_ms_time();
            
      // Display which effect we are drawing in the logs
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "START TRANS WITH EFFECT: %d", trans_effect);
            
      trans_running = true;
      
      // Begin this transition
      app_timer_register(1, trans_timer_handler, NULL);
      
    }

}

  
static void main_window_load(Window *window) {
  
  // Setup the layers
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  transition_layer = layer_create(bounds);
  layer_set_update_proc(transition_layer, update_transition_layer);
  layer_add_child(window_layer, transition_layer);
  
    // Set the source and dest bitmaps
    source_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SOURCE_IMAGE);
    dest_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DEST_IMAGE);  
}
  

static void main_window_unload(Window *window) {
  
  layer_remove_from_parent(transition_layer);
  layer_destroy(transition_layer);
  
  gbitmap_destroy(source_bitmap);
  source_bitmap = NULL;
  
  gbitmap_destroy(dest_bitmap);
  dest_bitmap = NULL;
  
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  
  #ifndef PBL_COLOR
  window_set_fullscreen(s_main_window, true);
  #endif
  
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  // Initialise random number generator with seed
  srand(time(NULL));
  
  // Start some random transitions
  start_new_transition();
}

static void deinit() {
  window_destroy(s_main_window);
}

  
  
    
int main() {
  init();
  app_event_loop();
  deinit();
}