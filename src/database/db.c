#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <uuid/uuid.h>

#include "../spreden.h"
#include "../dstruct/list.h"
#include "database.h"


/* hash functions */

int hash_add(struct db *db, const char *uuid, int team)
{
	uuid_t temp;
	struct team_hash_entry *entry;

	/* make sure it's a valid uuid */
	if (uuid_parse(uuid, temp) < 0) {
		fprintf(stderr, "%s: invalid uuid '%s'\n", progname, uuid);
		return -1;
	}

	/* allocate hash entry */
	entry = malloc(sizeof(struct team_hash_entry));
	if (!entry) {
		fprintf(stderr, "%s: malloc failed\n", progname);
		return -2;
	}

	/* copy fields */
	strncpy(entry->uuid, uuid, UUID_LENGTH);
	entry->uuid[UUID_LENGTH] = '\0';
	entry->team = team;

	/* add to hash table */
	HASH_ADD_STR(db->teams_hash, uuid, entry);

	return 0;
}

int hash_get(struct db *db, const char *uuid)
{
	const struct team_hash_entry *entry;

	HASH_FIND_STR(db->teams_hash, uuid, entry);
	if (entry)
		return entry->team;

	return -1;
}

static void db_print_sizes(const struct db *db)
{
	(void)db;

	fprintf(stderr, "db: DB_MAX_TEAMS = %u\n", DB_MAX_TEAMS);
	fprintf(stderr, "db: DB_MAX_GAMES = %u\n", DB_MAX_GAMES);
	fprintf(stderr, "db: DB_MAX_WEEKS = %u\n", DB_MAX_WEEKS);
	fprintf(stderr, "db: sizeof(team) = %lu\n", sizeof(struct team));
	fprintf(stderr, "db: sizeof(game) = %lu\n", sizeof(struct game));
	fprintf(stderr, "db: sizeof(week) = %lu\n", sizeof(struct week));
	fprintf(stderr, "db: total alloc teams = %lu\n",
		sizeof(struct team) * DB_MAX_TEAMS);
	fprintf(stderr, "db: total alloc games = %lu\n",
		sizeof(struct game) * DB_MAX_GAMES);
	fprintf(stderr, "db: total alloc weeks = %lu\n",
		sizeof(struct week) * DB_MAX_WEEKS);
	fprintf(stderr, "db: total alloc db    = %lu\n", sizeof(struct db));
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
	db->teams_hash = NULL;
	db->num_teams = 0;
	db->num_games = 0;
	db->num_weeks = 0;
	list_init(&db->game_files);

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
