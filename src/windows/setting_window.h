/*******************************************************************************
 * FILENAME :        setting_window.h
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

#pragma once

#include <pebble.h>


/*******************************************************************************
 * CALLBACK DECLARATIONS
 */

/*
 * Callback:    SettingWindowComplete
 * ----------------------------------
 * called when the user is done setting the time
 */

typedef void (*SettingWindowComplete)(int64_t time_duration, void *context);



/*
 * Structure:   SettingWindowCallbacks
 * ----------------------------------
 * structure containing all SettingWindow callbacks
 */

typedef struct SettingWindowCallbacks {
    SettingWindowComplete setting_complete;
} SettingWindowCallbacks;



/*******************************************************************************
 * STRUCTURE DECLARATION
 */

/*
 * Structure:   SettingWindow
 * -----------------------
 * main structure containing all data for a SettingWindow
 */

typedef struct SettingWindow SettingWindow;



/*******************************************************************************
 * API FUNCTIONS
 */

/*
 * Function:    setting_window_create
 * -------------------------------
 * creates a new SettingWindow in memory but does not push it into view
 *
 *  setting_window_callbacks: callbacks for communication
 *
 *  returns: a pointer to a new SettingWindow structure
 */

SettingWindow *setting_window_create(
                    SettingWindowCallbacks setting_window_callbacks);



/*
 * Function:    setting_window_destroy
 * ----------------------------------
 * destroys an existing SettingWindow
 *
 *  setting_window: a pointer to the SettingWindow being destroyed
 */

void setting_window_destroy(SettingWindow *setting_window);



/*
 * Function:    setting_window_push
 * -------------------------------
 * push the window onto the stack
 *
 *  setting_window: a pointer to the SettingWindow being pushed
 *  animated: whether to animate the push or not
 */

void setting_window_push(SettingWindow *setting_window, bool animated);



/*
 * Function:    setting_window_pop
 * ------------------------------
 * pop the window off the stack
 *
 *  setting_window: a pointer to the SettingWindow to pop
 *  animated: whether to animate the pop or not
 */

void setting_window_pop(SettingWindow *setting_window, bool animated);



/*
 * Function:    setting_window_get_topmost_window
 * ---------------------------------------------
 * gets whether it is the topmost window or not
 *
 *  setting_window: a pointer to the SettingWindow being checked
 *
 *  returns: a boolean indicating if it is the topmost window
 */

bool setting_window_get_topmost_window(SettingWindow *setting_window);



/*
 * Function:    setting_window_set_highlight_color
 * --------------------------------------------
 * sets the over-all color scheme of the window
 *
 *  color: the GColor to set the highlight to
 */

void setting_window_set_highlight_color(SettingWindow *setting_window,
                                        GColor color);