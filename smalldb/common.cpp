#include "common.hpp"

#include <sys/types.h>
#include <sys/socket.h>
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