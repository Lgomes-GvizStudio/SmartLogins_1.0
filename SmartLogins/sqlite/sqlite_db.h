#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include <sqlite3.h>

// Funções para inicializar e fechar o banco de dados
int initDatabase(const char *dbName, sqlite3 **db);
void closeDatabase(sqlite3 *db);

// Funções para executar comandos SQL
int executeSQL(sqlite3 *db, const char *sql);

// Funções para inserir e consultar dados
int insertPlayer(sqlite3 *db, const char *name, int score);
void queryPlayers(sqlite3 *db);

#endif // SQLITE_DB_H
