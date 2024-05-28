#include <stdio.h>
#include <time.h>
#include "logging.h"

static FILE *log_file = NULL; // Global variable to hold the log file pointer

void init_logging() {
    // Get the current time
    time_t raw_time;
    struct tm *time_info;
    char file_name[50]; // Buffer to hold the file name

    time(&raw_time);
    time_info = localtime(&raw_time);

    // Create the file name based on the current date
    strftime(file_name, sizeof(file_name), "./logging/logs/%Y-%m-%d.log", time_info);

    // Open the log file in append mode
    log_file = fopen(file_name, "a");
    if (log_file == NULL) {
        printf("Error opening log file.\n");
        // Handle error accordingly
    }
}

void close_logging() {
    // Close the log file if it's open
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

#include <stdarg.h> // Add this line for va_start, va_end, and va_list

void log_message(const char *format, ...) {
    // Get the current time
    time_t raw_time;
    struct tm *time_info;
    char time_str[20]; // Buffer to hold the formatted time

    time(&raw_time);
    time_info = localtime(&raw_time);

    // Format the current time as HH:MM:SS
    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);

    // Write the log message with timestamp to the terminal
    va_list args;
    va_start(args, format);
    printf("[%s] ", time_str);
    vprintf(format, args);
    printf("\n");
    va_end(args);

    // Check if the log file is open
    if (log_file != NULL) {
        // Write the log message with timestamp to the file
        va_list file_args;
        va_start(file_args, format);
        fprintf(log_file, "[%s] ", time_str);
        vfprintf(log_file, format, file_args);
        fprintf(log_file, "\n");
        va_end(file_args);

        fflush(log_file); // Flush the buffer to ensure the message is written immediately
    } else {
        fprintf(stderr, "Log file is not open.\n");
        // Handle error accordingly
    }
}


