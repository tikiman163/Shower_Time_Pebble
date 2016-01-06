#include <pebble.h>

Window *window;
TextLayer *text_layer;

static bool st_timer_on = false;
static int s_uptime = 0;
static int pulse_interval = 30;

static void set_text() {
  static char s_total_buffer[64];
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char time_buffer[8];
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  int seconds = s_uptime % 60;
  int minutes = (s_uptime % 3600) / 60;
  int hours = s_uptime / 3600;
  
  int interval_seconds = pulse_interval %60;
  int interval_minutes = (pulse_interval % 3600) / 60;
  int interval_hours = pulse_interval / 3600;
  
  snprintf(s_total_buffer, sizeof(s_total_buffer),
           "Time %s\nTimer: %d:%02d:%02d\nInterval:\n%d:%02d:%02d",
           time_buffer, hours, minutes, seconds, interval_hours, interval_minutes, interval_seconds);
  text_layer_set_text(text_layer, s_total_buffer);
}

static void update_timer() {
  if((s_uptime % pulse_interval) == 0 && s_uptime != 0) {
    vibes_long_pulse();
  }
  
  s_uptime++;
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
  set_text();
  if(st_timer_on) {
    update_timer();
  }
}

void window_load(Window *window)
{
  //We will add the creation of the Window's elements here soon!
  text_layer = text_layer_create(GRect(0, 20, 144, 128));//H168 - 128 = 40 / 2 = 20
  text_layer_set_text(text_layer, "00:00\nShower Timer:\n0h 0m 0s\nInterval: 30s");
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
  
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
  text_layer_destroy(text_layer);
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