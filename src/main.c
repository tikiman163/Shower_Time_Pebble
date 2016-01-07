#include <pebble.h>

Window *window;
TextLayer *text_layer_interface;
TextLayer *text_layer_time;

static bool st_timer_on = false;
static int s_uptime = 0;
static int pulse_interval = 30;

static void set_text() {
  static char s_total_buffer[64];
  
  static char time_buffer[8];
  static char time_text_buffer[16];
  clock_copy_time_string(time_buffer, sizeof(time_buffer));
  snprintf(time_text_buffer, sizeof(time_text_buffer), "Time %sM", time_buffer);
  text_layer_set_text(text_layer_time, time_text_buffer);
  
  int seconds = s_uptime % 60;
  int minutes = (s_uptime % 3600) / 60;
  int hours = s_uptime / 3600;
  
  int interval_seconds = pulse_interval %60;
  int interval_minutes = (pulse_interval % 3600) / 60;
  int interval_hours = pulse_interval / 3600;
  
  snprintf(s_total_buffer, sizeof(s_total_buffer),
           "Timer: %d:%02d:%02d\nInterval:\n%d:%02d:%02d",
           hours, minutes, seconds, interval_hours, interval_minutes, interval_seconds);
  text_layer_set_text(text_layer_interface, s_total_buffer);
}

static void update_timer() {
  if((s_uptime % pulse_interval) == 0 && s_uptime != 0) {
    vibes_long_pulse();
  }
  
  s_uptime++;
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
  if(st_timer_on) {
    update_timer();
  }
  set_text();
}

void window_load(Window *window)
{
  static char time_buffer[8];
  clock_copy_time_string(time_buffer, sizeof(time_buffer));
  static char initial_time_buffer[16];
  snprintf(initial_time_buffer, sizeof(initial_time_buffer), "Time %sM", time_buffer);
  //We will add the creation of the Window's elements here soon!
  
  text_layer_interface = text_layer_create(GRect(0, 43, 144, 82));//24*3 = 72 2*5 = '82' / 2 = '43'
  text_layer_set_text(text_layer_interface, "Timer: 0:00:00\nInterval:\n0:00:00");
  text_layer_set_background_color(text_layer_interface, GColorBlack);
  text_layer_set_text_color(text_layer_interface, GColorWhite);
  text_layer_set_font(text_layer_interface, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(text_layer_interface, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_interface));
  
  text_layer_time = text_layer_create(GRect(0, 0, 144, 18));
  text_layer_set_text(text_layer_time, initial_time_buffer);
  text_layer_set_background_color(text_layer_time, GColorWhite);
  text_layer_set_text_color(text_layer_time, GColorBlue);
  text_layer_set_font(text_layer_time, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(text_layer_time, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_time));
  
}

static void timer_start() {
    st_timer_on = true;
}

static void timer_stop() {
    st_timer_on = false;
}

static void timer_toggle() {
  if (st_timer_on) {
    timer_stop();
  }
  else {
    timer_start();
  }
}
 
void window_unload(Window *window)
{
  //We will safely destroy the Window's elements here!
  text_layer_destroy(text_layer_interface);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  pulse_interval += 2;
  set_text();
}

static void up_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  pulse_interval += 30;
  set_text();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  timer_toggle();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_uptime = 0;
  set_text();
}

static void select_long_down_handler(ClickRecognizerRef recognizer, void *context) {
  
}

static void select_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  pulse_interval = 30;
  set_text();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(pulse_interval > 10) {
    pulse_interval -= 2;
    set_text();
  }
}

static void down_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  pulse_interval -= 30;
  if(pulse_interval < 10) {
    pulse_interval = 10;
  }
  set_text();
}

void config_provider(Window *window) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 500, up_click_handler);
  window_multi_click_subscribe(BUTTON_ID_UP, 2, 10, 0, true, up_multi_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, select_long_down_handler);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 10, 0, true, select_multi_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 500, down_click_handler);
  window_multi_click_subscribe(BUTTON_ID_DOWN, 2, 10, 0, true, down_multi_click_handler);
}

void init()
{
  window = window_create();
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  window_stack_push(window, true);
  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);
  tick_timer_service_subscribe(SECOND_UNIT, time_handler);
}

void deinit()
{
  window_unload(window);
  window_destroy(window);
  timer_stop();
}

int main(void)
{
  init();
  window_load(window);
  app_event_loop();
  deinit();
}