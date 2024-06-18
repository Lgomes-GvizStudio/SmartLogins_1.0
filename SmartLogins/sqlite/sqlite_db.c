#include "sqlite_db.h"
#include <stdio.h>

int initDatabase(const char *dbName, sqlite3 **db) {
    int rc = sqlite3_open(dbName, db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
        return rc;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }
    return SQLITE_OK;
}

void closeDatabase(sqlite3 *db) {
    sqlite3_close(db);
}

int executeSQL(sqlite3 *db, const char *sql) {
    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    } else {
        fprintf(stdout, "SQL executed successfully\n");
    }
    return SQLITE_OK;
}

int insertPlayer(sqlite3 *db, const char *name, int score) {
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO PLAYER (NAME, SCORE) VALUES ('%s', %d);", name, score);
    return executeSQL(db, sql);
}

void queryPlayers(sqlite3 *db) {
    const char *sql = "SELECT ID, NAME, SCORE FROM PLAYER;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("ID\tNAME\tSCORE\n");
    printf("--\t----\t-----\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        int score = sqlite3_column_int(stmt, 2);
        printf("%d\t%s\t%d\n", id, name, score);
    }

    sqlite3_finalize(stmt);
}
