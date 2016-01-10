#include <stdlib.h>
#include <string.h>

#include <uuid/uuid.h>
#include <uthash/uthash.h>

#include "spreden.h"

enum db_type {
	DB_TEAM,
	DB_CONF,
	DB_GAME
};

struct db_object {
	uuid_t uuid;
	UT_hash_handle hh;
	enum db_type type;
	void *data;
};

struct db {
	struct db_object *hash_table;
	struct db_object *objects;
	unsigned int num_objects;
};

int db_init(struct state *s)
{
	s->db = malloc(sizeof(struct db));
	if (!s->db)
		return -1;

	s->db->hash_table = NULL;
	s->db->objects = malloc(sizeof(struct db_object) * DB_MAX_OBJECTS);
	if (!s->db->objects)
		return -2;
	s->db->num_objects = 0;

	return 0;
}

static int db_add(struct state *s,
		  const uuid_t uuid,
		  enum db_type type,
		  void *data)
{
	struct db_object *o = s->db->objects + s->db->num_objects;

	if (s->db->num_objects >= DB_MAX_OBJECTS)
		return -1;

	memcpy(o->uuid, uuid, sizeof(uuid_t));
	o->type = type;
	o->data = data;

	s->db->num_objects++;
	HASH_ADD(hh, s->db->hash_table, uuid, sizeof(uuid_t), o);

	return 0;
}

int db_add_team(struct state *s, const uuid_t uuid, struct team *t)
{
	return db_add(s, uuid, DB_TEAM, t);
}

struct team *db_get_team(struct state *s, const uuid_t uuid)
{
	struct db_object *o;

	HASH_FIND(hh, s->db->hash_table, uuid, sizeof(uuid_t), o);

	if (!o || o->type != DB_TEAM)
		return NULL;

	return o->data;
}

int db_add_game(struct state *s, const uuid_t uuid, struct game *g)
{
	return db_add(s, uuid, DB_GAME, g);
}

struct game *db_get_game(struct state *s, const uuid_t uuid)
{
	struct db_object *o;

	HASH_FIND(hh, s->db->hash_table, uuid, sizeof(uuid_t), o);

	if (!o || o->type != DB_GAME)
		return NULL;

	return o->data;
}
