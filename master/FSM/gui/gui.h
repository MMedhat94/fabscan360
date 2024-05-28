#ifndef GUI_H
#define GUI_H
/*********************** Includes ***********************/
#include "../FSM.h"
#include "../FSM_types.h"

/*********************** Function definition***********************/
/*
*   Description:
*       Initializes gtk
*   In: 
*       void
*   Out:
*       E_OK: Init passed
*       E_NOK: Init failed
*/
extern Return_t gui_init(void);

/*
*   Description:
*       runs main function of gtk
*   In: 
*       void
*   Out:
*       E_OK: main function is running
*       E_NOK: main function returned with an error
*/
extern Return_t gui_run(void);

/*
*   Description:
*       Creates the first welcome screen to the user
*   In: 
*       void
*   Out:
*       void
*/
extern void gui_create_welcome_screen(void);

/*
*   Description:
*       Creates the password screen to the user
*   In: 
*       state: indicates the current state
*   Out:
*       void
*/
extern void gui_numpad_screen(State_t state);

/*
*   Description:
*       Creates the instructions screen to the user
*   In: 
*       void
*   Out:
*       void
*/
extern void gui_create_instructions_screen(void);

/*
*   Description:
*       Creates the screen for wrong pasword
*   In: 
*       void
*   Out:
*       void
*/

extern void gui_display_image(void);

/*
*   Description:
*       Creates the loading screen for scanning the model
*       And calls socket and motor modules to start the scanning
*   In: 
*       void
*   Out:
*       void
*/
extern void gui_scan_model(void);

/*
*   Description:
*       Creates the loading screen for sending the images
*   In: 
*       void
*   Out:
*       void
*/
extern void gui_display_loading_screen(void);

/*
*   Description:
*       Creates the status screen where it display the internal variable carrying the latest status message
*   In: 
*       void
*   Out:
*       void
*/
extern void gui_create_status_screen(void);
#endif /* GUI_H */