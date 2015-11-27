#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

#include "spreden.h"

enum spreden_options {
	SPREDEN_OPTION_HELP,
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
		"--scripts\n"
		"--verbose\n"
		"--version\n";
	fputs(usage, stdout);
}

int main(int argc, char **argv)
{
	int c, index;
	struct spreden_state state;
	bool version = false, usage = false;
	static const struct option options[] = {
		{ "help",    no_argument, NULL, SPREDEN_OPTION_HELP },
		{ "verbose", no_argument, NULL, SPREDEN_OPTION_VERBOSE },
		{ "version", no_argument, NULL, SPREDEN_OPTION_VERSION },
		{ NULL, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, "", options, &index)) != -1) {
		switch (c) {
		case SPREDEN_OPTION_HELP:
			usage = true;
			break;
		case SPREDEN_OPTION_VERBOSE:
			state.verbose = true;
			break;
		case SPREDEN_OPTION_VERSION:
			version = true;
			break;
		}
	}

	if (version) {
		display_version();
		return EXIT_SUCCESS;
	} else if (usage) {
		display_usage();
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}
