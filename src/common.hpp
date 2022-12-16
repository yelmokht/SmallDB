#ifndef __COMMON_H
#define __COMMON_H

#include <unistd.h>

#include <string>

//Indique la fin de toutes les messages
#define END_OF_MESSAGE "-1"
//Taille du buffer d'échange de message
#define BUFFER_SIZE 256

// The macro allows us to retrieve the name of the calling function
#define checked(call) _checked(call, #call)

int _checked(int ret, const std::string &calling_function);

bool recv_message(int fd, char *buffer);

bool send_message(int sock, char *buffer);

#endif  // __COMMON_H
