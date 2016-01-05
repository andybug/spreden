#ifndef SPREDEN_H
#define SPREDEN_H

#include <stdbool.h>
#include <limits.h>
#include "list.h"

#define SPREDEN_VERSION_MAJOR 0
#define SPREDEN_VERSION_MINOR 1

#define SPREDEN_DEFAULT_SCRIPT_DIR "/usr/share/spreden/scripts"
#define SPREDEN_DEFAULT_DATA_DIR   "/usr/share/spreden/data"

#define SPREDEN_ROUND_ALL SHRT_MAX

enum spreden_action {
	SPREDEN_ACTION_PREDICT,
	SPREDEN_ACTION_RANK,
	SPREDEN_ACTION_USAGE,
	SPREDEN_ACTION_VERSION
};

struct spreden_round {
	short year;
	short round;
};

struct spreden_rc {
	enum spreden_action action;
	const char *sport;
	struct spreden_round data_begin;
	struct spreden_round data_end;
	struct spreden_round action_round;
	struct list algorithms;
};

struct spreden_state {
	bool verbose;
	struct spreden_rc rc;
	struct list script_dirs;
	struct list data_dirs;
};

extern int rc_parse(struct spreden_state *state, const char *rc);

#endif
