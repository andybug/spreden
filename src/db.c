#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <uuid/uuid.h>
#include <uthash/uthash.h>

#include "spreden.h"

#define DB_MAX_TEAMS    256
#define DB_MAX_GAMES  32768

struct db {
	struct team teams[DB_MAX_TEAMS];
	unsigned int num_teams;
	struct game games[DB_MAX_GAMES];
	unsigned int num_games;
};


static void db_print_sizes(const struct db *db)
{
	fprintf(stderr, "db: DB_MAX_TEAMS = %u\n", DB_MAX_TEAMS);
	fprintf(stderr, "db: DB_MAX_GAMES = %u\n", DB_MAX_GAMES);
	fprintf(stderr, "db: sizeof(team) = %lu\n", sizeof(struct team));
	fprintf(stderr, "db: sizeof(game) = %lu\n", sizeof(struct game));
	fprintf(stderr, "db: total alloc teams = %lu\n",
		sizeof(struct team) * DB_MAX_TEAMS);
	fprintf(stderr, "db: total alloc games = %lu\n",
		sizeof(struct game) * DB_MAX_GAMES);
}

static int db_init(struct state *s)
{
	size_t alloc_size;
	struct db *db;

	/* allocate the db all at once */
	alloc_size = sizeof(struct db);
	db = malloc(alloc_size);
	if (!db) {
		fprintf(stderr, "%s: malloc failed\n", progname);
		return -1;
	}

	/* init members */
	db->num_teams = 0;
	db->num_games = 0;

	/* finally, set the db pointer in the state */
	s->db = db;

	if (verbose)
		db_print_sizes(db);

	return 0;
}

/* api functions */

int db_load(struct state *s)
{
	assert(s->db == NULL);

	if (db_init(s) < 0)
		return -1;

	return 0;
}
