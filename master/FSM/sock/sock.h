#ifndef SOCK_H
#define SOCK_H

/*
*   Description:
*       Connects to a slave board and receive the captured image
*       Captured images will be saved in the following format <phase>_<board>.jpeg
*       where phase is incremented each time the API is called.
*       And <board> is 0...num_boards for each time the API is called
*   In: 
*       num_boards: number of boards to accept connections from
*       phase:      used in naming the received image files
*   Out:
*       Return_t type which indicates the result of connection
*/
extern Return_t sock_capture_image(uint8 num_boards, uint8 phase);
#endif //SOCK_H