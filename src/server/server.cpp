#include <arpa/inet.h>
#include <err.h> // err
#include <netinet/tcp.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <deque>

#include "../common.hpp"
#include "server.hpp"

using namespace std;

database_t *DB;
vector<client_t *> clients;
mutex_t mutex;

void config_socket_opt(int sock)
{
	int opt = 1;
	checked(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)));
}

void close_client_socket(client_t *client)
{
	checked(close(client->sock));
}

void close_client_thread(client_t *client)
{
	vector<client_t *>::iterator client_iterator = find(clients.begin(), clients.end(), client);
	pthread_t tid_save = client->tid;
	free(client);
	clients.erase(client_iterator);
	pthread_cancel(tid_save);
}

void server_disconnect_client(client_t *client)
{
	warnx("Client %d disconnected (normal). Closing connection and thread", client->client_id);
	close_client_socket(client);
	close_client_thread(client);
}

void server_lost_client_connection(client_t *client)
{
	warnx("Lost connection to client %d ", client->client_id);
	warnx("Closing connection %d ", client->client_id);
	close_client_socket(client);
	warnx("Closing Thread for connection %d ", client->client_id);
	close_client_thread(client);
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
		cout << "\nReceived SIGINT signal!" << endl;
		for (auto client : clients)
		{
			warnx("Disconnecting client %d...", client->client_id);
			server_disconnect_client(client);
		}
		cout << "Committing database changes to the disk..." << endl;
		db_save(DB);
		cout << "Done." << endl;
		exit(0);
		break;
	default:
		break;
	}
}

void client_init(client_t *client, int sock, database_t *db)
{
	client->client_id = (int)clients.size() + 1;
	client->sock = sock;
	client->status = CLIENT_CONNECTED;
	client->db = db;
}

void server_block_signals(sigset_t &mask)
{
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
}

void server_configure(server_t *server, struct sockaddr_in &address)
{
	DB = server->db;
	server->sock = checked(socket(AF_INET, SOCK_STREAM, 0));
	config_socket_opt(server->sock);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(28772);
	checked(bind(server->sock, (struct sockaddr *)&address, sizeof(address)));
	checked(listen(server->sock, 10));
}

void server_configure_signal_handler(struct sigaction &action)
{
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &action, NULL);
	sigaction(SIGINT, &action, NULL);
}

void server_configure_mutex(mutex_t &mutex)
{
	pthread_mutex_init(&mutex.new_access, NULL);
	pthread_mutex_init(&mutex.write_access, NULL);
	pthread_mutex_init(&mutex.reader_registration, NULL);
}

void *client_thread(void *args)
{
	client_t *client = (client_t *)args;
	vector<string> messages;
	char buffer[BUFFER_SIZE];
	while (true)
	{
		switch(client->status) {
			case CLIENT_CONNECTED :
			if (!recv_message(client->sock, buffer)) {
				client->status = CLIENT_LOST_CONNECTION;
				break;
			}
			if (strcmp(buffer, DISCONNECT_MESSAGE) == 0)
			{
				client->status = CLIENT_DISCONNECTED;
				break;
			}
			messages = parse_and_execute(client->sock, client->db, buffer, &mutex);
			send_protocol(client, messages);
				break;
			case CLIENT_DISCONNECTED :
				server_disconnect_client(client);
				return NULL;
				break;
			case CLIENT_LOST_CONNECTION :
				server_lost_client_connection(client);
				return NULL;
				break;
			default :
				break;
		}
	}
	return NULL;
}

int server_run(server_t *server)
{
	// Création et paramétrage du socket
	struct sockaddr_in address;
	size_t addrlen = sizeof(address);
	server_configure(server, address);
	// Signaux
	struct sigaction action;
	sigset_t mask;
	server_configure_signal_handler(action);
	server_block_signals(mask);
	// Initialisation des mutex
	server_configure_mutex(mutex);
	// Acceptation des nouvelles connexions
	while (1)
	{
		int new_socket = checked(accept(server->sock, (struct sockaddr *)&address, (socklen_t *)&addrlen));
		client_t *client = (client_t *)malloc(sizeof(client_t));
		client_init(client, new_socket, server->db);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		checked(pthread_create(&client->tid, NULL, client_thread, client));
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
		clients.push_back(client);
		warnx("Accepted connection (%d).", client->client_id);
	}
	checked(close(server->sock));
	return 0;
}
