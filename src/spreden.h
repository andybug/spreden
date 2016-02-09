#ifndef SPREDEN_H
#define SPREDEN_H

#include <stdbool.h>
#include <limits.h>

#include <uuid/uuid.h>

#include "list.h"

#define SPREDEN_VERSION_MAJOR 0
#define SPREDEN_VERSION_MINOR 1

#define DEFAULT_SCRIPTS_DIR "/usr/share/spreden/scripts"
#define DEFAULT_DATA_DIR    "/usr/share/spreden/data"

#define WEEK_ID_NONE   (-1)
#define WEEK_ID_BEGIN  SHRT_MIN
#define WEEK_ID_END    SHRT_MAX

#define TEAM_NAME_MAX    32
#define TEAM_SCHED_MAX  256

enum action {
	ACTION_ANALYZE,
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

struct team {
	char name[TEAM_NAME_MAX];
	int sched[TEAM_SCHED_MAX];
	int sched_len;
};

struct game;

/* rc contains user-defined parameters */
struct rc {
	enum action action;
	const char *sport;
	struct week_id data_begin;
	struct week_id data_end;
	struct week_id target_begin;
	struct week_id target_end;
	struct list user_algorithms;
	struct list script_dirs;
	struct list data_dirs;
};

struct db;

struct state {
	struct rc rc;
	struct db *db;
};


/* global data */

/* rc.c */
extern const char *progname;
extern bool verbose;


/* functions */

/* rc.c */
extern int rc_read_options(struct state *s, int argc, char **argv);

/* db.c */
extern int db_load(struct state *s);

#endif
