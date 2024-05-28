#include <stdio.h>
#include <stdint.h>
#include "gui/gui.h"
#include "FSM.h"
#include <gtk/gtk.h>
#include <time.h>
#include "logging/logging.h"
/*********************** MACRO definition***********************/
#define TIMER_INTERVAL_SEC 600	// 10 minutes in seconds
/*********************** Static variables definition***********************/
static State_t current_state; // Static variable to hold current state
static time_t timer_start_time = 0; // Global variable to store the start time of the timer
/*********************** Static Function declaration***********************/
static void start_or_reset_timer();
static void close_all_windows(void);
static gboolean  timer_callback(gpointer data);
static void fsm_init(void);
/*********************** Static Function definition***********************/
/*
*   Description:
*       Resets the start time to be checked by the timer callback
*   In: 
*       void
*   Out:
*       void
*/
static void start_or_reset_timer(void) {
    timer_start_time = time(NULL);
}

/*
*   Description:
*       closes all active windows
*   In: 
*       void
*   Out:
*       void
*/
static void close_all_windows(void) {
    GList *windows = gtk_window_list_toplevels(); // Get a list of all top-level windows
    GList *iter;
    for (iter = windows; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *window = GTK_WIDGET(iter->data);
        gtk_widget_destroy(window); // Destroy the window
    }
    g_list_free(windows); // Free the list
}

/*
*   Description:
*       Resets the start time to be checked by the timer callback
*   In: 
*       gboolean: TRUE means the timer will start again
*				  FALSE means the timer will stop
*   Out:
*       data: unused input
*/
static gboolean timer_callback(gpointer data)
{
	(void)data;
    time_t current_time = time(NULL);
    if (current_time - timer_start_time >= TIMER_INTERVAL_SEC) {
        // 5 minutes have elapsed, execute the specific function
        log_message("5 minutes have passed. Execute the specific function here.\n");
        
        // Close all GTK windows
        close_all_windows();

        // Reset the timer
        start_or_reset_timer();
		//Start the FSM from INIT state
		fsm_init();
		fsm_handle_event(EVENT_PROCEED);
    }
    // Keep the timer running
    return TRUE;
}

/*
*   Description:
*       Initializes the needed modules
*   In: 
*       void
*   Out:
*       void
*/
static void fsm_init(void) {
    // Set initial state
    current_state = STATE_INIT;
}

/*
*   Description:
*       performs the transition between states
*   In: 
*       new_state: indicates the new state of the FSM
*   Out:
*       void
*/
static void fsm_transition(State_t new_state) {
    // Perform exit actions from current state
    switch (current_state) {
        case STATE_INIT:
            
            break;
        case STATE_WELCOME:
            // Perform exit actions for STATE_WELCOME state
            break;
        case STATE_PASSWORD:
            // Perform exit actions for STATE_PASSWORD state
            break;
        case STATE_INSTRUCTIONS:
            // Perform exit actions for STATE_INSTRUCTIONS state
            break;
        case STATE_SCAN:
            // Perform exit actions for STATE_SCAN state
            break;
        case STATE_CONFIRM_PIC:
            // Perform exit actions for STATE_CONFIRM_PIC state
            break;
        case STATE_WEEK_NUMBER:
            // Perform exit actions for STATE_WEEK_NUMBER state
            break;
        case STATE_SEND_PIC:
            // Perform exit actions for STATE_SEND_PIC state
            break;
        case STATE_STATUS:
            // Perform exit actions for STATE_STATUS state
            break;
        default:
            // Handle unexpected state
            break;
    }

    // Update current state
    current_state = new_state;

    // Perform entry actions for new state
    switch (current_state) {
        case STATE_INIT:
            // Perform entry actions for STATE_INIT state
            break;
        case STATE_WELCOME:
            gui_create_welcome_screen();
			start_or_reset_timer();
            break;
        case STATE_PASSWORD:
            gui_numpad_screen(STATE_PASSWORD);
			start_or_reset_timer();
            break;
        case STATE_INSTRUCTIONS:
            gui_create_instructions_screen();
			start_or_reset_timer();
            break;
        case STATE_SCAN:
            gui_scan_model();
			start_or_reset_timer();
            break;
        case STATE_CONFIRM_PIC:
            gui_display_image();
			start_or_reset_timer();
            break;
        case STATE_WEEK_NUMBER:
			gui_numpad_screen(STATE_WEEK_NUMBER);
			start_or_reset_timer();
            break;
        case STATE_SEND_PIC:
            gui_display_loading_screen();
			start_or_reset_timer();
            break;
        case STATE_STATUS:
            gui_create_status_screen();
			start_or_reset_timer();
            break;
        default:
            // Handle unexpected state
            break;
    }
}

/*********************** Static Function definition***********************/
void fsm_handle_event(Event_t event) {
    switch (current_state) {
        case STATE_INIT:
            switch (event) {
                case EVENT_PROCEED:
                    fsm_transition(STATE_WELCOME);
                    break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        case STATE_WELCOME:
            switch (event) {
                case EVENT_PROCEED:
                    fsm_transition(STATE_PASSWORD);
                    break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        case STATE_PASSWORD:
            switch (event) {
                case EVENT_PROCEED:
                    fsm_transition(STATE_INSTRUCTIONS);
                    break;
                case EVENT_LOG_OUT:
                    fsm_transition(STATE_WELCOME);
                    break;
                case EVENT_WRONG_PASSWORD:
                    fsm_transition(STATE_STATUS);
                    break;
				case EVENT_BACK_END_ERROR:
					fsm_transition(STATE_STATUS);
					break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        case STATE_INSTRUCTIONS:
            switch (event) {
                case EVENT_PROCEED:
                    fsm_transition(STATE_SCAN);
                    break;
                case EVENT_LOG_OUT:
                    fsm_transition(STATE_WELCOME);
                    break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        case STATE_SCAN:
            switch (event) {
                case EVENT_PROCEED:
                    fsm_transition(STATE_CONFIRM_PIC);
                    break;
				case EVENT_SCAN_ERROR:
					fsm_transition(STATE_STATUS);
                    break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        case STATE_CONFIRM_PIC:
            switch (event) {
                case EVENT_PROCEED:
                    fsm_transition(STATE_WEEK_NUMBER);
                    break;
                case EVENT_LOG_OUT:
                    fsm_transition(STATE_WELCOME);
                    break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        case STATE_WEEK_NUMBER:
            switch (event) {
                case EVENT_PROCEED:
                    fsm_transition(STATE_SEND_PIC);
                    break;
                case EVENT_LOG_OUT:
                    fsm_transition(STATE_WELCOME);
                    break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        case STATE_SEND_PIC:
            switch (event) {
                case EVENT_PROCEED:
                    fsm_transition(STATE_STATUS);
                    break;
				case EVENT_BACK_END_ERROR:
                    fsm_transition(STATE_STATUS);
                    break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        case STATE_STATUS:
            switch (event) {
                case EVENT_LOG_OUT:
                    fsm_transition(STATE_WELCOME);
                    break;
                default:
                    // Handle unexpected event
                    break;
            }
            break;
        default:
            // Handle unexpected state
            break;
    }
}

int main() {
	init_logging();
	log_message("*********** POWER UP ***********\n");
	(void)gui_init();
	//Start a timer to logout the user if inactive for TIMER_INTERVAL_SEC seconds in the same window
    g_timeout_add_seconds(TIMER_INTERVAL_SEC, timer_callback, NULL);
	start_or_reset_timer();
	
    fsm_init();

    fsm_handle_event(EVENT_PROCEED);
    
    (void)gui_run();
    return 0;
}
