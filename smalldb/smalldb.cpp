#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <err.h> // err
#include <iostream>

#include "common.hpp"
#include "errorcodes.hpp"

using namespace std;
// TODO: Mieux diviser le code en helpers function pour réduire taille du main
// TODO: Traitement d'erreur

int main(void)
{
	// Gestion de signal: PIPE: Établissement de la connexion
	signal(SIGPIPE, SIG_IGN);
	//** SOCKET
	// Création du socket
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	// Paramétrage du socket
	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(28772);
	// Mise à l'écoute
	bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	listen(server_fd, 5);
	// Acceptation
	size_t addrlen = sizeof(address);
	int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen); // Appel bloquant
	if (new_socket == DISCONNECTED)
	{
		err(NETWORK_ERROR, "Unnable to established connexion with client.\n");
	}
	warnx("Client %s connecté.", inet_ntoa(address.sin_addr));

	//*Création du thread:
	// TODO

	//*Traitement de la requête
	char buffer[1024];
	uint32_t length;
	while(read_exact(new_socket, (char*)&length, 4) && read_exact(new_socket, buffer, ntohl(length)))
	{
		cout << "(server): Message reçu: " << buffer << endl;
		write(new_socket, &buffer, sizeof(buffer));
	}

	close(server_fd);
	close(new_socket);
	return 0;
}
