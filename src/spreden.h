#ifndef SPREDEN_H
#define SPREDEN_H

#include <stdbool.h>
#include <limits.h>

#include <uuid/uuid.h>

#include "list.h"

#define SPREDEN_VERSION_MAJOR 0
#define SPREDEN_VERSION_MINOR 1

#define DEFAULT_SCRIPT_DIR "/usr/share/spreden/scripts"
#define DEFAULT_DATA_DIR   "/usr/share/spreden/data"

#define DB_MAX_OBJECTS 8192

#define WEEK_ID_NONE (-1)
#define WEEK_ID_ALL  SHRT_MAX

enum action {
	ACTION_NONE,
	ACTION_PREDICT,
	ACTION_RANK,
	ACTION_USAGE,
	ACTION_VERSION
};

struct week_id {
	short year;
	short week;
};

struct team;
struct game;

/* rc contains user-defined parameters */
struct rc {
	const char *name;
	bool verbose;
	enum action action;
	const char *sport;
	struct week_id data_begin;
	struct week_id data_end;
	struct week_id action_begin;
	struct week_id action_end;
	struct list user_algorithms;
	struct list script_dirs;
	struct list data_dirs;
};

struct db;

struct state {
	struct rc rc;
	struct db *db;
};

/* rc.c */
extern void rc_init(struct rc *rc);
extern int  rc_read_options(struct rc *rc, int argc, char **argv);

/* db.c */
extern int db_init(struct state *s);
extern int db_add_team(struct state *s, const uuid_t uuid, struct team *t);
extern struct team *db_get_team(struct state *s, const uuid_t uuid);
extern int db_add_game(struct state *s, const uuid_t uuid, struct game *g);
extern struct game *db_get_game(struct state *s, const uuid_t uuid);

#endif
