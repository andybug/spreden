#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "spreden.h"

static void display_version(void)
{
	printf("spreden version %d.%d\n",
	       SPREDEN_VERSION_MAJOR,
	       SPREDEN_VERSION_MINOR);
}

static void display_usage(void)
{
	static const char usage[] =
		"usage: spreden <command> [args] [options]\n"
		"    commands:\n"
		"        analyze\n"
		"        help\n"
		"        predict\n"
		"        rank\n"
		"        version\n";
	fputs(usage, stdout);
}

static void init_state(struct state *state)
{
	rc_init(state);
	db_init(state);
}

int main(int argc, char **argv)
{
	struct state state;

	init_state(&state);

	if (rc_read_options(&state, argc, argv) < 0)
		return EXIT_FAILURE;

	switch (state.rc.action) {
	case ACTION_NONE:
		/* this should never happen... */
		assert(0);
		break;
	case ACTION_USAGE:
		display_usage();
		break;
	case ACTION_VERSION:
		display_version();
		break;
	case ACTION_ANALYZE:
	case ACTION_RANK:
	case ACTION_PREDICT:
		puts("action not implemented");
		break;
	}

	return EXIT_SUCCESS;
}
