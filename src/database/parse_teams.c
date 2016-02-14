#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <yajl/yajl_parse.h>

#include "../spreden.h"
#include "database.h"

enum record_key {
	KEY_NONE,
	KEY_NAME,
	KEY_UUID
};

struct context {
	struct db *db;
	const char *filename;
	unsigned int record;
	bool in_map;
	bool has_uuid;
	bool has_name;
	enum record_key current;
	char uuid[UUID_LENGTH+1];
	char name[TEAM_NAME_MAX];
};


/* callbacks */

static int cb_string(void *ctx, const unsigned char *s, size_t len)
{
	struct context *c = ctx;

	switch (c->current) {
	case KEY_NONE:
		return 0;
	case KEY_NAME:
		if (len > TEAM_NAME_MAX) {
			fprintf(stderr, "%s: team name too long in %s record %u\n",
				progname, c->filename, c->record);
			return 0;
		}
		strncpy(c->name, (char *)s, len);
		c->name[len] = '\0';
		c->has_name = true;
		break;
	case KEY_UUID:
		if (len != UUID_LENGTH) {
			fprintf(stderr, "%s: incorrect uuid length in %s record %u\n",
				progname, c->filename, c->record);
			return 0;
		}
		strncpy(c->uuid, (char *)s, UUID_LENGTH);
		c->uuid[UUID_LENGTH] = '\0';
		c->has_uuid = true;
		break;
	}

	c->current = KEY_NONE;

	return 1;
}

static int cb_start_map(void *ctx)
{
	struct context *c = ctx;

	if (c->in_map || (c->has_name || c->has_uuid)) {
		fprintf(stderr, "%s: unexpected start of map at %s record %u\n",
			progname, c->filename, c->record);
		return 0;
	}

	c->in_map = true;
	return 1;
}

static int cb_map_key(void *ctx, const unsigned char *key, size_t len)
{
	struct context *c = ctx;
	(void)len;

	if (c->current != KEY_NONE) {
		fprintf(stderr, "%s: unexpected key in %s record %u\n",
			progname, c->filename, c->record);
		return 0;
	}

	if (strncmp("name", (const char *)key, 4) == 0)
		c->current = KEY_NAME;
	else if (strncmp("uuid", (const char *)key, 4) == 0)
		c->current = KEY_UUID;
	else {
		fprintf(stderr, "%s: unknown key in %s record %u\n",
			progname, c->filename, c->record);
		return 0;
	}

	return 1;
}

static int cb_end_map(void *ctx)
{
	struct context *c = ctx;

	if (!c->in_map || !c->has_name || !c->has_uuid) {
		fprintf(stderr, "%s: unexpected end of map at %s record %u\n",
			progname, c->filename, c->record);
		return 0;
	}

	c->in_map = false;
	c->has_name = false;
	c->has_uuid = false;
	c->record++;

	return 1;
}

static const yajl_callbacks callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	cb_string,
	cb_start_map,
	cb_map_key,
	cb_end_map,
	NULL,
	NULL
};


/* helper functions */

static void init_context(struct context *c, struct db *db, const char *filename)
{
	c->db = db;
	c->filename = filename;
	c->record = 1;
	c->in_map = false;
	c->has_uuid = false;
	c->has_name = false;
	c->current = KEY_NONE;
}


/* api functions */

int db_parse_teams(struct db *db, const char *filename)
{
	static const int BUF_SIZE = 1024;
	char buf[BUF_SIZE];
	FILE *f;
	size_t read;
	yajl_handle handle;
	yajl_status status;
	struct context context;

	init_context(&context, db, filename);

	/* open the file */
	f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "%s: could not open '%s' for reading\n",
			progname, filename);
		return -1;
	}

	/* setup yajl */
	handle = yajl_alloc(&callbacks, NULL, &context);

	while ((read = fread(buf, 1, BUF_SIZE, f))) {
		status = yajl_parse(handle, (unsigned char *)buf, read);
		if (status != yajl_status_ok) {
			fprintf(stderr, "%s: json parse error in '%s'\n",
				progname, filename);
			return -2;
		}
	}

	yajl_complete_parse(handle);
	yajl_free(handle);

	return 0;
}
