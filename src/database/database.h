#ifndef DATABASE_H
#define DATABASE_H

#include "../spreden.h"

/* db.c */
extern int hash_add(struct db *db, const char *uuid, int team);
extern int hash_get(struct db *db, const char *uuid);

/* parse_teams.c */
extern int parse_teams(struct db *db, const char *filename);

#endif
