#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "../common.hpp"

using namespace std;

// Variable globale afin de permettre le gestionnaire de signal ait accès à cette donnée.
int sock; // Socket serveur

/**
 * @brief Gestionnaire de signal du client
 *
 * @param signum Code du signal
 */
void handler(int signum)
{
	switch (signum)
	{
	case SIGPIPE:
		cout << "Received SIGPIPE signal" << endl;
		cout << "Server connection lost..." << endl;
		cout << "Closing client..." << endl;
		close(sock);
		exit(1);
		break;
	case SIGINT:
		cout << "Received SIGINT signal!" << endl;
		cout << "Closing client..." << endl;
		send_message(sock, DISCONNECT_MESSAGE);
		close(sock);
		exit(0);
		break;
	default:
		break;
	}
}
/**
 * @brief Configure le gestionnaire de signal du client
 *
 */
void client_configure_signal_handler()
{
	signal(SIGPIPE, handler);
	signal(SIGINT, handler);
}
/**
 * @brief Créer et configure le socket servant à ce connecter avec le serveur
 *
 * @param serv_addr Structure d'adressage
 * @param serv_ip IP du serveur
 */
void client_configure_socket(struct sockaddr_in &serv_addr, const char *serv_ip)
{
	sock = checked(socket(AF_INET, SOCK_STREAM, 0));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(28772);
	// Conversion de string vers IPv4 ou IPv6 en binaire
	inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr);
}
/**
 * @brief Connecte le client au serveur
 *
 * @param serv_addr Structure d'adressage
 */
void client_connect_server(struct sockaddr_in &serv_addr)
{
	while ((connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
	{
		cout << "Trying connection with server..." << endl;
		sleep(1);
	}
	cout << "Connexion established!" << endl;
}
/**
 * @brief Envoi la requête au serveur via le socket
 *
 * @param buffer Requête qui sera envoyé
 */
void client_send_query(char *buffer)
{
	buffer[strlen(buffer) - 1] = '\0';
	if (!send_message(sock, buffer))
		cerr << "Error: client_send_query(): " << strerrorname_np(errno) << endl;
}
/**
 * @brief Reçois les résultats du serveur via le socket
 *
 * @param buffer Buffer qui recevra le résultat
 */
void client_recv_result(char *buffer)
{
	// Reçois d'un message
	while ((recv_message(sock, buffer)))
	{
		// Envoi de la confirmation de réception
		send_message(sock, RECEIVED_MESSAGE);
		// Vérifier si c'est la fin du message
		if (strcmp(buffer, END_OF_MESSAGE) == 0)
		{
			break;
		}
		cout << buffer;
	}
}
/**
 * @brief Fonction principale contenant la boucle de récupération et envoi de requêtes, ainsi
 * que la réception des résultats
 */
void client_run()
{
	char buffer[BUFFER_SIZE];
	cout << ">";
	while ((fgets(buffer, sizeof(buffer), stdin)) != NULL)
	{
		// Envoi de la query vers le server
		client_send_query(buffer);
		// Attente de réponse du server
		client_recv_result(buffer);
		// Reset du buffer
		memset(buffer, 0, sizeof(buffer));
		cout << ">";
	}
}

int main(int argc, const char *argv[])
{
	// Configuration du gestionnaire de signal
	client_configure_signal_handler();
	if (argc < 2)
	{
		cout << "Paramètre IP obligatoire non indiqué." << endl;
		exit(1);
	}
	// Création et Paramétrage du socket
	struct sockaddr_in serv_addr;
	client_configure_socket(serv_addr, argv[1]);
	// Connexion au serveur
	client_connect_server(serv_addr);
	// Récupérarion de la requête
	client_run();
	// Fermeture du socket
	close(sock);
	return 0;
}
