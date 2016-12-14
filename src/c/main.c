#include <pebble.h>

#define ANTIALIASING true

#define KEY_VIBRATION 0
#define KEY_MN_MARKS 1
#define KEY_FIVE_MARKS 2

#define KEY_COLOR_BG_R 3
#define KEY_COLOR_BG_G 4
#define KEY_COLOR_BG_B 5

#define KEY_COLOR_HANDS_R 6
#define KEY_COLOR_HANDS_G 7
#define KEY_COLOR_HANDS_B 8

#define KEY_COLOR_BUBLES_R 9
#define KEY_COLOR_BUBLES_G 10
#define KEY_COLOR_BUBLES_B 11

#define KEY_COLOR_TEXT_R 12
#define KEY_COLOR_TEXT_G 13
#define KEY_COLOR_TEXT_B 14
#define KEY_TEMPERATURE 15
#define KEY_LAST_REFRESH 16

#define QUIET_TIME_START 22
#define QUIET_TIME_END 10

//1 EN
//0 FR
// Penser lamgue openweather
#define LANGAGE 1 

#if defined(PBL_ROUND)   

#define HAND_MARGIN  25 
#define RATIO_HM_HAND 1.7

#else 
#define HAND_MARGIN  20
#define RATIO_HM_HAND 1.7

#endif

#if defined(PBL_ROUND)  
#define FINAL_RADIUS 90
#else
#define FINAL_RADIUS 72
#endif

# define HANDS_WIDTH 10
# define HAND_HOUR_OFFSET 0



#define TAP_DURATION 0

typedef struct {
  int hours;
  int minutes;
  int mounth;
  int day;
  int wday;
  char datetext[11];
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer;
static Layer *layer;
static bool is_vibration;
static bool is_mn_marks;
static bool is_five_marks;

static GColor color_bg;
static GColor color_hands;
static GColor color_bubles;
static GColor color_text;
static bool is_ready;

static bool is_photo_shooting=false;


time_t last_refresh=0;
int duration=1800;
static char temperature_buffer[10]="?";

#if defined(PBL_ROUND)

static GRect rect_text_steps = {{ 41, 2},{ 100, 200 } };
static GRect rect_text_sleep = {{ 41, 154},{ 100, 200 } };
static GRect rect_text_date = {{117, 70},{ 100, 100 } };
static GRect rect_text_bat = {{-35, 78},{ 100, 100 } };

# else

static GRect rect_text_steps = {{ -2, 4},{ 35, 40 } };
static GRect rect_text_sleep = {{ 112, 4},{ 35, 40 } };
static GRect rect_text_bat = {{-4, 143},{ 35, 40 } };
static GRect rect_text_date = {{113, 133},{ 35, 40 } };
/*
    rect_twelve.x= 0;
    rect_twelve.y= 0;
    graphics_fill_circle(ctx, rect_twelve, 6);
    rect_twelve.x= 144;
    rect_twelve.y= 0;
    graphics_fill_circle(ctx, rect_twelve, 6);
    rect_twelve.x= 0;
    rect_twelve.y= 168;
    graphics_fill_circle(ctx, rect_twelve, 6);
    rect_twelve.x= 144;
    rect_twelve.y= 168;
*/

#endif


static char weather_layer_buffer[32]=" ";
static char battery_buffer[10]=" ";
static int battery_level;
static bool is_charging;
static bool is_connected;
static char weather_layer_buffer_big[100]=" ";
static bool draw_hands = true;


static Time s_last_time;


#if defined(PBL_ROUND)  
static int s_radius = 90;
#else
static int s_radius = 72;
#endif


static GFont text_font;
static char pebble_Lang[6];

static char *weekdayLangFr[7]={"D","L","M","M.","J","V","S"};
static char *weekdayLangEn[7]={"S","M","T","W","T","F","S"};
static char *weekdayLangGe[7]={"S","M","D","M","D","F","S"};
static char *weekdayLangSp[7]={"D","L","M","M.","J","V","S"};

static void app_focus_changed(bool focused) {
  if (focused) {
    layer_set_hidden(layer, false);  
    layer_mark_dirty(layer);
  }
}

static void app_focus_changing(bool focusing) {
  if (focusing) {
    layer_set_hidden(layer, true);  
  }
}


static void update_proc(Layer *layer, GContext *ctx) {
  // Color background 
  if (is_ready){
    graphics_context_set_text_color(ctx, color_text);
    graphics_context_set_stroke_width(ctx, HANDS_WIDTH);

    graphics_context_set_fill_color(ctx, color_bg);



    #if defined(PBL_ROUND)
    GPoint s_center = {90,90};
    #else
    GPoint s_center = {72,84};
    #endif 
    graphics_context_set_fill_color(ctx, color_hands);
    graphics_fill_circle(ctx, s_center, s_radius+50);
    graphics_context_set_fill_color(ctx, color_bg);
    graphics_fill_circle(ctx, s_center, s_radius);

    graphics_context_set_fill_color(ctx, color_bubles);

    // marks
    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_stroke_color(ctx, color_hands);


    if(is_mn_marks){
      for (int minute=0;minute<60;minute++){
        #if defined(PBL_ROUND)

        GPoint mark = (GPoint) {
          .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius - 7) / TRIG_MAX_RATIO) + s_center.x,
          .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius - 7) / TRIG_MAX_RATIO) + s_center.y,
        };
        #else
        GPoint mark = (GPoint) {
          .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius - 3) / TRIG_MAX_RATIO) + s_center.x,
          .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius - 3) / TRIG_MAX_RATIO) + s_center.y,
        };

        #endif

        GPoint mark_end = (GPoint) {
          .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius) / TRIG_MAX_RATIO) + s_center.x,
          .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius) / TRIG_MAX_RATIO) + s_center.y,
        };
        graphics_context_set_stroke_width(ctx, 1);
        graphics_draw_line(ctx, mark, mark_end);
      }
    }  

    if(is_five_marks){
      for (int minute=0;minute<60;minute+=5){
        #if defined(PBL_ROUND)
        GPoint mark = (GPoint) {
          .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius - 7) / TRIG_MAX_RATIO) + s_center.x,
          .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius - 7) / TRIG_MAX_RATIO) + s_center.y,
        };
        #else
        GPoint mark = (GPoint) {
          .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius - 3) / TRIG_MAX_RATIO) + s_center.x,
          .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius - 3) / TRIG_MAX_RATIO) + s_center.y,
        };

        #endif

        GPoint mark_end = (GPoint) {
          .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius) / TRIG_MAX_RATIO) + s_center.x,
          .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * minute / 60) * (int32_t)(s_radius) / TRIG_MAX_RATIO) + s_center.y,
        };
        graphics_context_set_stroke_width(ctx, 5);
        graphics_draw_line(ctx, mark, mark_end);
      }
    }



    // 12h  mark
    #if defined(PBL_ROUND) 
    GPoint rect_twelve = {90, 0};
    graphics_fill_circle(ctx, rect_twelve, 25);
    graphics_context_set_fill_color(ctx, color_bubles);
    // 3h  mark
    rect_twelve.x= 180; 
    rect_twelve.y= 90;
    graphics_fill_circle(ctx, rect_twelve, 25);

    // 6h  mark
    rect_twelve.x= 90;
    rect_twelve.y= 180;
    graphics_fill_circle(ctx, rect_twelve, 25);
    // 9h  mark
    rect_twelve.x= 0;
    rect_twelve.y= 90;
    graphics_fill_circle(ctx, rect_twelve, 25);

    // 12h-3-6-9  mark
    graphics_context_set_fill_color(ctx, color_text);
    if(is_five_marks){
      rect_twelve.x= 90;
      rect_twelve.y= 0;
      graphics_fill_circle(ctx, rect_twelve, 6);
      rect_twelve.x= 0;
      rect_twelve.y= 90;
      graphics_fill_circle(ctx, rect_twelve, 6);
      rect_twelve.x= 180;
      rect_twelve.y= 90;
      graphics_fill_circle(ctx, rect_twelve, 6);
      rect_twelve.x= 90;
      rect_twelve.y= 180;
      graphics_fill_circle(ctx, rect_twelve, 6);
    }

    #else
    GPoint rect_twelve = {0, 0};
    graphics_fill_circle(ctx, rect_twelve, 35);
    graphics_context_set_fill_color(ctx, color_bubles);
    // 3h  mark
    rect_twelve.x= 144; 
    rect_twelve.y= 0;
    graphics_fill_circle(ctx, rect_twelve, 35);

    // 6h  mark
    rect_twelve.x= 0;
    rect_twelve.y= 168;
    graphics_fill_circle(ctx, rect_twelve, 35);
    // 9h  mark
    rect_twelve.x= 144;
    rect_twelve.y= 168;
    graphics_fill_circle(ctx, rect_twelve, 35);

    // 12h-3-6-9  mark
    graphics_context_set_fill_color(ctx, color_text);
    if(is_five_marks){
      rect_twelve.x= 0;
      rect_twelve.y= 0;
      graphics_fill_circle(ctx, rect_twelve, 6);
      rect_twelve.x= 144;
      rect_twelve.y= 0;
      graphics_fill_circle(ctx, rect_twelve, 6);
      rect_twelve.x= 0;
      rect_twelve.y= 168;
      graphics_fill_circle(ctx, rect_twelve, 6);
      rect_twelve.x= 144;
      rect_twelve.y= 168;
      graphics_fill_circle(ctx, rect_twelve, 6);
    }

    #endif


    // center
    graphics_context_set_fill_color(ctx, color_hands);
    graphics_fill_circle(ctx, s_center, 3);


    Time mode_time = s_last_time;

    // Adjust for minutes through the hour
    float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
    float hour_angle;

    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;

    hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

    // Plot hands
    GPoint minute_hand_start = (GPoint) {
      .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(13) / TRIG_MAX_RATIO) + s_center.x,
      .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(13) / TRIG_MAX_RATIO) + s_center.y,
    };
    GPoint minute_hand = (GPoint) {
      .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.x,
      .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.y,
    };
    GPoint hour_hand_start = (GPoint) {
      .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(13) / TRIG_MAX_RATIO) + s_center.x,
      .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(13) / TRIG_MAX_RATIO) + s_center.y,
    };

    GPoint hour_hand = (GPoint) {
      .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(s_radius - (RATIO_HM_HAND * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.x,
      .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius - (RATIO_HM_HAND * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.y,
    };
    GPoint hour_hand_hole = (GPoint) {
      .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(36) / TRIG_MAX_RATIO) + s_center.x,
      .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(36) / TRIG_MAX_RATIO) + s_center.y,
    };

    graphics_context_set_stroke_color(ctx, color_hands); 

    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, s_center, hour_hand_start);
    graphics_draw_line(ctx,s_center , minute_hand_start);

    graphics_context_set_stroke_width(ctx, HANDS_WIDTH);
    graphics_draw_line(ctx, hour_hand_start, hour_hand);


    graphics_context_set_stroke_color(ctx, color_bg);
    graphics_context_set_stroke_width(ctx, HANDS_WIDTH-5);
    graphics_draw_line(ctx, hour_hand_start, hour_hand_hole);
    
    graphics_context_set_fill_color(ctx, color_hands);
    graphics_context_set_stroke_width(ctx, HANDS_WIDTH);
    graphics_context_set_stroke_color(ctx, color_hands);
    graphics_draw_line(ctx, minute_hand_start, minute_hand);

    // Health
    HealthMetric metric = HealthMetricStepCount;

    time_t start = time_start_of_today();
    time_t end = time(NULL);
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, 
                                                                           start, end);

    char steps_buffer[20]=" ";
    char sleep_buffer[20]=" ";


    int steps=0;
    int ksteps=0;
    int dsteps=0;
    if(mask & HealthServiceAccessibilityMaskAvailable) {
      // Data is available!

      steps = (int)health_service_sum_today(metric);
      ksteps = (int)steps/1000;
      dsteps = (int)(steps - ksteps*1000)/100;


      snprintf(steps_buffer, sizeof(steps_buffer), "%i.%i", ksteps, dsteps);


      metric = HealthMetricSleepSeconds;
      mask = health_service_metric_accessible(metric,start, end);
      if(mask & HealthServiceAccessibilityMaskAvailable) {
        int seconds = (int)health_service_sum_today(metric);
        int hours=(int)seconds/3600;
        int minutes = ((int)seconds/60)%60;
        if (minutes>9){
          snprintf(sleep_buffer, sizeof(sleep_buffer), "%i:%i",hours, minutes);
        }
        else{
          snprintf(sleep_buffer, sizeof(sleep_buffer), "%i:0%i",hours, minutes);
        }

      }

    } else {
      // No data recorded yet today
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health data unavailable");
    }


    // Draw temperature conditions
    // Draw date
    if (!is_charging)
      snprintf(battery_buffer, sizeof(battery_buffer), "%i",(int)battery_level/10);
    else
      snprintf(battery_buffer, sizeof(battery_buffer), "C");


    if(!is_connected){

      graphics_context_set_stroke_color(ctx, color_hands); 
      graphics_fill_circle(ctx, s_center, 15);
    }

    #ifdef PBL_COLOR 
    if (is_photo_shooting){
      snprintf(steps_buffer, sizeof(steps_buffer), "10.2");
      snprintf(sleep_buffer, sizeof(sleep_buffer), "10.2");
    }
    #endif

    APP_LOG(APP_LOG_LEVEL_DEBUG,"passe ici draw text");

    graphics_draw_text(ctx, s_last_time.datetext, text_font, rect_text_date, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
    if(steps>400)
      graphics_draw_text(ctx, steps_buffer, text_font, rect_text_sleep, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
    else
      graphics_draw_text(ctx, sleep_buffer, text_font, rect_text_sleep, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 

    graphics_draw_text(ctx, temperature_buffer, text_font, rect_text_steps, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
    graphics_draw_text(ctx, battery_buffer, text_font, rect_text_bat, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 

  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {



  // Read tuples for data
  Tuple *vibration_tuple = dict_find(iterator, KEY_VIBRATION);
  Tuple *mn_marks_tuple = dict_find(iterator, KEY_MN_MARKS);
  Tuple *five_marks_tuple = dict_find(iterator, KEY_FIVE_MARKS);

  Tuple *color_r_bg_tuple = dict_find(iterator, KEY_COLOR_BG_R);
  Tuple *color_g_bg_tuple = dict_find(iterator, KEY_COLOR_BG_G);
  Tuple *color_b_bg_tuple = dict_find(iterator, KEY_COLOR_BG_B);

  Tuple *color_r_hands_tuple = dict_find(iterator, KEY_COLOR_HANDS_R);
  Tuple *color_g_hands_tuple = dict_find(iterator, KEY_COLOR_HANDS_G);
  Tuple *color_b_hands_tuple = dict_find(iterator, KEY_COLOR_HANDS_B);

  Tuple *color_r_bubles_tuple = dict_find(iterator, KEY_COLOR_BUBLES_R);
  Tuple *color_g_bubles_tuple = dict_find(iterator, KEY_COLOR_BUBLES_G);
  Tuple *color_b_bubles_tuple = dict_find(iterator, KEY_COLOR_BUBLES_B);

  Tuple *color_r_text_tuple = dict_find(iterator, KEY_COLOR_TEXT_R);
  Tuple *color_g_text_tuple = dict_find(iterator, KEY_COLOR_TEXT_G);
  Tuple *color_b_text_tuple = dict_find(iterator, KEY_COLOR_TEXT_B);

  Tuple *temperature_tuple = dict_find(iterator, KEY_TEMPERATURE);

  if(temperature_tuple){

    snprintf(temperature_buffer, sizeof(temperature_buffer), "%s", temperature_tuple->value->cstring);
    persist_write_string(KEY_TEMPERATURE, temperature_buffer);
    time_t t = time(NULL);
    struct tm now = *(localtime(&t)); 
    last_refresh = mktime(&now);
    persist_write_int(KEY_LAST_REFRESH, last_refresh);
  }

  int red;
  int green;
  int blue;
  // If all data is available, use it
  if(vibration_tuple && mn_marks_tuple && five_marks_tuple && color_r_bg_tuple && color_g_bg_tuple && color_b_bg_tuple 
     && color_r_hands_tuple && color_g_hands_tuple && color_b_hands_tuple && color_r_bubles_tuple
     && color_g_bubles_tuple && color_b_bubles_tuple && color_r_text_tuple && color_g_text_tuple && color_b_text_tuple
    ) {



    is_vibration =vibration_tuple->value->int32;
    persist_write_bool(KEY_VIBRATION, is_vibration);
    is_mn_marks =mn_marks_tuple->value->int32;
    persist_write_bool(KEY_MN_MARKS, is_mn_marks);
    is_five_marks =five_marks_tuple->value->int32;
    persist_write_bool(KEY_FIVE_MARKS, is_five_marks);

    red = color_r_bg_tuple->value->int32;
    green = color_g_bg_tuple->value->int32;
    blue = color_b_bg_tuple->value->int32;
    persist_write_int(KEY_COLOR_BG_R, red);
    persist_write_int(KEY_COLOR_BG_G, green);
    persist_write_int(KEY_COLOR_BG_B, blue);
    color_bg = GColorFromRGB(red, green, blue);

    red = color_r_hands_tuple->value->int32;
    green = color_g_hands_tuple->value->int32;
    blue = color_b_hands_tuple->value->int32;
    persist_write_int(KEY_COLOR_HANDS_R, red);
    persist_write_int(KEY_COLOR_HANDS_G, green);
    persist_write_int(KEY_COLOR_HANDS_B, blue);
    color_hands = GColorFromRGB(red, green, blue);

    red = color_r_bubles_tuple->value->int32;
    green = color_g_bubles_tuple->value->int32;
    blue = color_b_bubles_tuple->value->int32;
    persist_write_int(KEY_COLOR_BUBLES_R, red);
    persist_write_int(KEY_COLOR_BUBLES_G, green);
    persist_write_int(KEY_COLOR_BUBLES_B, blue);
    color_bubles = GColorFromRGB(red, green, blue);

    red = color_r_text_tuple->value->int32;
    green = color_g_text_tuple->value->int32;
    blue = color_b_text_tuple->value->int32;
    persist_write_int(KEY_COLOR_TEXT_R, red);
    persist_write_int(KEY_COLOR_TEXT_G, green);
    persist_write_int(KEY_COLOR_TEXT_B, blue);
    color_text = GColorFromRGB(red, green, blue);
  } 

  layer_mark_dirty(s_canvas_layer);  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}



static void createdatetext(void){
  if (strcmp(pebble_Lang, "en_US") == 0)
  {
    snprintf(s_last_time.datetext, sizeof(s_last_time.datetext), "%s\n%i", weekdayLangEn[s_last_time.wday],s_last_time.day);
    return; 
  }

  if (strcmp(pebble_Lang, "fr_FR") == 0)
  {
    snprintf(s_last_time.datetext, sizeof(s_last_time.datetext), "%s\n%i", weekdayLangFr[s_last_time.wday],s_last_time.day);
    return;
  }

  if (strcmp(pebble_Lang, "de_DE") == 0)
  {
    snprintf(s_last_time.datetext, sizeof(s_last_time.datetext), "%s\n%i", weekdayLangGe[s_last_time.wday],s_last_time.day);
    return;
  }

  if (strcmp(pebble_Lang, "es_ES") == 0)
  {
    snprintf(s_last_time.datetext, sizeof(s_last_time.datetext), "%s\n%i", weekdayLangSp[s_last_time.wday],s_last_time.day);
    return;
  }
  // Par defaut, anglais
  snprintf(s_last_time.datetext, sizeof(s_last_time.datetext), "%s\n%i", weekdayLangEn[s_last_time.wday],s_last_time.day);

}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {

  time_t t = time(NULL);
  struct tm now = *(localtime(&t)); 

  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;
  //APP_LOG(APP_LOG_LEVEL_DEBUG,"tick %d", s_last_time.minutes);

  if(is_photo_shooting){  
    s_last_time.hours =10;
    s_last_time.minutes = 10;
  }

  s_last_time.mounth = tick_time->tm_mon;
  s_last_time.day = tick_time->tm_mday;
  s_last_time.wday = tick_time->tm_wday;

  createdatetext(); 

  if (tick_time->tm_min == 0 && tick_time->tm_hour >= QUIET_TIME_END && tick_time->tm_hour <= QUIET_TIME_START&&is_vibration) {
    vibes_double_pulse();
  }

  if((now.tm_min % 30 == 0)&&(is_connected)) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);  

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0); 
    app_message_outbox_send(); 

  }
  else
    // Get weather update every 30 minutes
    if(((mktime(&now)-last_refresh)>duration)&&(is_connected)){
    snprintf(temperature_buffer, sizeof(temperature_buffer), " ");
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);  

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0); 
    app_message_outbox_send(); 
  }   
  // Redraw
  is_ready=true;
  layer_mark_dirty(s_canvas_layer);  
}

/* ****************************************************************** */
/* ****************************************************************** */
/* ****************************************************************** */


static void handle_battery(BatteryChargeState charge) {

  if((battery_level!=charge.charge_percent)||(is_charging!=charge.is_charging)){

    battery_level = charge.charge_percent;
    is_charging=charge.is_charging;

    layer_mark_dirty(s_canvas_layer);
  }
}

void bt_handler(bool connected) {

  if(connected!=is_connected){
    is_connected=connected; 
    // APP_LOG(APP_LOG_LEVEL_DEBUG,"dirty handler bt");
    layer_mark_dirty(s_canvas_layer);
  }
}

static void initBatteryLevel() {
  BatteryChargeState battery_state = battery_state_service_peek();
  battery_level = battery_state.charge_percent;
}

static void init() {

  is_ready=false;

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  snprintf(pebble_Lang, sizeof(pebble_Lang), "%s", i18n_get_system_locale());
  text_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  if(persist_exists(KEY_LAST_REFRESH)){
    last_refresh = persist_read_int(KEY_LAST_REFRESH);
  }

  if(persist_exists(KEY_TEMPERATURE)){
    persist_read_string(KEY_TEMPERATURE, temperature_buffer, sizeof(temperature_buffer));
  }

  if(persist_exists(KEY_VIBRATION)&& persist_exists(KEY_MN_MARKS)&& persist_exists(KEY_FIVE_MARKS)&&
     persist_exists(KEY_COLOR_BG_R)&&persist_exists(KEY_COLOR_BG_G)&&persist_exists(KEY_COLOR_BG_B)&&
     persist_exists(KEY_COLOR_HANDS_R)&&persist_exists(KEY_COLOR_HANDS_G)&&persist_exists(KEY_COLOR_HANDS_B)&&
     persist_exists(KEY_COLOR_BUBLES_R)&&persist_exists(KEY_COLOR_BUBLES_G)&&persist_exists(KEY_COLOR_BUBLES_B)&&
     persist_exists(KEY_COLOR_TEXT_R)&&persist_exists(KEY_COLOR_TEXT_G)&&persist_exists(KEY_COLOR_TEXT_B)){

    is_vibration=persist_read_bool(KEY_VIBRATION); 
    is_mn_marks=persist_read_bool(KEY_MN_MARKS); 
    is_five_marks=persist_read_bool(KEY_FIVE_MARKS); 

    int red;
    int green;
    int blue;

    red = persist_read_int(KEY_COLOR_BG_R);
    green = persist_read_int(KEY_COLOR_BG_G);
    blue = persist_read_int(KEY_COLOR_BG_B);
    color_bg=GColorFromRGB(red, green, blue);

    red = persist_read_int(KEY_COLOR_HANDS_R);
    green = persist_read_int(KEY_COLOR_HANDS_G);
    blue = persist_read_int(KEY_COLOR_HANDS_B);
    color_hands=GColorFromRGB(red, green, blue);

    red = persist_read_int(KEY_COLOR_BUBLES_R);
    green = persist_read_int(KEY_COLOR_BUBLES_G);
    blue = persist_read_int(KEY_COLOR_BUBLES_B);
    color_bubles=GColorFromRGB(red, green, blue);

    red = persist_read_int(KEY_COLOR_TEXT_R);
    green = persist_read_int(KEY_COLOR_TEXT_G);
    blue = persist_read_int(KEY_COLOR_TEXT_B);
    color_text=GColorFromRGB(red, green, blue);
  }
  else {
    color_hands = GColorBlack;
    color_text=GColorBlack;
    color_bg=GColorWhite;
    color_bubles=GColorWhite;

    is_vibration = true;
    is_mn_marks=true;
    is_five_marks=true;
  }
  s_main_window = window_create();

  s_main_window = window_create();
  window_stack_push(s_main_window, true);

  s_canvas_layer = window_get_root_layer(s_main_window);

  layer = layer_create(layer_get_bounds(s_canvas_layer));
  layer_set_update_proc(layer, update_proc);
  layer_add_child(s_canvas_layer, layer);
 
  BatteryChargeState charge_state = battery_state_service_peek();
  is_charging = charge_state.is_charging;
  is_connected = connection_service_peek_pebble_app_connection();

  initBatteryLevel();  

  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(bt_handler);   

  //JS Messages

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  //
  app_message_open(1500, 1500);


  //  layer_mark_dirty(s_canvas_layer);
  app_focus_service_subscribe_handlers((AppFocusHandlers){
    .did_focus = app_focus_changed,
    .will_focus = app_focus_changing
  });
}

static void deinit() {

  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();  
  window_destroy(s_main_window);

}

int main() {
  init();
  app_event_loop();
  deinit();
}
