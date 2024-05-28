#ifndef SOCK_H
#define SOCK_H  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int sock_init(void);

extern int sock_connect(void);

extern int sock_send_image(void);

extern int sock_close(void);


#endif  /* SOCK_H */