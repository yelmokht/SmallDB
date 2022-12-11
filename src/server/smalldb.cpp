#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <err.h> // err
#include <pthread.h>
#include <semaphore.h>

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <utility>

#include "common.hpp"
#include "db.hpp"
#include "queries.hpp"
#include "errorcodes.hpp"

using namespace std;

typedef struct
{
	int client_id;
	int tid;
	int sock;
	database_t *db;
} client_t;

database_t *DB;
vector<client_t *> clients;
mutex_t mutex;

/**
 * Create and configure the server socket
 */
void createSocket(int &server_fd, struct sockaddr_in &address)
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	// Paramétrage du socket
	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(28772);
}

void disconnectClient(client_t *client)
{
	int id = client->client_id;
	// warnx("Client %d disconnected.", client->client_id);
	vector<client_t *>::iterator client_iterator = find(clients.begin(), clients.end(), client);
	close(client->sock);
	free(client);
	clients.erase(client_iterator);
}

void *service(void *args)
{
	client_t *client = (client_t *)args;
	//*Traitement de la requête
	char buffer[1024];
	uint32_t length;
	while ((recv_exactly(client->sock, (char *)&length, 4)) && (recv_exactly(client->sock, buffer, ntohl(length))))
	{
		/* cout << "Message reçu "
			 << "(" << ntohl(length) << "): " << buffer << endl; */
		parse_and_execute(client->sock, client->db, buffer, &mutex);
	}
	disconnectClient(client);
	return NULL;
}

/**
 * Reads results from file and send it through the socket
 */
void sendResults(int sock)
{
	ifstream file("temp.txt");
	string b;
	char buffer[1024];
	while (getline(file, b))
	{
		strcpy(buffer, b.c_str());
		buffer[strlen(buffer)] = '\n';
		if (!sendSocket(sock, buffer))
		{
			cerr << "Message not sent" << endl;
		}
	}
	file.close();
	sprintf(buffer, "%d", EOF);
	sendSocket(sock, buffer);
}

void handler(int signum)
{
	switch (signum)
	{
	case SIGUSR1:
		cout << "Received SIGUSR1 signal!" << endl;
		db_save(DB);
		cout << "DB saved successfully!" << endl;
		break;
	case SIGINT:
		cout << endl
			 << "Received SIGINT signal!" << endl;
		cout << "Committing database changes to the disk..." << endl;
		db_save(DB);
		for (auto client : clients)
		{
			disconnectClient(client);
		}
		cout << "Done." << endl;
		exit(0);
		break;
	default:
		break;
	}
}

void client_t_init(client_t *args, int sock, database_t *db)
{
	args->client_id = (int)clients.size() + 1;
	args->sock = sock;
	args->db = db;
}

void block_signals(sigset_t &mask)
{
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
}

void configure_signal_handler(struct sigaction &action)
{
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &action, NULL);
	sigaction(SIGINT, &action, NULL);
}

int main(int argc, char *argv[])
{
	//*Lancement de base de donnée
	if (argc < 2)
	{
		cout << "Paramètre obligatoire non fourni: chemin vers la db" << endl;
	}
	database_t db;
	DB = &db;
	char *db_path = argv[1];
	db_load(&db, db_path);
	// Signaux
	struct sigaction action;
	sigset_t mask;
	configure_signal_handler(action);
	block_signals(mask); // Signaux à bloquer pour les threads fils
	// Initialisation des mutex
	pthread_mutex_init(&mutex.new_access, NULL);
	pthread_mutex_init(&mutex.write_access, NULL);
	pthread_mutex_init(&mutex.reader_registration, NULL);
	// Création et paramétrage du socket
	int server_fd;
	struct sockaddr_in address;
	createSocket(server_fd, address);
	bind(server_fd, (struct sockaddr *)&address, sizeof(address)); // Mise à l'écoute
	// warnx("Waiting client connection...");
	listen(server_fd, 10);
	// Acceptation
	int new_socket;
	size_t addrlen = sizeof(address);
	while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) > 0)
	{
		pthread_t tid;
		client_t *args = (client_t *)malloc(sizeof(client_t));
		client_t_init(args, new_socket, &db);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		args->tid = pthread_create(&tid, NULL, service, args);
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
		clients.push_back(args);
		// warnx("Client connected (%lu).", clients.size());
	}
	perror("accept: ");
	cout << "new_socket: "<< new_socket << endl;
	close(server_fd);
	return 0;
}
