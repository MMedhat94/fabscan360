#ifndef FSM_TYPES_H
#define FSM_TYPES_H

// Define states
typedef enum {
    E_OK,
    E_NOK,
    E_BACKEND_CONNECTION_ISSUE,
    E_BACKEND_WRONG_PASSWORD
} Return_t;


typedef unsigned char	    uint8;
typedef unsigned short int  uint16;
typedef unsigned int	    uint32;
typedef unsigned long long	uint64;
#endif /* FSM_TYPES_H */