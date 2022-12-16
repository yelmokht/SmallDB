#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <deque>
#include <iostream>

#include "common.hpp"

using namespace std;

int _checked(int ret, const std::string &calling_function)
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
	int recv_bytes = 0;
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
bool send_message(int sock, const char *buffer)
{
	int sent_bytes = 0;
	if ((sent_bytes = send(sock, buffer, BUFFER_SIZE, 0)) < 0)
		cerr << "Error: send_message(): " << strerrorname_np(errno) << endl;
	return sent_bytes > 0;
}

bool send_protocol(client_t *client, vector<string> list) {
	int i = 0;
	char buffer[BUFFER_SIZE];
	while (client->status == CLIENT_CONNECTED && i < list.size()) {
		send_message(client->sock, list[i].c_str());
		if (recv_message(client->sock, buffer) > 0) {
			if (strcmp(buffer, DISCONNECT_MESSAGE) == 0) {
				client->status = CLIENT_DISCONNECTED;
				return false;
			}
		}
		else {
			client->status = CLIENT_LOST_CONNECTION;
			return false;
		}
		i++;
	}
	return true;
}