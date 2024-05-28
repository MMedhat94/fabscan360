#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../FSM_types.h"
#include "logging/logging.h"
/********************  Macro definitions  ********************/
#define PORT 9000
/********************  Funciton definitions  ********************/
Return_t sock_capture_image(uint8 num_boards, uint8 phase) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    uint8 i = 0;
    char filename[10];

    // Server configuration
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        log_message("Socket creation failed");
		return E_NOK;
    }

    // Set SO_REUSEADDR option to allow reusing the same address and port combination
    int reuse_addr = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) {
        log_message("Setting SO_REUSEADDR failed");
        close(server_socket);
        return E_NOK;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        log_message("Binding failed");
		return E_NOK;
    }

    if (listen(server_socket, num_boards) < 0) { // Listen for slave boards
        log_message("Listening failed");
		return E_NOK;
    }
    log_message("Server listening on port %d\n", PORT);

    for(i=0;i<num_boards;i++)
    {
        // Accept first client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            log_message("Acceptance failed for client");
            return E_NOK;
        }

        log_message("Accepted connection from %s:%d (Client %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), i);

        // Receive the image data from client 1
        snprintf(filename, sizeof(filename), "%d_%d.jpg", phase, i);
        FILE* image_file1 = fopen(filename, "wb");
        if (image_file1 == NULL) {
            log_message("Error opening file for client");
            return E_NOK;
        }

        char buffer1[1024];
        size_t bytes_received1;
        while ((bytes_received1 = recv(client_socket, buffer1, sizeof(buffer1), 0)) > 0) {
            fwrite(buffer1, 1, bytes_received1, image_file1);
        }

        fclose(image_file1);
        log_message("Image received and saved as %d_%d.jpg (Client %d)\n", phase, i, i);

        //Close the client and server sockets
        close(client_socket);
    }

    close(server_socket);
    return E_OK;
}
