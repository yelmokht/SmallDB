#include <iostream>

#include "db.hpp"
#include "server.hpp"

using namespace std;

int main(int argc, const char *argv[])
{
	if (argc < 2)
	{
		cout << "Paramètre obligatoire non fourni: chemin vers la db" << endl;
	}
	// Lancement de base de donnée
	database_t db;
	db_load(&db, argv[1]);
	// Lancement du serveur
	server_t server;
	server.db = &db;
	server_run(&server);
	return 0;
}