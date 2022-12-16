#ifndef _SERVER_HPP
#define _SERVER_HPP

#include <pthread.h>

#include "db.hpp"
#include "queries.hpp"

/**
 * @brief Status de la connexion client-serveur
 */
enum status_t
{
	CLIENT_CONNECTED,
	CLIENT_DISCONNECTED,
	CLIENT_LOST_CONNECTION
};
/**
 * @brief Structure contenant les informations relatives au serveur
 */
typedef struct
{
	int sock;
	database_t *db;
} server_t;
/**
 * @brief Structure contenant les informations relatives à un client
 *
 */
typedef struct
{
	int client_id;
	int sock;
	status_t status;
	pthread_t tid;
	database_t *db;
} client_t;

/**
 * @brief Configurer les options du socket SO_REUSEADDR et SO_REUSEPORT
 *
 * @param sock socket du serveur
 */
void config_socket_opt(int sock);
/**
 * @brief Ferme le socket du client
 *
 * @param client Struct du client contenant ses informations
 */
void close_client_socket(client_t *client);
/**
 * @brief Interrompt le thread client et libère ses ressources
 *
 * @param client Struct du client contenant ses informations
 */
void close_client_thread(client_t *client);
/**
 * @brief Déconnecte le client suite à une déconnexion normal
 *
 * @param client Struct du client contenant ses informations
 */
void server_disconnect_client(client_t *client);
/**
 * @brief Déconnecte le client suite à une perte de connexion
 *
 * @param client Struct du client contenant ses informations
 */
void server_lost_client_connection(client_t *client);
/**
 * @brief Gestionnaire de signaux
 *
 * @param signum Numéro du signal reçu
 */
void handler(int signum);
/**
 * @brief Initialise les paramètres du client
 *
 * @param client Struct du client contenant ses informations
 * @param sock Socket du client
 * @param db Database
 */
void client_init(client_t *client, int sock, database_t *db);
/**
 * @brief Bloque les signaux pour les threads client(thread-fils)
 *
 * @param mask sigset_t
 */
void server_block_signals(sigset_t &mask);
/**
 * @brief Créer et configure le serveur
 *
 * @param server Struct du serveur contenant ses informations
 * @param address sockaddr_in
 */
void server_configure(server_t *server, struct sockaddr_in &address);
/**
 * @brief Configure le gestionnaire de signaux du serveur
 *
 * @param action sigaction
 */
void server_configure_signal_handler(struct sigaction &action);
/**
 * @brief Configure les mutex du mécanisme de synchronisme
 *
 * @param mutex Struct contenant tous les mutex nécessaires
 */
void server_configure_mutex(mutex_t &mutex);
/**
 * @brief Fonction du thread fils qui est responsable de communiquer avec un client
 *
 * @param args Struct du client contenant ses informations
 */
void *client_thread(void *args);
/**
 * @brief Lance le serveur
 *
 * @param server Struct du serveur contenant ses informations
 * @return int Succès/Error
 */
int server_run(server_t *server);

#endif
