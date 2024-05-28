#include <gtk/gtk.h>
#include "gui.h"
#include "../backend/backend.h"
#include "../sock/sock.h"
#include <string.h>
#include "../motor/motor.h"
#include "logging/logging.h"
/*********************** Macro definition***********************/
#define NUMBER_OF_BOARDS    3U
#define CAPTURE_WAIT_TIME   15U
#define UPLOAD_WAIT_TIME	3U
#define DEFAULT_FONT        "Garamond"
#define DEFAULT_SIZE        30
#define INSTRUCTIONS_SIZE   20
#define STATUS_BODY_SIZE   	30
/*********************** Static variables definition***********************/

// Static variable to store the entered password
static char entered_password[PASSWORD_LENGTH + 1] = "";

// Static variable to store the entered week number
static char entered_week[WEEK_LENGTH + 1] = "";

// Static variable for password label
static GtkWidget *numpad_label;

// Allocate memory for image_paths
static char **image_paths=(char **)NULL;

// Global variables
static int current_image_index = 0; // Index of the currently displayed image

// Global variables
static struct
{
	char *title;
	char *body;
}status_message;
/*********************** Static type definition***********************/
/* To be used to communicate between the progress bar function and the model scan function */
typedef struct ProgressData 
{
    GtkProgressBar *progressbar;
    GtkWidget *window;
}progress_t;
/*********************** Static Function definition***********************/
/*
*   Description:
*       Destroys all the children of a widget including the widget itself
*   In: 
*       pointer to GtkWidget
*   Out:
*       void
*/
static void destroy_orphans_recursive(GtkWidget *widget) {
    if (widget == NULL)
        return;

    // Get the widget's parent
    GtkWidget *parent = gtk_widget_get_parent(widget);

    // Check if the widget has a parent
    if (parent == NULL) {
        // If there's no parent, destroy the widget
        gtk_widget_destroy(widget);
    } else {
        // If the widget has children, traverse them recursively
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
        GList *iter;
        for (iter = children; iter != NULL; iter = g_list_next(iter)) {
            GtkWidget *child = GTK_WIDGET(iter->data);
            destroy_orphans_recursive(child);
        }
        g_list_free(children);
    }
}

/*
*   Description:
*       Function to be executed when the user presses PROCEED 
*   In: 
*       widget: A self pointer to the widget itself
*       data  : gpointer to data to be used in the call back
*   Out:
*       void
*/
static void gui_proceed_button(GtkWidget *widget, gpointer data)
{
    (void)widget;
    log_message("GTK proceed button pressed!\n");
    GtkWidget *previous_window = GTK_WIDGET(data);
    fsm_handle_event(EVENT_PROCEED);
    destroy_orphans_recursive(previous_window);
}

/*
*   Description:
*       Function to be executed when the user presses Log out
*   In: 
*       widget: A self pointer to the widget itself
*       data  : gpointer to data to be used in the call back
*   Out:
*       void
*/
static void gui_logout_button(GtkWidget *widget, gpointer data)
{
    (void)widget;
    log_message("GTK log out button pressed!\n");
    GtkWidget *previous_window = GTK_WIDGET(data);
    fsm_handle_event(EVENT_LOG_OUT);
    destroy_orphans_recursive(previous_window);
}

/*
*   Description:
*       Callback function for when a number button is clicked
*   In: 
*       widget: A self pointer to the widget itself 
*       data  : gpointer to data to be used in the call back 
*   Out:
*       void
*/
static void password_button_clicked(GtkWidget *widget, gpointer data) {
    (void)data;
    // Get the button's label (which represents the number)
    const char *number = gtk_button_get_label(GTK_BUTTON(widget));
    uint8 input_length = strlen(entered_password);

    if( input_length >=PASSWORD_LENGTH)
    {
        // Clear the entered password
        entered_password[0] = '\0';
    }
    else
    {
        // Append the number to the entered password
        strcat(entered_password, number);
    }
        // Update the password label
        gtk_label_set_text(GTK_LABEL(numpad_label), entered_password);
}

/*
*   Description:
*       Callback function for when the clear button is clicked
*   In: 
*       widget: A self pointer to the widget itself
*       data  : gpointer to data to be used in the call back
*   Out:
*       void
*/
static void password_clear_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    // Clear the entered password
    entered_password[0] = '\0';
    
    // Update the password label
    gtk_label_set_text(GTK_LABEL(numpad_label), entered_password);
}

/*
*   Description:
*       Callback function for when the enter button is clicked
*   In: 
*       widget: A self pointer to the widget itself
*       data  : gpointer to data to be used in the call back
*   Out:
*       void
*/
static void password_enter_clicked(GtkWidget *widget, gpointer data) {
    Return_t ret = E_NOK;
    (void)widget;
    GtkWidget *previous_window = GTK_WIDGET(data);
    ret = backend_password_check(entered_password);
    // Check if the entered password is correct
    if (E_OK == ret) {
        // Password is correct
        log_message("Correct password entered!\n");
        // Add code here to handle the correct password case (e.g., open a new window)
        fsm_handle_event(EVENT_PROCEED);
        destroy_orphans_recursive(previous_window);
        // Clear the entered password
        entered_password[0] = '\0';
    } else if (E_BACKEND_WRONG_PASSWORD == ret)
    {
        // Password is incorrect
        log_message("Incorrect password entered: %s\n", entered_password);
        // Clear the entered password
        entered_password[0] = '\0';
        // Update the password label
        gtk_label_set_text(GTK_LABEL(numpad_label), entered_password);
		status_message.title = "Wrong password";
		status_message.body = "Press the button below to return to home page!";
        fsm_handle_event(EVENT_WRONG_PASSWORD);
        destroy_orphans_recursive(previous_window);
    }
    else 
    {
        /* TODO: Backend connection error*/
        log_message("Backend connection error while trying to fetch team data\n");
		status_message.title = "Backend connection error";
		status_message.body = "Press the button below to return to home page!";
        fsm_handle_event(EVENT_BACK_END_ERROR);
        destroy_orphans_recursive(previous_window);
    }
}

/*
*   Description:
*       Callback function for when a number button is clicked
*   In: 
*       widget: A self pointer to the widget itself 
*       data  : gpointer to data to be used in the call back 
*   Out:
*       void
*/
static void week_button_clicked(GtkWidget *widget, gpointer data) {
    (void)data;
    // Get the button's label (which represents the number)
    const char *number = gtk_button_get_label(GTK_BUTTON(widget));
    uint8 input_length = strlen(entered_week);

    if( input_length >=WEEK_LENGTH)
    {
        // Clear the entered week
        entered_week[0] = '\0';
    }
    else
    {
        // Append the number to the entered week
        strcat(entered_week, number);
    }
        // Update the week label
        gtk_label_set_text(GTK_LABEL(numpad_label), entered_week);
}

/*
*   Description:
*       Callback function for when the clear button is clicked
*   In: 
*       widget: A self pointer to the widget itself
*       data  : gpointer to data to be used in the call back
*   Out:
*       void
*/
static void week_clear_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    // Clear the entered week
    entered_week[0] = '\0';
    
    // Update the week label
    gtk_label_set_text(GTK_LABEL(numpad_label), entered_week);
}

/*
*   Description:
*       Callback function for when the enter button is clicked
*   In: 
*       widget: A self pointer to the widget itself
*       data  : gpointer to data to be used in the call back
*   Out:
*       void
*/
static void week_enter_clicked(GtkWidget *widget, gpointer data) {
    Return_t ret = E_NOK;
    (void)widget;
    GtkWidget *previous_window = GTK_WIDGET(data);
    ret = backend_set_week_number(entered_week);
    // Check if the entered week is correct
    if (E_OK == ret) {
        // week is correct
        log_message("Valid week entered!\n");
        // Add code here to handle the correct week case (e.g., open a new window)
        fsm_handle_event(EVENT_PROCEED);
        destroy_orphans_recursive(previous_window);
        // Clear the entered week
        entered_week[0] = '\0';
    } else
    {
        // week is invalid
        log_message("Invalid week entered: %s\n", entered_week);
        // Clear the entered week
		entered_week[0] = '\0';
		// Update the week label
		gtk_label_set_text(GTK_LABEL(numpad_label), entered_week);
    }
}

/*
*   Description:
*       customize the font and size for any widget
*       Any type of widget should be added to the if branches in the function
*   In: 
*       widget:         A self pointer to the widget itself
*       font_family:    The font to be set
*       font_size:      The font size to be set
*   Out:
*       void
*/
static void set_font_and_size(GtkWidget *widget, const gchar *font_family, gint font_size) {
    // Apply CSS to change font family and size
    GtkCssProvider *provider = gtk_css_provider_new();
    gchar *css_selector;
    if (GTK_IS_LABEL(widget)) {
        css_selector = g_strdup("label");
    } else if (GTK_IS_BUTTON(widget)) {
        css_selector = g_strdup("button");
    } else {
        css_selector = g_strdup("widget"); // Default selector for other types of widgets
    }
    gchar *css_data = g_strdup_printf(
        "%s {"
        "   font-family: %s;"
        "   font-size: %dpx;"
        "}", css_selector, font_family, font_size);
    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);
    g_free(css_data);

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    g_free(css_selector);
}

static void set_color(GtkWidget *widget) {
    // Apply CSS to change font family and size
    GtkCssProvider *provider = gtk_css_provider_new();
    if (GTK_IS_WINDOW(widget)) {
        gtk_css_provider_load_from_data(provider,
        "window {"
		"   background-color: BurlyWood;"
        "}"
        , -1, NULL);
    } else if (GTK_IS_BUTTON(widget)) {
		gtk_css_provider_load_from_data(provider,
        "button {"
        "   border-radius: 20px;"
		"   background-color: SandyBrown;"
        "}"
        , -1, NULL);
    }
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static gboolean upload_images(gpointer data) {
	Return_t ret = E_NOK;
	static uint8 pic_number = 0;
    // Cast the data pointer to a structure that contains both the progress bar and the window
    progress_t *progress_data = data;

    // Get the current percentage
    gdouble percentage = gtk_progress_bar_get_fraction(progress_data->progressbar);

    // Increase the percentage
    percentage += 1.0/NUMBER_OF_PICTURES;

    // Update the progress bar with the new percentage
    gtk_progress_bar_set_fraction(progress_data->progressbar, percentage);
    log_message("Percentage = %f \n", (float)percentage*100);

	// Check if image_paths is not NULL and contains valid file paths
	if (image_paths != NULL) {
		if (pic_number < NUMBER_OF_PICTURES) {
			// Call backend_send_image for each file name
			ret = backend_send_image(image_paths[pic_number]);
			// Check the result and handle errors if necessary
			if (ret != E_OK) {
				log_message("Error sending image: %s\n", image_paths[pic_number]);
				status_message.title = "Problem occured";
				status_message.body = "Backend connection failed!";
				fsm_handle_event(EVENT_BACK_END_ERROR);
				// Destroy the window when the progress reaches 100%
				destroy_orphans_recursive(progress_data->window);
				g_free(progress_data); // Free the memory allocated for the structure
				pic_number = 0;
				return FALSE;
				// Handle error accordingly
			} else {
				log_message("Image sent successfully: %s\n", image_paths[pic_number]);
				pic_number++;
			}
		}
	} else {
		log_message("Error: image_paths is NULL\n");
		status_message.title = "Problem occured";
		status_message.body = "Images not found!";
		fsm_handle_event(EVENT_BACK_END_ERROR);
		// Destroy the window when the progress reaches 100%
		destroy_orphans_recursive(progress_data->window);
		g_free(progress_data); // Free the memory allocated for the structure
		pic_number = 0;
		return FALSE;
		// Handle error accordingly
	}
    
    // If the percentage reaches or exceeds 100%, return FALSE to stop the timer
    if (percentage >= 1.0) {
		log_message("100 percent reached");
        status_message.title = "Thank you!";
		status_message.body = "The images have been uploaded successfully";
		fsm_handle_event(EVENT_PROCEED);
		destroy_orphans_recursive(progress_data->window);
        g_free(progress_data); // Free the memory allocated for the structure
		pic_number = 0;
        return FALSE;
    }
    return TRUE;
}

/*
*   Description:
*       Function called every CAPTURE_WAIT_TIME seconds
*       - Updates the progress bar
*       - Communicates with the other 3 boards to capture images for the model
*       - rotates the motor after capturing the images is done
*   In: 
*       data:         A struct of type ProgressData, that includes pointer to the progress bar widget
*                     and a pointer to the window to be able to delete it after the scanning is done
*   Out:
*       gboolean variable which if set to true will be called again after CAPTURE_WAIT_TIME seconds
*       and if set to FALSE, the function will not be set again and will delete the parent window
*/
static gboolean scan_model(gpointer data) {
	Return_t ret = E_NOK;
    static uint8 phase = 1;
    // Cast the data pointer to a structure that contains both the progress bar and the window
    progress_t *progress_data = data;

    // Get the current percentage
    gdouble percentage = gtk_progress_bar_get_fraction(progress_data->progressbar);

    // Increase the percentage
    percentage += 1.0/(NUMBER_OF_PICTURES/NUMBER_OF_BOARDS);

    // Update the progress bar with the new percentage
    gtk_progress_bar_set_fraction(progress_data->progressbar, percentage);
    log_message("Percentage = %f \n", (float)percentage*100);

    //Motor movement
	//Init can be called only once in case the usb is not released at the end of motor_move_by_angle
	ret = motor_init();
	if (E_OK == ret)
	{
		ret = motor_move_by_angle((uint16)(360U/(NUMBER_OF_PICTURES/NUMBER_OF_BOARDS)));
	}
	
	/* This if condition is entered in case there is a porblem initalizing or controlling the servo motor*/
	if (E_NOK == ret)
	{
		status_message.title = "Problem occured";
		status_message.body = "Issue controlling servo motor";
		fsm_handle_event(EVENT_SCAN_ERROR);
        // Destroy the window when the progress reaches 100%
        destroy_orphans_recursive(progress_data->window);
        g_free(progress_data); // Free the memory allocated for the structure
        phase = 1;
		return FALSE;
	}

    // Capture the images from the slave boards
    ret = sock_capture_image(NUMBER_OF_BOARDS, phase++);
	if (E_NOK == ret)
	{
		log_message("Issue communicating with the cameras\n");
		status_message.title = "Problem occured";
		status_message.body = "Issue communicating with the cameras";
		fsm_handle_event(EVENT_SCAN_ERROR);
        // Destroy the window when the progress reaches 100%
        destroy_orphans_recursive(progress_data->window);
        g_free(progress_data); // Free the memory allocated for the structure
        phase = 1;
		return FALSE;
	}
    // If the percentage reaches or exceeds 100%, return FALSE to stop the timer
    if (percentage >= 1.0) {
		log_message("100 percent reached");
        fsm_handle_event(EVENT_PROCEED);
        // Destroy the window when the progress reaches 100%
        destroy_orphans_recursive(progress_data->window);
        g_free(progress_data); // Free the memory allocated for the structure
		phase = 1;
        return FALSE;
    }
    return TRUE;
}

// Function to display the current image
static void display_image(GtkWidget *image)
{
    // Load the current image file
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_paths[current_image_index], NULL);

    // Calculate the desired width and height for the resized image
    gint desired_width = 800;
    gint desired_height = 415; // Leave space for buttons

    // Scale the image to the desired size
    GdkPixbuf *resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf, desired_width, desired_height, GDK_INTERP_BILINEAR);

    // Update the image widget with the resized image
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), resized_pixbuf);

    // Clean up the pixbuf
    g_object_unref(pixbuf);
    g_object_unref(resized_pixbuf);
}

// Callback function for the "Next" button
static void next_button_clicked(GtkWidget *button, gpointer data)
{
    (void)button;
    GtkWidget *image = GTK_WIDGET(data);
    current_image_index = (current_image_index + 1) % ((NUMBER_OF_PICTURES/NUMBER_OF_BOARDS) * 3); // Increment index (wrap around if necessary)
    display_image(image);
}

// Callback function for the "Previous" button
static void previous_button_clicked(GtkWidget *button, gpointer data)
{
    (void)button;
    GtkWidget *image = GTK_WIDGET(data);
    current_image_index = (current_image_index - 1 + ((NUMBER_OF_PICTURES/NUMBER_OF_BOARDS) * 3)) % ((NUMBER_OF_PICTURES/NUMBER_OF_BOARDS) * 3); // Decrement index (wrap around if necessary)
    display_image(image);
}
/***********************Function definition***********************/

Return_t gui_init(void){
    log_message("GTK initialized!\n");
    gtk_init(NULL, NULL);
    return E_OK;
}

Return_t gui_run(void){
    log_message("GTK running!\n");
    gtk_main();
    return E_OK;
}

void gui_create_welcome_screen(void)
{
    // initialize global variable
    current_image_index = 0;
    entered_password[0] = '\0';
    entered_week[0] = '\0';
    status_message.title = '\0';
    status_message.body = '\0';
    // Initialize other modules
    backend_clear_info();
    log_message("GTK created welcome screen!\n");
    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Welcome!");
    gtk_window_fullscreen(GTK_WINDOW(window)); // Set fullscreen
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); // Remove window decorations
	set_color(window);
	
    // Disable window closing
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    // Create a label with welcome title
    GtkWidget *label_welcome = gtk_label_new("360FabScan");
    set_font_and_size(label_welcome, DEFAULT_FONT, DEFAULT_SIZE);

    // Load the image directly within the function
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("../../digifab2024.jpg", NULL);
    GdkPixbuf *resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 800, 375, GDK_INTERP_BILINEAR);
    GtkWidget *image = gtk_image_new_from_pixbuf(resized_pixbuf);

    // Create a button in the new window
    GtkWidget *button = gtk_button_new_with_label("Press here");
    set_font_and_size(button, DEFAULT_FONT, DEFAULT_SIZE);
    g_signal_connect(button, "clicked", G_CALLBACK(gui_proceed_button), window);
    set_color(button);
	
    // Create a grid container
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Add widgets to the grid
    gtk_grid_attach(GTK_GRID(grid), label_welcome, 1, 0, 3, 1); // Label welcome at row 0
    gtk_grid_attach(GTK_GRID(grid), image, 0, 1, 5, 5); // Label inst_1 at row 1
    gtk_grid_attach(GTK_GRID(grid), button, 2, 10, 1, 1); // Button at row 2
    
    // Show all widgets
    gtk_widget_show_all(window);
}



void gui_numpad_screen(State_t state)
{
    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Enter number");
    gtk_window_fullscreen(GTK_WINDOW(window)); // Set fullscreen
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); // Remove window decorations
    // Disable window closing
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	set_color(window);
	
    // Create a grid to organize widgets
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE); // Make columns homogeneous
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE); // Make rows homogeneous

	// Create a Proceed button in the new window
    GtkWidget *button_log_out = gtk_button_new_with_label("Log out");
    set_font_and_size(button_log_out, DEFAULT_FONT, DEFAULT_SIZE);
    g_signal_connect(button_log_out, "clicked", G_CALLBACK(gui_logout_button), window);
	gtk_grid_attach(GTK_GRID(grid), button_log_out, 2, 0, 1, 1);
	set_color(button_log_out);
	
    // Create a label widget to inform the user to enter their number
	GtkWidget *title;
	if (STATE_PASSWORD == state)
	{
		title= gtk_label_new("Enter your password");
		log_message("GTK created password numpad screen!\n");
	}
    else
	{
		title = gtk_label_new("Enter the week number");
		log_message("GTK created week number numpad screen!\n");
	}
    gtk_grid_attach(GTK_GRID(grid), title, 0, 1, 3, 1);
    set_font_and_size(title, DEFAULT_FONT, DEFAULT_SIZE);
	
    // Create a label to display the entered numbered
    numpad_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), numpad_label, 0, 2, 3, 1);
    set_font_and_size(numpad_label, DEFAULT_FONT, DEFAULT_SIZE);
    
    // Create number buttons
    char *numbers[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "Reset", "0", "Enter"}; // "C" for clear, "E" for enter
    for (int i = 0; i < 12; i++) {
        GtkWidget *button = gtk_button_new_with_label(numbers[i]);
		set_color(button);
        gtk_widget_set_size_request(button, 70, 70); // Set button size
        if ((i < 9) || (i==10))
		{
            g_signal_connect(button, "clicked", (STATE_PASSWORD == state)?G_CALLBACK(password_button_clicked):G_CALLBACK(week_button_clicked), NULL);
        }
		else if (i == 9) 
		{
            g_signal_connect(button, "clicked", (STATE_PASSWORD == state)?G_CALLBACK(password_clear_clicked):G_CALLBACK(week_clear_clicked), NULL);
        } else if (i == 11) 
		{
            g_signal_connect(button, "clicked", (STATE_PASSWORD == state)?G_CALLBACK(password_enter_clicked):G_CALLBACK(week_enter_clicked), window);
        }
        gtk_grid_attach(GTK_GRID(grid), button, i % 3, i / 3 + 3, 1, 1);
        set_font_and_size(button, DEFAULT_FONT, DEFAULT_SIZE);
    }

    // Show all widgets
    gtk_widget_show_all(window);
}

void gui_create_instructions_screen(void)
{
    char welcome_text[50];  /* variable to be used to welcome the team */
    char device_info[400];  /* variable to be used to print device info */
    log_message("GTK created instructions screen!\n");
    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Press the button to scan your model");
    gtk_window_fullscreen(GTK_WINDOW(window)); // Set fullscreen
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); // Remove window decorations
	set_color(window);
	
    // Disable window closing
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    // Create a label with welcome title
    snprintf(welcome_text, sizeof(welcome_text), "Welcome team %s", backend_get_team_number());
    GtkWidget *label_welcome = gtk_label_new(welcome_text);
    set_font_and_size(label_welcome, DEFAULT_FONT, DEFAULT_SIZE*2);

    // Create a label with instructions
    GtkWidget *label_inst_1 = gtk_label_new("- The device has 3 cameras inside the box placed on the same vertical \n  line at 3 different heights.");
    set_font_and_size(label_inst_1, DEFAULT_FONT, INSTRUCTIONS_SIZE);
    gtk_label_set_xalign(GTK_LABEL(label_inst_1), 0); // Align to the left

    // Create a label with instructions
    snprintf(device_info, sizeof(device_info), "- The object will rotate, 3 images at the 3 different heights will be taken \n  each %d degrees, so in total %d images will be captured.", 360/(NUMBER_OF_PICTURES/NUMBER_OF_BOARDS), NUMBER_OF_PICTURES);
    GtkWidget *label_inst_2 = gtk_label_new(device_info);
    set_font_and_size(label_inst_2, DEFAULT_FONT, INSTRUCTIONS_SIZE);
    gtk_label_set_xalign(GTK_LABEL(label_inst_2), 0); // Align to the left

    // Create a label with instructions
    GtkWidget *label_inst_3 = gtk_label_new("- You will review the images after the scanning is done to confirm \n  uploading to the server.");
    set_font_and_size(label_inst_3, DEFAULT_FONT, INSTRUCTIONS_SIZE);
    gtk_label_set_xalign(GTK_LABEL(label_inst_3), 0); // Align to the left

	// Create a label with instructions
    GtkWidget *label_inst_4 = gtk_label_new("- If you are not satisified with the pictures, log out and try again.\n");
    set_font_and_size(label_inst_4, DEFAULT_FONT, INSTRUCTIONS_SIZE);
    gtk_label_set_xalign(GTK_LABEL(label_inst_4), 0); // Align to the left
	
    // Create a label with instructions
    GtkWidget *label_inst_5 = gtk_label_new("Step (1) Place your model inside the device\nStep (2) Make sure the inner lights are working.");
    set_font_and_size(label_inst_5, DEFAULT_FONT, INSTRUCTIONS_SIZE);
    gtk_label_set_xalign(GTK_LABEL(label_inst_5), 0); // Align to the left

    // Create a label with instructions
    GtkWidget *label_inst_6 = gtk_label_new("Step (3) Close the door and press the button below.");
    set_font_and_size(label_inst_6, DEFAULT_FONT, INSTRUCTIONS_SIZE);
    gtk_label_set_xalign(GTK_LABEL(label_inst_6), 0); // Align to the left

    // Create a Proceed button in the new window
    GtkWidget *button_proceed = gtk_button_new_with_label("Start scan");
    set_font_and_size(button_proceed, DEFAULT_FONT, DEFAULT_SIZE);
    g_signal_connect(button_proceed, "clicked", G_CALLBACK(gui_proceed_button), window);
	set_color(button_proceed);
	
	// Create a log out button in the new window
    GtkWidget *button_logout = gtk_button_new_with_label("Log out");
    set_font_and_size(button_logout, DEFAULT_FONT, DEFAULT_SIZE);
    g_signal_connect(button_logout, "clicked", G_CALLBACK(gui_logout_button), window);
	set_color(button_logout);
	
    // Create a vertical box to hold the label and buttons
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(box), label_welcome);
    gtk_container_add(GTK_CONTAINER(box), label_inst_1);
    gtk_container_add(GTK_CONTAINER(box), label_inst_2);
    gtk_container_add(GTK_CONTAINER(box), label_inst_3);
    gtk_container_add(GTK_CONTAINER(box), label_inst_4);
    gtk_container_add(GTK_CONTAINER(box), label_inst_5);
	gtk_container_add(GTK_CONTAINER(box), label_inst_6);
    gtk_container_add(GTK_CONTAINER(box), button_proceed);
	gtk_container_add(GTK_CONTAINER(box), button_logout);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Show all widgets
    gtk_widget_show_all(window);
}

// Function to create and display the window
void gui_display_image(void)
{
	if ((char **)NULL == image_paths)
	{
		image_paths = (char **)malloc((NUMBER_OF_PICTURES/NUMBER_OF_BOARDS) * 3 * sizeof(char *));
		// Generate file names based on the number of pictures
		for (uint8 i = 0; i < (NUMBER_OF_PICTURES/3U); i++) {
			for (uint8 j = 0; j < NUMBER_OF_BOARDS; j++) {
				// Calculate the index
				uint8 index = i * NUMBER_OF_BOARDS + j;
	
				// Allocate memory for the file name
				image_paths[index] = (char *)malloc(10 * sizeof(char)); // Assuming file names won't exceed 10 characters
				
				// Construct the file name
				sprintf(image_paths[index], "%d_%d.jpg", i, j);
			}
		}
	}

    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Image Viewer");
    gtk_window_fullscreen(GTK_WINDOW(window)); // Set fullscreen
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); // Remove window decorations
	set_color(window);
	
	log_message("GTK created window for displaying captured images\n");
    // Disable window closing
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    // Create a GtkImage widget to display the image
    GtkWidget *image = gtk_image_new();
    display_image(image); // Display the initial image

    // Create "Next" button
    GtkWidget *button_next = gtk_button_new_with_label("Next");
    g_signal_connect(button_next, "clicked", G_CALLBACK(next_button_clicked), image);
	set_color(button_next);
	
    // Create "Previous" button
    GtkWidget *button_previous = gtk_button_new_with_label("Previous");
    g_signal_connect(button_previous, "clicked", G_CALLBACK(previous_button_clicked), image);
	set_color(button_previous);
	
    // Create "Log out" button
    GtkWidget *button_logout = gtk_button_new_with_label("Log out");
    g_signal_connect(button_logout, "clicked", G_CALLBACK(gui_logout_button), window);
	set_color(button_logout);
	
	// Create "Confirm" button
    GtkWidget *button_confirm = gtk_button_new_with_label("Confirm pictures");
    g_signal_connect(button_confirm, "clicked", G_CALLBACK(gui_proceed_button), window);
	set_color(button_confirm);
	
    // Create a horizontal box for the buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(button_box), button_previous, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), button_next, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(button_box), button_logout, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(button_box), button_confirm, TRUE, TRUE, 0);

    // Create a vertical box for the image and buttons
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    // Add the vbox to the window
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Show all widgets
    gtk_widget_show_all(window);
}

void gui_scan_model(void){
    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Model scanning");
    gtk_window_fullscreen(GTK_WINDOW(window)); // Set fullscreen
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); // Remove window decorations
	set_color(window);
	
	log_message("GTK created window for showing progress bar while scanning the model\n");
	
    // Disable window closing
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    // Create a label with instructions
    GtkWidget *label_title = gtk_label_new("Model scanning in progress ...\n");
    set_font_and_size(label_title, DEFAULT_FONT, DEFAULT_SIZE);

    // Create a label with instructions
    GtkWidget *label_inst_1 = gtk_label_new("Wait until the progress bar reaches 100%\n");
    set_font_and_size(label_inst_1, DEFAULT_FONT, DEFAULT_SIZE);
    gtk_label_set_xalign(GTK_LABEL(label_inst_1), 0); // Align to the left


    // Create a progress bar
    GtkWidget *progressbar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progressbar), TRUE); // Show text percentage
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), 0.0); // Set initial percentage to 0%

    // Add the progress bar to the window
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(box), label_title);
    gtk_container_add(GTK_CONTAINER(box), label_inst_1);
    gtk_container_add(GTK_CONTAINER(box), progressbar);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Show all widgets
    gtk_widget_show_all(window);

    // Create a structure to hold both the progress bar and the window
    progress_t *data = g_new(progress_t, 1);
    data->progressbar = GTK_PROGRESS_BAR(progressbar);
    data->window = window;

    // Make the first call to capture images, and the following call will be in scan_model()
    gtk_progress_bar_set_fraction(data->progressbar, (gdouble)(1.0/(NUMBER_OF_PICTURES/3)));
    (void)sock_capture_image(NUMBER_OF_BOARDS, 0U);
    // Create a timer to update the percentage every CAPTURE_WAIT_TIME seconds
    g_timeout_add_seconds(CAPTURE_WAIT_TIME, scan_model, data);
}

void gui_display_loading_screen(void){
    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Model scanning");
    gtk_window_fullscreen(GTK_WINDOW(window)); // Set fullscreen
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); // Remove window decorations
	set_color(window);
	
	log_message("GTK created window for showing progress bar while uploading the images\n");
	
    // Disable window closing
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    // Create a label with instructions
    GtkWidget *label_title = gtk_label_new("Uploading images in progress ...\n");
    set_font_and_size(label_title, DEFAULT_FONT, DEFAULT_SIZE);

    // Create a label with instructions
    GtkWidget *label_inst_1 = gtk_label_new("Wait until the progress bar reaches 100%\n");
    set_font_and_size(label_inst_1, DEFAULT_FONT, DEFAULT_SIZE);
    gtk_label_set_xalign(GTK_LABEL(label_inst_1), 0); // Align to the left


    // Create a progress bar
    GtkWidget *progressbar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progressbar), TRUE); // Show text percentage
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), 0.0); // Set initial percentage to 0%

    // Add the progress bar to the window
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(box), label_title);
    gtk_container_add(GTK_CONTAINER(box), label_inst_1);
    gtk_container_add(GTK_CONTAINER(box), progressbar);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Show all widgets
    gtk_widget_show_all(window);

    // Create a structure to hold both the progress bar and the window
    progress_t *data = g_new(progress_t, 1);
    data->progressbar = GTK_PROGRESS_BAR(progressbar);
    data->window = window;

    g_timeout_add_seconds(UPLOAD_WAIT_TIME, upload_images, data);
}

void gui_create_status_screen(void)
{
    log_message("GTK created screen to print status\n");
    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), status_message.title);
    gtk_window_fullscreen(GTK_WINDOW(window)); // Set fullscreen
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); // Remove window decorations
	set_color(window);
	
    // Disable window closing
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    // Create a label with welcome title
    GtkWidget *label_welcome = gtk_label_new(status_message.title);
    set_font_and_size(label_welcome, DEFAULT_FONT, DEFAULT_SIZE);

    // Create a label with instructions
    GtkWidget *label_inst_1 = gtk_label_new(status_message.body);
    set_font_and_size(label_inst_1, DEFAULT_FONT, STATUS_BODY_SIZE);

    // Create a button in the new window
    GtkWidget *button = gtk_button_new_with_label("Press here");
    set_font_and_size(button, DEFAULT_FONT, DEFAULT_SIZE);
    g_signal_connect(button, "clicked", G_CALLBACK(gui_logout_button), window);
    set_color(button);
	
    // Create a grid container
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE); // Allow the button row to expand vertically
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE); // Make columns homogeneous
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Add widgets to the grid
    gtk_grid_attach(GTK_GRID(grid), label_welcome, 0, 0, 3, 1); // Label welcome at row 0
    gtk_grid_attach(GTK_GRID(grid), label_inst_1, 0, 1, 3, 1); // Label inst_1 at row 1
    gtk_grid_attach(GTK_GRID(grid), button, 1, 4, 1, 1); // Button at row 2

    // Show all widgets
    gtk_widget_show_all(window);
}