#ifndef __COMMON_H
#define __COMMON_H

#include <unistd.h>


bool recv_exactly(int fd, char *buffer, int size);

bool sendSocket(int sock, char *buffer);

#endif  // __COMMON_H
