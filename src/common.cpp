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
	int recv_bytes, i = 0;
	while (i < size && (recv_bytes = recv(fd, buffer, size - i, 0)) > 0)
	{
		i += recv_bytes;
	}
	if (recv_bytes < 0)
	{
		cout << endl
			 << "Buffer: " << endl
			 << buffer << "(" << size << ")" << endl;
		cerr << "recv_exactly(): " << strerror(errno)
			 << " " << errno << " " << strerrorname_np(errno);
	}
	return recv_bytes > 0;
}

/**
 * Send the size of the string and then the string through a socket
 */
bool sendSocket(int sock, char *buffer)
{
	int length = strlen(buffer) + 1;
	/* length = htonl(length);
	if ((send(sock, &length, sizeof(4), 0)) < 0)
	{
		cerr << "Message length was not sent" << endl;
		return false;
	}
	length = ntohl(length); */
	if ((send(sock, buffer, 128, 0)) < 0)
	{
		cerr << "Message was not sent" << endl;
		return false;
	}
	// cout << "The message was successfully sent: " << buffer << "(" << length << ")" << endl;
	return true;
}