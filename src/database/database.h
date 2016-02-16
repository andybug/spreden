#ifndef DATABASE_H
#define DATABASE_H

#include <uthash.h>

#include "../spreden.h"

#define UUID_LENGTH  36

#define DB_MAX_TEAMS    256
#define DB_MAX_GAMES  32768
#define DB_MAX_WEEKS    512

struct week {
	struct week_id id;
	int game_begin;
	int game_end;
};

struct team_hash_entry {
	UT_hash_handle hh;
	char uuid[UUID_LENGTH+1];
	int team;
};

struct db {
	struct team teams[DB_MAX_TEAMS];
	struct game games[DB_MAX_GAMES];
	struct week weeks[DB_MAX_WEEKS];
	struct team_hash_entry *teams_hash;
	unsigned int num_teams;
	unsigned int num_games;
	unsigned int num_weeks;
};

/* db.c */
extern int hash_add(struct db *db, const char *uuid, int team);
extern int hash_get(struct db *db, const char *uuid);

/* scan.c */
extern int db_scan(struct state *s);

/* parse_teams.c */
extern int db_parse_teams(struct db *db, const char *filename);

#endif
