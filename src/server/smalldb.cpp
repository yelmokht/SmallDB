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

// TODO: Mieux diviser le code en helpers function pour réduire taille du main
// TODO: Traitement d'erreur+

typedef struct
{
	int client_id;
	int tid;
	int sock;
	database_t *db;
} thread_args;

vector<thread_args *> list_tid;
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

void disconnectClient(thread_args *t_args)
{
	int id = t_args->client_id;
	// warnx("Client %d disconnected.", t_args->client_id);
	vector<thread_args *>::iterator t_args_iterator = find(list_tid.begin(), list_tid.end(), t_args);
	close(t_args->sock);
	free(t_args);
	list_tid.erase(t_args_iterator);
}

void *service(void *args)
{
	thread_args *t_args = (thread_args *)args;
	//*Traitement de la requête
	char buffer[1024];
	uint32_t length;
	while ((recv_exactly(t_args->sock, (char *)&length, 4)) && (recv_exactly(t_args->sock, buffer, ntohl(length))))
	{
		/* cout << "Message reçu "
			 << "(" << ntohl(length) << "): " << buffer << endl; */
		parse_and_execute(t_args->sock, t_args->db, buffer, &mutex);
	}
	disconnectClient(t_args);
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
	printf("Signal %d received", signum);
}

int main(int argc, char *argv[])
{
	//*Lancement de base de donnée
	if (argc < 2)
	{
		cout << "Paramètre obligatoire non fourni: chemin vers la db" << endl;
	}
	database_t db;
	char *db_path = argv[1];
	db_load(&db, db_path);
	//*Initialisation des mutex
	pthread_mutex_init(&mutex.new_access, NULL);
	pthread_mutex_init(&mutex.write_access, NULL);
	pthread_mutex_init(&mutex.reader_registration, NULL);
	// Création et paramétrage du socket
	int server_fd;
	struct sockaddr_in address;
	createSocket(server_fd, address);
	// Mise à l'écoute
	bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	// warnx("Waiting client connection...");
	listen(server_fd, 10);
	// Acceptation
	int new_socket;
	size_t addrlen = sizeof(address);
	while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) > 0)
	{
		pthread_t tid;
		thread_args *args = (thread_args *)malloc(sizeof(thread_args));
		args->client_id = (int)list_tid.size() + 1;
		args->sock = new_socket;
		args->db = &db;
		// pthread_mask activé
		args->tid = pthread_create(&tid, NULL, service, args);
		// pthread_mask desactivé
		list_tid.push_back(args);
		// warnx("Client connected (%lu).", list_tid.size());
	}

	if (new_socket == DISCONNECTED)
	{
		err(NETWORK_ERROR, "Unnable to established connexion with client.\n");
	}

	close(server_fd);
	return 0;
}
