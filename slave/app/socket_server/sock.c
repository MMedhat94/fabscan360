#include "sock.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define LOG_FILE "log_sock.txt"

static int client_socket;
static struct sockaddr_in server_addr;
static FILE *log_file;

static void log_error(const char *message) {
    // Open the log file in append mode
    log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        printf("Failed to open log file\n");
        exit(EXIT_FAILURE);
    }

    // Get the current date and time
    time_t rawtime;
    struct tm *info;
    char timestamp[80];

    time(&rawtime);
    info = localtime(&rawtime);

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", info);

    // Write the error message to the log file
    fprintf(log_file, "[%s] %s\n", timestamp, message);

    //show the error in the terminal too
    printf("%s\n", message);

    // Close the log file
    fclose(log_file);
}

int sock_init(void) {
    // Client configuration
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        log_error("Socket creation failed");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9000); // Replace with the server's port number
    server_addr.sin_addr.s_addr = inet_addr("192.168.51.10"); // Replace with the server's IP address

    log_error("Socket creation succeeded");
    return EXIT_SUCCESS;
}

int sock_connect(void)
{
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        //log_error("Connection failed");
        return EXIT_FAILURE;
    }
    log_error("Connection Succeeded");
    return EXIT_SUCCESS;
}

int sock_send_image(void)
{
    // Read and send the image data
    FILE* image_file = fopen("captured_image.jpeg", "rb"); // Open the image file to send
    if (image_file == NULL) {
        log_error("Error opening file");
        return EXIT_FAILURE;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), image_file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(image_file);
    log_error("Image sent to the server");
    return EXIT_SUCCESS;
}

int sock_close(void)
{
    // Close the client socket
    close(client_socket);
    log_error("Connection closed");

    return EXIT_SUCCESS;
}
