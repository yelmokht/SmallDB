#include <iostream>

#include "server.hpp"
#include "db.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	//*Lancement de base de donnée
	if (argc < 2)
	{
		cout << "Paramètre obligatoire non fourni: chemin vers la db" << endl;
	}
	database_t db;
	const char *db_path = argv[1];
	db_load(&db, db_path);
    server_t server;
    server.db = &db;
    server_run(&server);
    return 0;
}