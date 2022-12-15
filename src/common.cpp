#include "common.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

#include <iostream>

using namespace std;

int _checked(int ret, std::string calling_function)
{
	if (ret < 0)
	{
		perror(calling_function.c_str());
		exit(EXIT_FAILURE);
	}
	return ret;
}

/**
 *
 * @brief Reçoit un message de BUFFER_SIZE bytes
 *
 * @param fd Socket émetteur de message
 * @param buffer Buffer pour stocker message de taille BUFFER_SIZE
 * @return true Message reçu
 * @return false Connexion perdue ou error
 */
bool recv_message(int fd, char *buffer)
{
	int recv_bytes;
	if ((recv_bytes = recv(fd, buffer, BUFFER_SIZE, 0)) < 0)
	{
		cerr << "Error: recv_message(): " << strerrorname_np(errno) << endl;
	}
	return recv_bytes > 0;
}

/**
 * @brief Envois un message de BUFFER_SIZE bytes
 *
 * @param sock Socket récepteur du message
 * @param buffer Message à envoyer de taille BUFFER_SIZE
 * @return true: Message envoyé
 * @return false: Connexion perdue ou error
 */
bool send_message(int sock, char *buffer)
{
	int sent_bytes;
	if ((sent_bytes = send(sock, buffer, BUFFER_SIZE, 0)) < 0)
		cerr << "Error: send_message(): " << strerrorname_np(errno) << endl;
	return sent_bytes > 0;
}