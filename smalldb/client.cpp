#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include "common.hpp"

using namespace std;

void sendQuery(int sock, char *buffer)
{
   //TODO: Gestion d'erreur
   uint32_t length = strlen(buffer);
   buffer[length - 1] = '\0';
   length = htonl(length);
   send(sock, &length, sizeof(length), 0);
   length = ntohl(length);
   send(sock, buffer, length, 0);
   cout << "Envoi..."
        << "(" << length << ")" << endl;
}

int main(int argc, char const *argv[])
{
   if (argc < 2)
   {
      printf("Paramètre IP obligatoire non indiqué.\n");
      exit(1);
   }
   signal(SIGPIPE, SIG_IGN);
   int sock = socket(AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in serv_addr;
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(28772);
   // Conversion de string vers IPv4 ou IPv6 en binaire
   inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

   connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

   char buffer[1024];
   uint32_t length;
   // Récuperer requête
   while ((fgets(buffer, sizeof(buffer), stdin)) != NULL)
   {
      // Envoi via socket
      sendQuery(sock, buffer);
      // Attente de réponse
      recv(sock, &buffer, 1024, 0);
      cout << buffer << endl;
   }

   close(sock);
   return 0;
}
