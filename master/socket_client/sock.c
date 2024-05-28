#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9000
#define IMAGE_FILE_PREFIX "received_image"

int main() {
    int server_socket, client_socket1, client_socket2;
    struct sockaddr_in server_addr, client_addr1, client_addr2;
    socklen_t client_addr_len = sizeof(client_addr1);

    // Server configuration
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 2) < 0) { // Listen for two clients
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept first client connection
    client_socket1 = accept(server_socket, (struct sockaddr*)&client_addr1, &client_addr_len);
    if (client_socket1 < 0) {
        perror("Acceptance failed for client 1");
        exit(EXIT_FAILURE);
    }

    printf("Accepted connection from %s:%d (Client 1)\n", inet_ntoa(client_addr1.sin_addr), ntohs(client_addr1.sin_port));

    // Receive the image data from client 1
    FILE* image_file1 = fopen("received_image1.jpg", "wb");
    if (image_file1 == NULL) {
        perror("Error opening file for client 1");
        exit(EXIT_FAILURE);
    }

    char buffer1[1024];
    size_t bytes_received1;
    while ((bytes_received1 = recv(client_socket1, buffer1, sizeof(buffer1), 0)) > 0) {
        fwrite(buffer1, 1, bytes_received1, image_file1);
    }

    fclose(image_file1);
    printf("Image received and saved as received_image1.jpg (Client 1)\n");

    // Accept second client connection
    client_socket2 = accept(server_socket, (struct sockaddr*)&client_addr2, &client_addr_len);
    if (client_socket2 < 0) {
        perror("Acceptance failed for client 2");
        exit(EXIT_FAILURE);
    }

    printf("Accepted connection from %s:%d (Client 2)\n", inet_ntoa(client_addr2.sin_addr), ntohs(client_addr2.sin_port));

    // Receive the image data from client 2
    FILE* image_file2 = fopen("received_image2.jpg", "wb");
    if (image_file2 == NULL) {
        perror("Error opening file for client 2");
        exit(EXIT_FAILURE);
    }

    char buffer2[1024];
    size_t bytes_received2;
    while ((bytes_received2 = recv(client_socket2, buffer2, sizeof(buffer2), 0)) > 0) {
        fwrite(buffer2, 1, bytes_received2, image_file2);
    }

    fclose(image_file2);
    printf("Image received and saved as received_image2.jpg (Client 2)\n");

    // Close the client and server sockets
    close(client_socket1);
    close(client_socket2);
    close(server_socket);

    return 0;
}
