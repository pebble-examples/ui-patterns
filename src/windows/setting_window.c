/*******************************************************************************
 * FILENAME :        setting_window.c
 *
 * DESCRIPTION :
 *      Setting screen to select a time duration
 *
 * NOTES :      NA
 *
 * AUTHOR :    Eric Phillips        START DATE :    07/12/15
 *
 * CHANGES :    NA
 *
 */

 #include <pebble.h>
 #include "setting_window.h"
 #include "../layers/selection_layer.h"

 #define TIMER_MINIMUM_DURATION 5000
 #define TIMELINE_MINIMUM_DURATION 900000


/*******************************************************************************
 * STRUCTURE DEFINITION
 */

/*
 * the structure of a SettingWindow
 */

struct SettingWindow {
    Window *window;
    TextLayer *main_text, *sub_text;
    Layer *selection;
    GColor highlight_color;
#ifdef PBL_SDK_3
    StatusBarLayer *status;
#endif
    SettingWindowCallbacks callbacks;

    int32_t field_values[3];
    char field_buffs[3][3];
    int8_t field_selection;
};



/*******************************************************************************
 * PRIVATE FUNCTIONS
 */

/*
 * update the sub text
 *
 * changes what is displayed in the sub text to different things, like
 * "Timer too short" or "End: 12:43"
 */

void update_sub_text(SettingWindow *setting_window) {
    int64_t duration = (int64_t)setting_window->field_values[0] * 3600000 +
        (int64_t)setting_window->field_values[1] * 60000 +
        (int64_t)setting_window->field_values[2] * 1000;
    // check duration
    if (duration < TIMER_MINIMUM_DURATION){
        text_layer_set_text(setting_window->sub_text, "Timer too short");
        layer_set_hidden(text_layer_get_layer(setting_window->sub_text), false);
        return;
    }
    else if (duration < TIMELINE_MINIMUM_DURATION){
        layer_set_hidden(text_layer_get_layer(setting_window->sub_text), true);
        return;
    }
    else
        layer_set_hidden(text_layer_get_layer(setting_window->sub_text), false);

    // format into time parts
    time_t end = ((int64_t)time(NULL) + (int64_t)time_ms(NULL, NULL) + duration)
                                                                    / 1000;
    static char buff[] = "End: 00:00 AM";
    struct tm *tick_time = localtime(&end);
    if (clock_is_24h_style())
        strftime(buff, sizeof(buff), "End: %k:%M", tick_time);
    else
        strftime(buff, sizeof(buff), "End: %l:%M %p", tick_time);
    // set text
    text_layer_set_text(setting_window->sub_text, buff);
}


/*******************************************************************************
 * CALLBACKS
 */

/*
 * selection layer get text callback
 */

static char* selection_handle_get_text(unsigned index, void *context) {
    SettingWindow *setting_window = (SettingWindow*)context;
    snprintf(setting_window->field_buffs[index],
        sizeof(setting_window->field_buffs[0]), "%02d",
        (int)setting_window->field_values[index]);
    return setting_window->field_buffs[index];
}



/*
 * selection layer complete callback
 */

static void selection_handle_complete(void *context) {
    SettingWindow *setting_window = (SettingWindow*)context;
    int64_t duration = (int64_t)setting_window->field_values[0] * 3600000 +
                      (int64_t)setting_window->field_values[1] * 60000 +
                      (int64_t)setting_window->field_values[2] * 1000;
    // call complete callback
    setting_window->callbacks.setting_complete(duration, setting_window);
}



/*
 * selection layer increment up callback
 */

static void selection_handle_inc(unsigned index, uint8_t clicks,
                                                    void *context) {
    SettingWindow *setting_window = (SettingWindow*)context;
    setting_window->field_values[index] += (clicks > 10) ? 2 : 1;
    if (setting_window->field_values[index] >= 60)
        setting_window->field_values[index] -= 60;
    // update text
    update_sub_text(setting_window);
}



/*
 * selection layer increment down callback
 */

static void selection_handle_dec(unsigned index, uint8_t clicks,
                                                    void *context) {
    SettingWindow *setting_window = (SettingWindow*)context;
    setting_window->field_values[index] -= (clicks > 10) ? 2 : 1;
    if (setting_window->field_values[index] < 0)
        setting_window->field_values[index] += 60;
    // update text
    update_sub_text(setting_window);
}



/*******************************************************************************
 * API FUNCTIONS
 */

/*
 * create a new SettingWindow and return a pointer to it
 * this includes creating all its children layers but
 * does not push it onto the window stack
 */

SettingWindow *setting_window_create(
                            SettingWindowCallbacks setting_window_callbacks) {
    SettingWindow *setting_window =
                            (SettingWindow*)malloc(sizeof(SettingWindow));
    if (setting_window != NULL) {
        setting_window->window = window_create();
        setting_window->callbacks = setting_window_callbacks;
        if (setting_window->window != NULL) {
            // zero some values
            setting_window->field_selection = 0;
            // get window parameters
            Layer *root = window_get_root_layer(setting_window->window);
            GRect bounds = layer_get_frame(root);
            // main text
#ifdef PBL_SDK_3
            setting_window->main_text = text_layer_create(
                GRect(0, 30, bounds.size.w, 40));
#else
            setting_window->main_text = text_layer_create(
                GRect(0, 15, bounds.size.w, 40));
#endif
            text_layer_set_text(setting_window->main_text, "Set Timer");
            text_layer_set_font(setting_window->main_text,
                fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
            text_layer_set_text_alignment(setting_window->main_text,
                GTextAlignmentCenter);
            layer_add_child(root,
                text_layer_get_layer(setting_window->main_text));
            // sub text
#ifdef PBL_SDK_3
            setting_window->sub_text = text_layer_create(
                GRect(1, 125, bounds.size.w, 40));
#else
            setting_window->sub_text = text_layer_create(
                GRect(1, 110, bounds.size.w, 40));
#endif
            text_layer_set_text(setting_window->sub_text, "Timer too short");
            text_layer_set_text_alignment(setting_window->sub_text,
                GTextAlignmentCenter);
            text_layer_set_font(setting_window->sub_text,
                fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
            layer_add_child(root,
                text_layer_get_layer(setting_window->sub_text));
            // create selection layer
            uint8_t num_cells = 3;
#ifdef PBL_SDK_3
            setting_window->selection = selection_layer_create(
                GRect(8, 75, 128, 34), num_cells);
#else
            setting_window->selection = selection_layer_create(
                GRect(8, 60, 128, 34), num_cells);
#endif
            for (int i = 0; i < num_cells; i++)
                selection_layer_set_cell_width(setting_window->selection, i,
                                                                            40);
            selection_layer_set_cell_padding(setting_window->selection, 4);
#ifdef PBL_COLOR
            selection_layer_set_active_bg_color(setting_window->selection,
                GColorRed);
            selection_layer_set_inactive_bg_color(setting_window->selection,
                GColorDarkGray);
#endif
            selection_layer_set_click_config_onto_window(
                setting_window->selection, setting_window->window);
            selection_layer_set_callbacks(setting_window->selection,
                setting_window, (SelectionLayerCallbacks) {
                    .get_cell_text = selection_handle_get_text,
                    .complete = selection_handle_complete,
                    .increment = selection_handle_inc,
                    .decrement = selection_handle_dec,
                });
            layer_add_child(window_get_root_layer(setting_window->window),
                setting_window->selection);

#ifdef PBL_SDK_3
            // create status bar
            setting_window->status = status_bar_layer_create();
            status_bar_layer_set_colors(setting_window->status,
                GColorClear, GColorBlack);
            layer_add_child(root,
                status_bar_layer_get_layer(setting_window->status));
#endif
            return setting_window;
        }
    }
    // error handling
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create SettingWindow");
    return NULL;
}



/*
 * destroy a previously created SettingWindow
 */

void setting_window_destroy(SettingWindow *setting_window) {
    if (setting_window != NULL) {
#ifdef PBL_SDK_3
        status_bar_layer_destroy(setting_window->status);
#endif
        selection_layer_destroy(setting_window->selection);
        text_layer_destroy(setting_window->sub_text);
        text_layer_destroy(setting_window->main_text);
        free(setting_window);
        setting_window = NULL;
        return;
    }
    // error handling
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted to free NULL SettingWindow");
}



/*
 * push the window onto the stack
 */

void setting_window_push(SettingWindow *setting_window, bool animated) {
    window_stack_push(setting_window->window, animated);
}



/*
 * pop the window off the stack
 */

void setting_window_pop(SettingWindow *setting_window, bool animated) {
    window_stack_remove(setting_window->window, animated);
}



/*
 * gets whether it is the topmost window on the stack
 */

bool setting_window_get_topmost_window(SettingWindow *setting_window) {
    return window_stack_get_top_window() == setting_window->window;
}



/*
 * set highlight color of this window
 * this is the overall color scheme used
 */

void setting_window_set_highlight_color(SettingWindow *setting_window,
                                                            GColor color) {
    setting_window->highlight_color = color;
    selection_layer_set_active_bg_color(setting_window->selection, color);
}