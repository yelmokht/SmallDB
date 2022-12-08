#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <string>

#include "common.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
   if (argc < 2)
   {
      printf("Paramètre IP obligatoire non indiqué.\n");
      exit(1);
   }
   signal(SIGPIPE, SIG_IGN);
   // Création et Paramétrage du socket
   int sock = socket(AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in serv_addr;
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(28772);
   // Conversion de string vers IPv4 ou IPv6 en binaire
   inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
   while ((connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
   {
      cout << "Trying connection with server..." << endl;
      sleep(1);
   }
   cout << "Connexion established!" << endl;

   // Récuperer requête
   char buffer[1024];
   uint32_t length;
   cout << ">";
   while ((fgets(buffer, sizeof(buffer), stdin)) != NULL)
   {
      buffer[strlen(buffer) - 1] = '\0';
      // Envoi via socket
      if (!sendSocket(sock, buffer))
         cout << "Erreur sending message to server" << endl;
      // Attente de réponse
      while((recv_exactly(sock, (char*)&length, 4)) && (recv_exactly(sock, buffer, ntohl(length))))
      {
         if (buffer == to_string(EOF)) {
            break;
         }
         cout << buffer;

      }
      memset(buffer,0,sizeof(buffer));
      cout << endl << ">";
   }

   close(sock);
   return 0;
}
