#ifndef __COMMON_H
#define __COMMON_H

#include <unistd.h>
#include <string>


bool recv_exactly(int fd, char *buffer, int size);

bool sendSocket(int sock, std::string buffer);

#endif  // __COMMON_H
