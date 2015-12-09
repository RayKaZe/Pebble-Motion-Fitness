#include <pebble.h>

#define KEY_GO 0
  
Window* window1;
Window* window2;
MenuLayer *menu_layer;

TextLayer *text_layer;
TextLayer *back_layer;
InverterLayer *inv_layer;

GBitmap *up_bitmap, *down_bitmap;
BitmapLayer *up_layer, *down_layer;

int mode = 0;
int restActive = 0;
int backActive = 0;
int upHidden = 0;
int pushups = 0;
int record  = 32;
int old_sec = 62;
int restcountdown = 10;
int timecountdown = 30;
int repetition = 0;

void inittrain() {
  pushups = 0;
  restcountdown = 10;
  timecountdown = 30;
  repetition = 0;
  restActive = 0;
}

char *itoa(int num)
{
  static char buff[20] = {};
  int i = 0, temp_num = num, length = 0;
  char *string = buff;
  if(num >= 0) {
    // count how many characters in the number
    while(temp_num) {
      temp_num /= 10;
      length++;
    }
    // assign the number to the buffer starting at the end of the 
    // number and going to the begining since we are doing the
    // integer to character conversion on the last number in the
    // sequence
    for(i = 0; i < length; i++) {
      buff[(length-1)-i] = '0' + (num % 10);
      num /= 10;
    }
    buff[i] = '\0'; // can't forget the null byte to properly end our string
  }
  else {
    buff[1] = '\0';
    buff[0] = '0';
  }
  return string;
}

void on_animation_stopped(Animation *anim, bool finished, void *context)
{
	//Free the memoery used by the Animation
	property_animation_destroy((PropertyAnimation*) anim);
}

void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
	//Declare animation
	PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
	
	//Set characteristics
	animation_set_duration((Animation*) anim, duration);
	animation_set_delay((Animation*) anim, delay);
	
	//Set stopped handler to free memory
	AnimationHandlers handlers = {
		//The reference to the stopped handler is the only one in the array
		.stopped = (AnimationStoppedHandler) on_animation_stopped
	};
	animation_set_handlers((Animation*) anim, handlers, NULL);
	
	//Start animation!
	animation_schedule((Animation*) anim);
}

void move_bar(int a, int b, int steps)
{
  int pixelsA = (a*144) / steps;
  int pixelsB = (b*144) / steps;
  
  GRect start = GRect(pixelsA-144, 80, 144, 60);
	GRect finish = GRect(pixelsB-144, 80, 144, 60);
	animate_layer((Layer *)inv_layer, &start, &finish, 100, 0);
}

// WORKOUT WINDOW
void back_in(int bMode)
{
  if (mode != 2) vibes_short_pulse();
  backActive = 1;
  int duration = 300;
  if (bMode == 0) {
    text_layer_set_text(back_layer, "\nREST");
  }
  if (bMode == 1) {
    text_layer_set_text(back_layer, "\nFINISHED");
  } 
  if (bMode == 2) {
    duration = 0;
    text_layer_set_text(back_layer, "\nRECORD");
  }
  GRect start = GRect(144, 0, 144, 168);
	GRect finish = GRect(0, 0, 144, 168);
	animate_layer((Layer *)back_layer, &start, &finish, duration, 0);
}

void back_out()
{
  if (mode != 2) vibes_short_pulse();
  backActive = 0;
  GRect start = GRect(0, 0, 144, 168);
	GRect finish = GRect(144, 0, 144, 168);
	animate_layer((Layer *)back_layer, &start, &finish, 300, 0);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	int new_sec = tick_time->tm_sec;
	
	if (old_sec != new_sec) {
    old_sec = new_sec;

    if (mode == 1 || restActive == 1) {
      //Move bar back
      if (mode == 1) {
        int old_c = timecountdown;
        timecountdown = timecountdown -1;
        move_bar(old_c, timecountdown, 30);
        
        if (timecountdown == 0) {
          back_in(1);
        }
      }
      
      if (mode == 0) {
        int old_c = restcountdown;
        restcountdown = restcountdown -1;
        move_bar(old_c, restcountdown, 10);
        
        if (restcountdown == 0) {
          restActive = 0;
          pushups = 0;
          text_layer_set_text(text_layer, "0");
          back_out();
        } else {        
          char *str = itoa(restcountdown);
          text_layer_set_text(text_layer, str);
        }
      }
    }
  }
}

void train_go()
{
  upHidden = (upHidden+1)%2;
	layer_set_hidden((Layer *)up_layer, upHidden);
  
  if (upHidden == 0) {
    int old = pushups;
    pushups = pushups + 1;
    int steps = 10;
    if (mode == 2) {
      steps = record;
    }
    if (mode != 1) {
      move_bar(old, pushups, steps);
    }
  }
  
  if (mode == 0 && pushups == 10) {
    repetition = repetition + 1;
    if (repetition >= 3) {
      back_in(1);
    } else {
      restcountdown = 10;
      back_in(0);
      restActive = 1;
    }
  }
  
  if (pushups > 0) {
    char *str = itoa(pushups);
    text_layer_set_text(text_layer, str);
  }
}

void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
  if (backActive == 0)
  train_go();
}

void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
  if (backActive == 0)
  train_go();
}

void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
  if (backActive == 0)
  train_go();
}

void click_config_provider(void *context) 
{
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}


static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
  if(window_is_loaded(window2)) {
    if (backActive == 0)
    train_go();
    train_go();
    train_go();
    train_go();
    train_go();
    train_go();
    train_go();
    train_go();
    train_go();
    train_go();
  }
}

// MENU
void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
	//Which row is it?
	switch(cell_index->row)
	{
	case 0:
		menu_cell_basic_draw(ctx, cell_layer, "1. Workout Mode", NULL, NULL);
		break;
	case 1:
		menu_cell_basic_draw(ctx, cell_layer, "2. Time Mode", NULL, NULL);
		break;
	case 2:
		menu_cell_basic_draw(ctx, cell_layer, "3. Marathon Mode", NULL, NULL);
		break;
	case 3:
		menu_cell_basic_draw(ctx, cell_layer, "4. Statistics", NULL, NULL);
		break;
	case 4:
		menu_cell_basic_draw(ctx, cell_layer, "5. Statistics", NULL, NULL);
		break;
	case 5:
		menu_cell_basic_draw(ctx, cell_layer, "6. Grape", "Bunches of 'em!", NULL);
		break;
	case 6:
		menu_cell_basic_draw(ctx, cell_layer, "7. Melon", "Only three left!", NULL);
		break;
	}
}

uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
  //Number of Menu items
	return 4;
}

void show_record() {
  char *str = itoa(record);
  text_layer_set_text(text_layer, str);
  back_in(2);
}

void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{


  //Get which row
  inittrain();
	mode = cell_index->row;
  window_stack_push(window2, true);
  
  if (mode == 3) {
    show_record();
  }
}


//LOADING
void load_train(Window *window)
{
  pushups = 0;
  backActive = 0;
  
  inittrain();
    
  //Controls
  window_set_click_config_provider(window, click_config_provider);
  
	//Load bitmaps into GBitmap structures
	up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UP);
	down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOWN);
		
	//Create BitmapLayers to show GBitmaps and add to Window
	down_layer = bitmap_layer_create(GRect(0, 10, 144, 64));
	bitmap_layer_set_bitmap(down_layer, down_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(down_layer));

  up_layer = bitmap_layer_create(GRect(0, 10, 144, 64));
	bitmap_layer_set_bitmap(up_layer, up_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(up_layer));
  
  //Load font 
	ResHandle font_handle_54 = resource_get_handle(RESOURCE_ID_IMAGINE_54);
  ResHandle font_handle_26 = resource_get_handle(RESOURCE_ID_IMAGINE_26);
  
  //Back layer
	back_layer = text_layer_create(GRect(144, 0, 144, 168));
	text_layer_set_background_color(back_layer, GColorWhite);
  text_layer_set_text_color(back_layer, GColorBlack);
	text_layer_set_text_alignment(back_layer, GTextAlignmentCenter);
	text_layer_set_font(back_layer, fonts_load_custom_font(font_handle_26));
  text_layer_set_text(back_layer, "\nRECORD");
	layer_add_child(window_get_root_layer(window), (Layer*) back_layer);
  
  //Text layer
	text_layer = text_layer_create(GRect(2, 72, 144, 60));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_load_custom_font(font_handle_54));
  text_layer_set_text(text_layer, "0");
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
  
  //Bar layer
	inv_layer = inverter_layer_create(GRect(160, 80, 144, 60));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
  
}
 
void unload_train(Window *window)
{
	//Destroy TextLayer
	text_layer_destroy(text_layer);
  text_layer_destroy(back_layer);
  inverter_layer_destroy(inv_layer);
	
	//Destroy GBitmaps
	gbitmap_destroy(up_bitmap);
	gbitmap_destroy(down_bitmap);
	
	//Destroy BitmapLayers
	bitmap_layer_destroy(up_layer);
	bitmap_layer_destroy(down_layer);
}

void load_menu(Window *window)
{
	//Create layer - 12 is approx height of the top bar
	menu_layer = menu_layer_create(GRect(0, 0, 144, 168 - 16));
	
	//Let it receive clicks
	menu_layer_set_click_config_onto_window(menu_layer, window);
	
	//Give it its callbacks
	MenuLayerCallbacks callbacks = {
		.draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
		.get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
		.select_click = (MenuLayerSelectCallback) select_click_callback
	};
	menu_layer_set_callbacks(menu_layer, NULL, callbacks);
  
	//Add to Window
	layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));
}

void unload_menu(Window *window)
{
	menu_layer_destroy(menu_layer);
}

void window1_load(Window *window)
{
  load_menu(window);
}
 
void window1_unload(Window *window)
{
  unload_menu(window);
}

void window2_load(Window *window)
{
  load_train(window);
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
}
 
void window2_unload(Window *window)
{
	unload_train(window);
  tick_timer_service_unsubscribe();
}

void init()
{
	window1 = window_create();
	WindowHandlers handlers1 = {
		.load = window1_load,
		.unload = window1_unload
	};
	window_set_window_handlers(window1, (WindowHandlers) handlers1);
	window_stack_push(window1, true);
  
  window2 = window_create();
	WindowHandlers handlers2 = {
		.load = window2_load,
		.unload = window2_unload
	};
	window_set_window_handlers(window2, (WindowHandlers) handlers2);
  
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}
 
void deinit()
{
 	window_destroy(window1);
 	window_destroy(window2);
}
 
int main(void)
{
	init();
	app_event_loop();
	deinit();
}