#include "socket_server/sock.h"
#include <unistd.h>
#include <stdlib.h>
int main (void)
{
    __uint8_t connect_return = EXIT_FAILURE;
    (void)sock_init();
    
    printf("Waiting for server connection\n");
    do
    {
        connect_return = sock_connect();
        usleep(1000000);  // Sleep for 1000,000 microseconds (1 seconds)
    }while(EXIT_FAILURE == connect_return);

    system("sudo libcamera-jpeg -o captured_image.jpeg -t 2000");
    (void)sock_send_image();
    (void)sock_close();
    return 0;
}
