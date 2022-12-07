#include "common.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

#include <iostream>


using namespace std;
/**
 * Reads the exactly number of bytes (size)
 */
bool recv_exactly(int fd, char *buffer, int size)
{
	int lu, i = 0;
	while (i < size && (lu = recv(fd, buffer, size - i, 0)) > 0)
	{
		i += lu;
	}

	if (lu < 0)
	{
		cerr << "read()";
	}
	return lu > 0;
}
/**
 * Send the size of the strign and then the string through a socket
*/
void sendSocket(int sock, char *buffer)
{
   //TODO: Gestion d'erreur
   uint32_t length = strlen(buffer);
   buffer[length - 1] = '\0';
   length = htonl(length);
   send(sock, &length, sizeof(length), 0);
   length = ntohl(length);
   send(sock, buffer, length, 0);
}