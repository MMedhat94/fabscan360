#include "backend.h"
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "logging/logging.h"
/********************  Static Macro definition  ********************/
//Defines the maximum week number to be accepted
#define MAX_WEEK_NUMBER	12
/********************  Static variables definition  ********************/
static backend_password_response_t  backend_password_response;

static backend_send_pic_request_t backend_send_pic_request;

static backend_send_pic_response_t backend_send_pic_response;

/******************** type definitions  ********************/
struct MemoryStruct {
    char *memory;
    size_t size;
};  /* A struct to hold response data and its size */
/********************  Static funciton definitions  ********************/
/* This function will be called by libcurl to handle the received data */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    /* Allocate memory for the received data */
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        /* Out of memory! */
        log_message("Failed to allocate memory\n");
        return 0;
    }

    /* Copy the received data into the allocated memory */
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; /* Null-terminate the string */

    return realsize;
}

//sends the data in backend_send_pic_request to the fablab backend
static Return_t backend_upload_fablab(void)
{
    /* Define variables */
    Return_t ret = E_NOK;
    CURL *curl = NULL;
    CURLcode res = 0;
    struct MemoryStruct chunk = {NULL,0};
    struct curl_slist *headers = NULL;
    cJSON *response_json = NULL;
	const cJSON *json_url = NULL;
    char request_data[300];  /* variable to be used to send the request */
	char url_data[300];  /* variable to be used to form the URL */

    /* Insert the user password to the CURL request*/
    snprintf(request_data, sizeof(request_data), "{\"weekNo\":\"%s\",\"imageURL\":\"%s\"}", backend_send_pic_request.week_number, backend_send_pic_request.secure_url);
    
    /* Initialize libcurl */
    curl_global_init(CURL_GLOBAL_ALL);

    /* Initialize a curl session */
    curl = curl_easy_init();

    if(curl) {
		snprintf(url_data, sizeof(url_data), "https://www.designedu.org/api/ras/%s", backend_password_response.team_number);
		log_message("Image sent to URL: %s\n", url_data);
        /* Set the URL to send the request to */
        curl_easy_setopt(curl, CURLOPT_URL, url_data);

        /* Set the Content-Type header to indicate JSON data */
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* Set the request method to POST */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* Set the POST data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data);
		log_message("Data sent to the backend is: %s\n", request_data);
        /* Initialize the memory struct */
        chunk.memory = malloc(1); /* Start with an empty string */
        chunk.size = 0;

        /* Set the callback function to handle the received data */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* Perform the request */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if(res != CURLE_OK)
            log_message("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        else
            log_message("Response: %s\n", chunk.memory); /* Print the received data */

        /* Parse the response JSON */
        response_json = cJSON_Parse(chunk.memory);

        /* Cleanup */
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory); /* Free the allocated memory */

        /* Check if response_json is NULL */
        if (response_json == NULL)
        {
            ret = E_BACKEND_CONNECTION_ISSUE;
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL)
            {
                log_message("Error before: %s\n", error_ptr);
            }
        }
        else
        {
            ret = E_OK;
        }

        if ( E_OK == ret)
        {
            /* Get the URL from the response */
            json_url = cJSON_GetObjectItemCaseSensitive(response_json, "message");
            if (cJSON_IsString(json_url) && (json_url->valuestring != NULL))
            {
                strncpy(backend_send_pic_response.send_pic_result, json_url->valuestring, sizeof(backend_send_pic_response.send_pic_result));
                log_message("Fablab backend response is  \"%s\"\n", backend_send_pic_response.send_pic_result);
                ret = E_OK;
            }
            else
            {
                /* If the connection was made but the parameter message was not found then a problem happened in the connection*/
                ret = E_BACKEND_CONNECTION_ISSUE;
				log_message("Fablab backend didn't respond properly to image upload\n");
            }
        }

        /* Cleanup global state */
        curl_global_cleanup();
    }
    else
    {
        ret = E_BACKEND_CONNECTION_ISSUE;
    }
    return ret;
}

// Callback function to receive debug information
//static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) {
//    // Print debug information to stdout
//    fwrite(data, 1, size, stdout);
//    return 0;
//}
/********************  Funciton definitions  ********************/

void backend_clear_info(void)
{
    memset(backend_password_response.cloudinary.url, '\0', sizeof(backend_password_response.cloudinary.url));
    memset(backend_password_response.cloudinary.api_key, '\0', sizeof(backend_password_response.cloudinary.api_key));
    memset(backend_password_response.cloudinary.preset, '\0', sizeof(backend_password_response.cloudinary.preset));
    memset(backend_password_response.team_number, '\0', sizeof(backend_password_response.team_number));
    memset(backend_send_pic_request.secure_url, '\0', sizeof(backend_send_pic_request.secure_url));
    memset(backend_send_pic_request.week_number, '\0', sizeof(backend_send_pic_request.week_number));
    memset(backend_send_pic_response.send_pic_result, '\0', sizeof(backend_send_pic_response.send_pic_result));
}

Return_t backend_password_check(char* password)
{
    /* Define variables */
    Return_t ret = E_NOK;
    CURL *curl = NULL;
    CURLcode res = 0;
    struct MemoryStruct chunk = {NULL,0};
    struct curl_slist *headers = NULL;
    cJSON *response_json = NULL;
    const cJSON *json_url = NULL;
    const cJSON *json_api_key = NULL;
    const cJSON *json_preset = NULL;
    const cJSON *json_team_number = NULL;
    char data[50];  /* variable to be used to send the password */

    /* Insert the user password to the CURL request*/
    snprintf(data, sizeof(data), "{\"groupToken\": \"%s\"}", password);
    
    /* Initialize libcurl */
    curl_global_init(CURL_GLOBAL_ALL);

    /* Initialize a curl session */
    curl = curl_easy_init();

    if(curl) {
        /* Set the URL to send the request to */
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.designedu.org/api/ras");

        /* Set the Content-Type header to indicate JSON data */
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* Set the request method to POST */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* Set the POST data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        /* Initialize the memory struct */
        chunk.memory = malloc(1); /* Start with an empty string */
        chunk.size = 0;

        /* Set the callback function to handle the received data */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* Perform the request */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if(res != CURLE_OK)
            log_message("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        else
            log_message("Response: %s\n", chunk.memory); /* Print the received data */

        /* Parse the response JSON */
        response_json = cJSON_Parse(chunk.memory);

        /* Cleanup */
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory); /* Free the allocated memory */

        /* Check if response_json is NULL */
        if (response_json == NULL)
        {
            ret = E_BACKEND_CONNECTION_ISSUE;
            const char *error_ptr = cJSON_GetErrorPtr();
			log_message("Response is Empty \n");
            if (error_ptr != NULL)
            {
                log_message("Error before: %s\n", error_ptr);
            }
        }
        else
        {
            ret = E_OK;
        }

        if ( E_OK == ret)
        {
            /* Get the URL from the response */
            json_url = cJSON_GetObjectItemCaseSensitive(response_json, "cloudinary_api_url");
            if (cJSON_IsString(json_url) && (json_url->valuestring != NULL))
            {
                strncpy(backend_password_response.cloudinary.url, json_url->valuestring, sizeof(backend_password_response.cloudinary.url));
                log_message("Cloudinary API URL is  \"%s\"\n", backend_password_response.cloudinary.url);
                ret = E_OK;
            }
            else
            {
                /* If the connection was made but the parameter cloudinary_api_url was not found then the wrong password was entered*/
                ret = E_BACKEND_WRONG_PASSWORD;
				log_message("Cloudinary API URL is not found in the response from Fablab\n");
            }
        }
        
        if ( E_OK == ret)
        {
            /* Get the API key from the response */
            json_api_key = cJSON_GetObjectItemCaseSensitive(response_json, "cloudinary_api_key");
            if (cJSON_IsString(json_api_key) && (json_api_key->valuestring != NULL))
            {
                strncpy(backend_password_response.cloudinary.api_key, json_api_key->valuestring, sizeof(backend_password_response.cloudinary.api_key));
                log_message("Cloudinary API KEY is  \"%s\"\n", backend_password_response.cloudinary.api_key);
                ret = E_OK;
            }
			else
			{
				log_message("Cloudinary API KEY is not found in the response from Fablab\n");
			}
        }

        if ( E_OK == ret)
        {
            /* Get the upload preset from the response */
            json_preset = cJSON_GetObjectItemCaseSensitive(response_json, "cloudinary_upload_preset");
            if (cJSON_IsString(json_preset) && (json_preset->valuestring != NULL))
            {
                strncpy(backend_password_response.cloudinary.preset, json_preset->valuestring, sizeof(backend_password_response.cloudinary.preset));
                log_message("Cloudinary upload preset is  \"%s\"\n", backend_password_response.cloudinary.preset);
                ret = E_OK;
            }
			else
			{
				log_message("Cloudinary Upload preset is not found in the response from Fablab\n");
			}
        }

        if ( E_OK == ret)
        {
            /* Get the API key from the response */
            json_team_number = cJSON_GetObjectItemCaseSensitive(response_json, "group_id");
            if (cJSON_IsString(json_team_number) && (json_team_number->valuestring != NULL))
            {
                strncpy(backend_password_response.team_number, json_team_number->valuestring, sizeof(backend_password_response.team_number));
                log_message("Team number is  \"%s\"\n", backend_password_response.team_number);
                ret = E_OK;
            }
			else
			{
				log_message("Team number is not found in the response from Fablab\n");
			}
        }

        /* Cleanup global state */
        curl_global_cleanup();
    }
    else
    {
        ret = E_BACKEND_CONNECTION_ISSUE;
    }
    return ret;
}

char* backend_get_team_number(void)
{
    return backend_password_response.team_number;
}

Return_t backend_set_week_number(char* week)
{
	if ( MAX_WEEK_NUMBER >= atoi(week))
	{
		memcpy(backend_send_pic_request.week_number, week, sizeof(backend_send_pic_request.week_number));
		return E_OK;
	}
	else
	{
		log_message("Week number entered is more than %d\n", MAX_WEEK_NUMBER);
		return E_NOK;
	}
}

Return_t backend_send_image(char* file)
{
	CURL *curl = NULL;
    CURLcode res = 0;
	struct MemoryStruct chunk = {NULL,0};
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";
	cJSON *response_json = NULL;
    const cJSON *json_url = NULL;
	Return_t ret = E_NOK;

    curl_global_init(CURL_GLOBAL_ALL);

    // Set up the POST form data
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_FILE, file,
                 CURLFORM_END);

    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "upload_preset",
                 CURLFORM_COPYCONTENTS, backend_password_response.cloudinary.preset,
                 CURLFORM_END);

    // Add API key to the form data
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "api_key",
                 CURLFORM_COPYCONTENTS, backend_password_response.cloudinary.api_key,
                 CURLFORM_END);

    // Initialize a curl session
    curl = curl_easy_init();

    if(curl) {
        // Set the URL that receives this POST
        curl_easy_setopt(curl, CURLOPT_URL, backend_password_response.cloudinary.url);

        // Add a custom header
        headerlist = curl_slist_append(headerlist, buf);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        // Specify the POST data
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		/* Initialize the memory struct */
        chunk.memory = malloc(1); /* Start with an empty string */
        chunk.size = 0;
		
		/* Set the callback function to handle the received data */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Set the debug callback function
        //curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Enable verbose mode

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK)
            log_message("curl_easy_perform() failed while uploading to cloudinary: %s\n", curl_easy_strerror(res));

		/* Parse the response JSON */
        response_json = cJSON_Parse(chunk.memory);
		
        // Always cleanup
        curl_easy_cleanup(curl);

        // Cleanup the formpost chain
        curl_formfree(formpost);

        // Cleanup the custom headers
        curl_slist_free_all(headerlist);
		
		free(chunk.memory); /* Free the allocated memory */
		
		/* Check if response_json is NULL */
        if (response_json == NULL)
        {
            ret = E_BACKEND_CONNECTION_ISSUE;
            const char *error_ptr = cJSON_GetErrorPtr();
			log_message("Response is Empty \n");
            if (error_ptr != NULL)
            {
                log_message("Error before: %s\n", error_ptr);
            }
        }
        else
        {
            ret = E_OK;
        }

        if ( E_OK == ret)
        {
            /* Get the URL from the response */
            json_url = cJSON_GetObjectItemCaseSensitive(response_json, "secure_url");
            if (cJSON_IsString(json_url) && (json_url->valuestring != NULL))
            {
                strncpy(backend_send_pic_request.secure_url, json_url->valuestring, sizeof(backend_send_pic_request.secure_url));
                log_message("SECURE URL is  \"%s\"\n", backend_send_pic_request.secure_url);
                ret = E_OK;
            }
            else
            {
                /* If the connection was made but the parameter secure_url was not found then a problem happened in the connection*/
                ret = E_BACKEND_CONNECTION_ISSUE;
            }
        }
		
    }
	else
	{
		ret = E_NOK;
		log_message("Failed to start CURL! \n");
	}
    curl_global_cleanup();
	if( E_OK == ret)
	{
		ret = backend_upload_fablab();
	}
    return ret;
}