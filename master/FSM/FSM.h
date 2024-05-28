#ifndef FSM_H
#define FSM_H

#include "FSM_types.h"

/*********************** MACRO definition***********************/
#define NUMBER_OF_PICTURES  9U
_Static_assert(NUMBER_OF_PICTURES % 3 == 0, "NUMBER_OF_PICTURES must be divisible by 3");
/*********************** type definition***********************/
// Define states
typedef enum {
    STATE_INIT,
    STATE_WELCOME,
    STATE_PASSWORD,
    STATE_INSTRUCTIONS,
    STATE_SCAN,
    STATE_CONFIRM_PIC,
    STATE_WEEK_NUMBER,
    STATE_SEND_PIC,
    STATE_STATUS
} State_t;

// Define events
typedef enum {
    EVENT_PROCEED,
    EVENT_LOG_OUT,
    EVENT_WRONG_PASSWORD,
	EVENT_BACK_END_ERROR,
	EVENT_SCAN_ERROR
} Event_t;

/*********************** Function declarations ***********************/
/*
*   Description:
*       perform an event for the FSM
*   In: 
*       event: indicates the event
*   Out:
*       void
*/
extern void fsm_handle_event(Event_t event);

#endif /* FSM_H */