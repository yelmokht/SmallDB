#include <arpa/inet.h>
#include <deque>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

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

bool recv_message(int fd, char *buffer)
{
	int recv_bytes = 0;
	if ((recv_bytes = recv(fd, buffer, BUFFER_SIZE, 0)) < 0)
	{
		cerr << "Error: recv_message(): " << strerrorname_np(errno) << endl;
	}
	return recv_bytes > 0;
}

bool send_message(int sock, const char *buffer)
{
	int sent_bytes = 0;
	if ((sent_bytes = send(sock, buffer, BUFFER_SIZE, 0)) < 0)
		cerr << "Error: send_message(): " << strerrorname_np(errno) << endl;
	return sent_bytes > 0;
}

bool send_protocol(client_t *client, vector<string> &message_list)
{
	int i = 0;
	char buffer[BUFFER_SIZE];
	while (i < message_list.size())
	{
		if (client->status == CLIENT_CONNECTED)
		{ // Envoi d'un message au client
			send_message(client->sock, message_list[i].c_str());
			// Reçois la confirmation de réception de la part du client
			if (recv_message(client->sock, buffer) > 0)
			{
				if (strcmp(buffer, DISCONNECT_MESSAGE) == 0)
				{ // Si le client se déconnecte lors de l'envoi de message
					client->status = CLIENT_DISCONNECTED;
					return false;
				}
			}
			else
			{ // Erreur dans le socket => Perte de connection
				client->status = CLIENT_LOST_CONNECTION;
				return false;
			}
		}
		else
		{
			cerr << "Error: send_protocol(): Client disconnected" << endl;
		}
		i++;
	}
	return true;
}