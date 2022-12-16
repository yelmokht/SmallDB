#ifndef __COMMON_HPP
#define __COMMON_HPP

#include <string>
#include <unistd.h>

#include "./server/server.hpp"

// Indique que je me déconnecte
#define DISCONNECT_MESSAGE (char *)"-3"
// Indique que j'ai recu le message
#define RECEIVED_MESSAGE (char *)"-2"
// Indique la fin de toutes les messages
#define END_OF_MESSAGE (char *)"-1"
// Taille du buffer d'échange de message
#define BUFFER_SIZE 256

// The macro allows us to retrieve the name of the calling function
#define checked(call) _checked(call, #call)
/**
 * @brief Vérifie la valeur de retour de l'appel système
 *
 * @param ret Valeur de retour de l'appel système
 * @param calling_function Nome de l'appel système
 * @return Valeur de retour de l'appel système
 */
int _checked(int ret, const std::string &calling_function);
/**
 *
 * @brief Reçoit un message de BUFFER_SIZE bytes
 *
 * @param fd Socket émetteur de message
 * @param buffer Buffer pour stocker message de taille BUFFER_SIZE
 * @return true Message reçu
 * @return false Connexion perdue ou error
 */
bool recv_message(int fd, char *buffer);
/**
 * @brief Envois un message de BUFFER_SIZE bytes
 *
 * @param sock Socket récepteur du message
 * @param buffer Message à envoyer de taille BUFFER_SIZE
 * @return true: Message envoyé
 * @return false: Connexion perdue ou error
 */
bool send_message(int sock, const char *buffer);
/**
 * @brief Envoi une liste de messages
 *
 * @param client Structure contenant les informations relatives au client
 * @param list Liste des messages à envoyer
 * @return  Succès de l'envoi
 */
bool send_protocol(client_t *client, std::vector<std::string> &message_list);

#endif // __COMMON_HPP
