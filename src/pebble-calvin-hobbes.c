#include <pebble.h>

#define KEY_TEMPERATURE 99

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;

static GFont s_time_font;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_charging_layer;
static GBitmap *s_charging_bitmap;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char buffer[] = "00:00";

  if(clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  text_layer_set_text(s_time_layer, buffer);

  static char bufferDate[] = "MON 00";
  strftime(bufferDate, sizeof("MON 00"), "%a %d", tick_time);
  text_layer_set_text(s_date_layer, bufferDate);
}

static void battery_handler(BatteryChargeState charge_state) {
  static char s_battery_buffer[16];

  if(charge_state.is_charging) {
    s_battery_buffer[0] = 0;
  } else {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
  }
  layer_set_hidden(bitmap_layer_get_layer(s_charging_layer), !charge_state.is_charging);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}

static void main_window_load(Window *window) {

  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  s_time_layer = text_layer_create(GRect(0, 25, 144, 168));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");

  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  s_date_layer = text_layer_create(GRect(0, 65, 144, 168));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "MON 00");

  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  s_charging_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_WHITE);
  s_charging_layer = bitmap_layer_create(GRect(127, 2, 10, 20));
  bitmap_layer_set_bitmap(s_charging_layer, s_charging_bitmap);
  bitmap_layer_set_compositing_mode(s_charging_layer, GCompOpOr);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_charging_layer));

  s_battery_layer = text_layer_create(GRect(0, -1, 141, 160));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text(s_battery_layer, "--%");

  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  battery_handler(battery_state_service_peek());

  update_time();
}

static void main_window_unload(Window *window) {

  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);

  fonts_unload_custom_font(s_time_font);
  text_layer_destroy(s_time_layer);

  text_layer_destroy(s_date_layer);

  text_layer_destroy(s_battery_layer);

  gbitmap_destroy(s_charging_bitmap);
  bitmap_layer_destroy(s_charging_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {

  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  battery_state_service_subscribe(battery_handler);
}

static void deinit() {

  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
