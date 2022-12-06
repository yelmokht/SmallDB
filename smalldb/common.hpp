#ifndef __COMMON_H
#define __COMMON_H

#include <unistd.h>


bool read_exact(int fd, void *buffer, int nbytes);

#endif  // __COMMON_H
