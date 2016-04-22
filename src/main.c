#include <pebble.h>

#define LABEL_OPEN "Open"
#define LABEL_TOGGLE "Toggle"
#define LABEL_CLOSE "Close"

Window *window;

static Window *s_main_window;
static TextLayer *open_label_layer, *toggle_label_layer, *close_label_layer;
static ActionBarLayer *s_action_bar_layer;
static GBitmap *s_open_bitmap, *s_close_bitmap, *s_toggle_bitmap;

typedef enum {
  open = 1,
  toggle,
  close
} Command;

void send_message(Command cmd) {
  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;
  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if(result == APP_MSG_OK) {
    // Add an item to ask for weather data
    int key = 1;
    int value = (int)cmd;
    dict_write_int(out_iter, key, &value, sizeof(int), true);
    dict_write_end(out_iter);

    // Send this message
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}


void open_button_handler(ClickRecognizerRef recognizer, void *context) {
  Command cmd = open;
  send_message(cmd);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Open button is clicked");
}

void toggle_button_handler(ClickRecognizerRef recognizer, void *context) {
  Command cmd = toggle;
  send_message(cmd);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Toggle button is clicked");
}

void close_button_handler(ClickRecognizerRef recognizer, void *context) {
  Command cmd = close;
  send_message(cmd);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Close button is clicked");
}

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) open_button_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) toggle_button_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) close_button_handler);
}

// AppMessage handlers

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  // A new message has been successfully received
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received but not handled");
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // A message was received, but had to be dropped
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // The message just sent has been successfully delivered
  APP_LOG(APP_LOG_LEVEL_INFO, "Message was successfully delivered");
}

static void outbox_failed_callback(DictionaryIterator *iter,
                                      AppMessageResult reason, void *context) {
  // The message just sent failed to be delivered
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
}

void init() {
  window = window_create();
  window_set_background_color(s_main_window, GColorWhite);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // add Open label
  GEdgeInsets label_insets_open = {.top = bounds.size.h * 0.15,
                                   .bottom = bounds.size.h * 0.72,
                                   .right = ACTION_BAR_WIDTH, .left = ACTION_BAR_WIDTH / 2};
  open_label_layer = text_layer_create(grect_inset(bounds, label_insets_open));
  text_layer_set_text(open_label_layer, LABEL_OPEN);
  // text_layer_set_background_color(open_label_layer, GColorRed);
  layer_add_child(window_layer, text_layer_get_layer(open_label_layer));

  // add Toggle label
  GEdgeInsets label_insets_toggle = {.top = bounds.size.h * 0.4,
                                     .bottom = bounds.size.h * 0.42,
                                     .right = ACTION_BAR_WIDTH, .left = ACTION_BAR_WIDTH / 2};
  toggle_label_layer = text_layer_create(grect_inset(bounds, label_insets_toggle));
  text_layer_set_text(toggle_label_layer, LABEL_TOGGLE);
  text_layer_set_font(toggle_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(toggle_label_layer));

  // add Close label
  GEdgeInsets label_insets_close = {.top = bounds.size.h * 0.76,
                                    .bottom = 0,
                                    .right = ACTION_BAR_WIDTH, .left = ACTION_BAR_WIDTH / 2};
  close_label_layer = text_layer_create(grect_inset(bounds, label_insets_close));
  text_layer_set_text(close_label_layer, LABEL_CLOSE);
  layer_add_child(window_layer, text_layer_get_layer(close_label_layer));

  // Action bar
  s_open_bitmap = gbitmap_create_with_resource(RESOURCE_ID_OPEN);
  s_close_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOSE);
  s_toggle_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TOGGLE);
  s_action_bar_layer = action_bar_layer_create();
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_open_bitmap);
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_toggle_bitmap);
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_close_bitmap);
  action_bar_layer_set_background_color(s_action_bar_layer, PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite));
  action_bar_layer_add_to_window(s_action_bar_layer, window);
  action_bar_layer_set_click_config_provider(s_action_bar_layer, click_config_provider);

  window_stack_push(window, true);

  // Register AppMessage callbacks
  app_message_open(64, 64);
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_outbox_failed(outbox_failed_callback);

}

void deinit() {
  text_layer_destroy(open_label_layer);
  text_layer_destroy(toggle_label_layer);
  text_layer_destroy(close_label_layer);
  action_bar_layer_destroy(s_action_bar_layer);

  gbitmap_destroy(s_open_bitmap);
  gbitmap_destroy(s_close_bitmap);
  gbitmap_destroy(s_toggle_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

int main() {
  init();
  app_event_loop();
  deinit();
  return 0;
}