#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <getopt.h>

#include "spreden.h"

enum spreden_options {
	SPREDEN_OPTION_HELP,
	SPREDEN_OPTION_PREDICT,
	SPREDEN_OPTION_RANK,
	SPREDEN_OPTION_SCRIPTS,
	SPREDEN_OPTION_VERBOSE,
	SPREDEN_OPTION_VERSION
};

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

static void handle_args(int argc, char **argv, struct spreden_state *state)
{
	int c, index;
	static const struct option options[] = {
		{ "help",    no_argument,       NULL, SPREDEN_OPTION_HELP },
		{ "predict", required_argument, NULL, SPREDEN_OPTION_PREDICT },
		{ "rank",    required_argument, NULL, SPREDEN_OPTION_RANK },
		{ "scripts", required_argument, NULL, SPREDEN_OPTION_SCRIPTS },
		{ "verbose", no_argument,       NULL, SPREDEN_OPTION_VERBOSE },
		{ "version", no_argument,       NULL, SPREDEN_OPTION_VERSION },
		{ NULL, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, "", options, &index)) != -1) {
		switch (c) {
		case SPREDEN_OPTION_HELP:
			state->rc.action = SPREDEN_ACTION_USAGE;
			break;
		case SPREDEN_OPTION_PREDICT:
			state->rc.action = SPREDEN_ACTION_PREDICT;
			break;
		case SPREDEN_OPTION_RANK:
			state->rc.action = SPREDEN_ACTION_RANK;
			break;
		case SPREDEN_OPTION_SCRIPTS:
			list_add_front(&state->script_dirs, optarg);
			break;
		case SPREDEN_OPTION_VERBOSE:
			state->verbose = true;
			break;
		case SPREDEN_OPTION_VERSION:
			state->rc.action = SPREDEN_ACTION_VERSION;
			break;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "%s: requires a run control string\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (rc_parse(state, argv[optind]) < 0)
		exit(EXIT_FAILURE);
}

static void init_state(struct spreden_state *state)
{
	state->verbose = false;
	state->rc.action = SPREDEN_ACTION_USAGE;
	list_init(&state->script_dirs);
	list_add_front(&state->script_dirs, SPREDEN_DEFAULT_SCRIPT_DIR);
	list_init(&state->data_dirs);
}

int main(int argc, char **argv)
{
	struct spreden_state state;

	/* display usage if no arguments provided */
	if (argc < 2) {
		display_usage();
		return EXIT_FAILURE;
	}

	init_state(&state);
	handle_args(argc, argv, &state);

	switch (state.rc.action) {
	case SPREDEN_ACTION_USAGE:
		display_usage();
		break;
	case SPREDEN_ACTION_VERSION:
		display_version();
		break;
	case SPREDEN_ACTION_RANK:
	case SPREDEN_ACTION_PREDICT:
		puts("rank/predict actions not yet implemented");
		break;
	}

	return EXIT_SUCCESS;
}
