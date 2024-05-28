#ifndef BACKEND_H
#define BACKEND_H

#include "../FSM_types.h"

/********************  Macro definitions  ********************/
#define PASSWORD_LENGTH 6U
#define WEEK_LENGTH 2U
/********************  Type definitions  ********************/
typedef struct
{
    char url[100];         /* the url to be used for cloudinary to upload the images */
    char api_key[100];     /* the api_key to be used within the url */
    char preset[100];      /* the upload preset to be used */
}backend_cloudinary_param_t;

typedef struct
{
    char                       team_number[3]; /* team number received from the backend */
    backend_cloudinary_param_t  cloudinary;
}backend_password_response_t;

typedef struct
{
    char                       secure_url[200];   /* secure url received from cloudinary after uploading an image */
    char                       week_number[3];    /* week number recieved from the user */
}backend_send_pic_request_t;

typedef struct
{
    char                       send_pic_result[100];   /* result of sending the images to the backe end */
}backend_send_pic_response_t;
/********************  External functions  ********************/

/*
*   Description:
*       Check the validity of the password from the backend
*   In: 
*       void
*   Out:
*       E_OK: Init passed
*       E_NOK: Init failed
*/
extern Return_t backend_password_check(char*);

/*
*   Description:
*       Clears all the info related to the backend
*       Password, team number and cloudinary parameters
*   In: 
*       void
*   Out:
*       void
*/
extern void backend_clear_info(void);

/*
*   Description:
*       Returns pointer to a string containing the team number fetched from backend
*   In: 
*       void
*   Out:
*       pointer to string containing team number
*/
extern char* backend_get_team_number(void);

/*
*   Description:
*       checks if week number is valid or not and saves it
*   In: 
*       week: input week number
*   Out:
*       E_OK: week number is valid and has been saved passed
*       E_NOK: week number is invalid
*/
extern Return_t backend_set_week_number(char* week);

/*
*   Description:
*       Sends a file to the backend
*   In: 
*       pointer to the file name
*   Out:
*       E_OK: image upload successful
*       E_NOK: image upload failed
*/
Return_t backend_send_image(char* file);
#endif  /* BACKEND_H */