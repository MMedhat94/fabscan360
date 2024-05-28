#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    // Client configuration
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9000); // Replace with the server's port number
    server_addr.sin_addr.s_addr = inet_addr("192.168.51.10"); // Replace with the server's IP address

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Read and send the image data
    FILE* image_file = fopen("captured_image.jpg", "rb"); // Open the image file to send
    if (image_file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), image_file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(image_file);
    printf("Image sent to the server\n");

    // Close the client socket
    close(client_socket);

    return 0;
}
