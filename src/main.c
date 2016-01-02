#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;

static void update_time() {
  // build a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  // display the time on the text layer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // get window properties that differ by watch
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // create the text layer
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  
  // create the text font
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DROID_SERIF_BOLD_42));
  
  // style the text layer
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // add the text layer as a child layer to the window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  // destroy the text layer
  text_layer_destroy(s_time_layer);
  
  // unload the font
  fonts_unload_custom_font(s_time_font);
}

static void init() {
  // create the main window and save the memory address
  s_main_window = window_create();
  
  // set main window handler functions
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // style the window
  window_set_background_color(s_main_window, GColorWhite);
  
  // register with the tick timer service
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // show the window on the watch with animated=true
  window_stack_push(s_main_window, true);
  
  // immediately display the time
  update_time();
}

static void deinit() {
  // destroy the main window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
