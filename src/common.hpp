#ifndef __COMMON_H
#define __COMMON_H

#include <unistd.h>
#include <string>

#include "./server/server.hpp"

// Indique que je me déconnecte
#define DISCONNECT_MESSAGE (char *)"-3"
// Indique que j'ai recu le message
#define RECEIVED_MESSAGE (char *)"-2"
//Indique la fin de toutes les messages
#define END_OF_MESSAGE (char *)"-1"
//Taille du buffer d'échange de message
#define BUFFER_SIZE 256

// The macro allows us to retrieve the name of the calling function
#define checked(call) _checked(call, #call)

int _checked(int ret, const std::string &calling_function);

bool recv_message(int fd, char *buffer);

bool send_message(int sock, const char *buffer);

bool send_protocol(client_t *client, std::vector<std::string> list);

#endif  // __COMMON_H
