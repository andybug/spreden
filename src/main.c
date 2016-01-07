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
		"spreden is a work in progress. Current options:\n"
		"--help\n"
		"--predict\n"
		"--rank\n"
		"--scripts\n"
		"--verbose\n"
		"--version\n";
	fputs(usage, stdout);
}

static void init_state(struct state *state)
{
	rc_init(&state->rc);
}

int main(int argc, char **argv)
{
	struct state state;

	/* display usage if no arguments provided */
	if (argc < 2) {
		display_usage();
		return EXIT_FAILURE;
	}

	init_state(&state);

	if (rc_read_options(&state.rc, argc, argv) < 0)
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
	case ACTION_RANK:
	case ACTION_PREDICT:
		puts("rank/predict actions not yet implemented");
		break;
	}

	return EXIT_SUCCESS;
}
