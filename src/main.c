#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static GColor s_background_color;
static GColor s_element_color;
static GFont s_time_font, s_date_font;
static int s_battery_level;
static Layer *s_battery_layer;

static void update_time() {
  // build a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // write the current hours and minutes into a buffer
  static char time_buffer[8];
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  // write the current date into a buffer
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%m  %d", tick_time);
  
  // display the time and date on the text layers
  text_layer_set_text(s_time_layer, time_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void update_battery(Layer *layer, GContext *ctx) {
  // get the battery layer properties
  GRect bounds = layer_get_bounds(layer);
  
  // calculate the width of the remaining battery bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.w);
  
  // calculate the remaining battery bar starting x-coordinate
  int xCoord = (bounds.size.w - width) / 2;
  
  // set the battery meter color
  GColor batteryMeterColor;
  if(width <= 20) {
    batteryMeterColor = GColorRed;
  } else {
    batteryMeterColor = s_element_color;
  }
  
  // draw the battery level background
  graphics_context_set_fill_color(ctx, s_background_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  // draw the battery meter bar
  graphics_context_set_fill_color(ctx, batteryMeterColor);
  graphics_fill_rect(ctx, GRect(xCoord, 0, width, bounds.size.h), 0, GCornerNone);
}

static void battery_callback(BatteryChargeState state) {
  // capture the state of the battery
  s_battery_level = state.charge_percent;
  
  // mark the layer as dirty to render as soon as possible
  layer_mark_dirty(s_battery_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // get window properties that differ by watch
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // create the time and date text layers
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  s_date_layer = text_layer_create(GRect(0, 120, 144, 30));
  
  // create the time and date text layer fonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DROID_SERIF_BOLD_50));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DROID_SERIF_BOLD_22));
  
  // style the time and date text layers
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, s_element_color);
  text_layer_set_text_color(s_date_layer, s_element_color);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // create the battery meter layer
  s_battery_layer = layer_create(GRect(0, 0, bounds.size.w, 10));
  layer_set_update_proc(s_battery_layer, update_battery);
  
  // add the display layers as children layers to the window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, s_battery_layer);
}

static void main_window_unload(Window *window) {
  // destroy the display layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
  
  // unload the fonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
}

static void init() {
  // create the main window and save the memory address
  s_main_window = window_create();
  
  // set main window handler functions
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // set color scheme
  s_background_color = GColorBlueMoon;
  s_element_color = GColorWhite;
  
  // style the window
  window_set_background_color(s_main_window, s_background_color);
  
  // register with the tick timer service
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // register with the battery state service
  battery_state_service_subscribe(battery_callback);
  
  // show the window on the watch with animated=true
  window_stack_push(s_main_window, true);
  
  // immediately display the time
  update_time();
  
  // immediately display the battery level
  battery_callback(battery_state_service_peek());
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
