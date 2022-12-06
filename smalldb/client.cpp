#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include "common.hpp"

using namespace std; 

//TODO: Safe_send et safe_read

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
   uint32_t longueur;
   //Récuperer requête
   while ((fgets(buffer, sizeof(buffer), stdin)) != NULL)
   {  
      buffer[strlen(buffer)+1] = '\0';
      longueur = htonl(strlen(buffer));
      cout << "Envoi..." << endl;
      //Envoi via socket
      write(sock, &longueur, 4);
      longueur = ntohl(strlen(buffer));
      write(sock, &buffer, longueur);
      //Attente de réponse
   }

   close(sock);
   return 0;
}
