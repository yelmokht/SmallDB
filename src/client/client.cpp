#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include <iostream>
#include <string>

#include "../common.hpp"

using namespace std;

int sock;

void handler(int signum)
{
   switch (signum)
   {
   case SIGPIPE:
      cout << "Server connection lost..." << endl;
      cout << "Closing client." << endl;
      close(sock);
      exit(1);
      break;
   case SIGINT:
      cout << endl
           << "Closing client" << endl;
      sendSocket(sock, (char*)("DISCONNECTED"));
      close(sock);
      exit(0);
      break;
   default:
      break;
   }
}

int main(int argc, char const *argv[])
{
   signal(SIGPIPE, handler);
   signal(SIGINT, handler);
   if (argc < 2)
   {
      cout << "Paramètre IP obligatoire non indiqué." << endl;
      exit(1);
   }
   // Création et Paramétrage du socket
   sock = socket(AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in serv_addr;
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(28772);

   // Conversion de string vers IPv4 ou IPv6 en binaire
   inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

   // Connexion au serveur
   while ((connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
   {
      cout << "Trying connection with server..." << endl;
      sleep(1);
   }
   cout << "Connexion established!" << endl;
   // Récupérarion de la requête
   char buffer[1024];
   uint32_t length;
   cout << ">";
   while ((fgets(buffer, sizeof(buffer), stdin)) != NULL)
   {
      // Envoi via socket
      buffer[strlen(buffer) - 1] = '\0';
      if (!sendSocket(sock, buffer))
         cout << "Erreur sending message to server" << endl;
      // Attente de réponse
      while ((recv_exactly(sock, (char *)&length, 4)) && (recv_exactly(sock, buffer, ntohl(length))))
      {
         if (strcmp(buffer, "-1") == 0)
         {
            break;
         }
         cout << buffer << " "
              << "(" << ntohl(length) << ")" << endl;
      }
      memset(buffer, 0, sizeof(buffer));
      cout << ">";
   }
   close(sock);
   return 0;
}
