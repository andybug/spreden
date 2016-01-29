#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <stdbool.h>

#include <getopt.h>

#include "spreden.h"
#include "list.h"

enum options {
	OPTION_HELP,
	OPTION_PREDICT,
	OPTION_RANK,
	OPTION_SCRIPTS,
	OPTION_VERBOSE,
	OPTION_VERSION
};

static int parse_control_string(struct state *s, const char *cs);
static int parse_algorithms(struct state *s,
			    const char *algos,
			    struct list *list);
static int update_data_range(struct state *s);
static bool validate_rc(const struct state *s);
static void print_rc(const struct state *s);

void rc_init(struct state *s)
{
	static const char *DEFAULT_NAME = "spreden";
	static const struct week_id UNSET_WEEK = { WEEK_ID_NONE, WEEK_ID_NONE };
	struct rc *rc = &s->rc;

	rc->name = DEFAULT_NAME;
	rc->verbose = false;
	rc->action = ACTION_NONE;
	rc->sport = NULL;
	rc->data_begin = UNSET_WEEK;
	rc->data_end = UNSET_WEEK;
	rc->action_begin = UNSET_WEEK;
	rc->action_end = UNSET_WEEK;
	list_init(&rc->user_algorithms);

	list_init(&rc->script_dirs);
	list_add_back(&rc->script_dirs, DEFAULT_SCRIPT_DIR);

	list_init(&rc->data_dirs);
	list_add_back(&rc->data_dirs, DEFAULT_DATA_DIR);
}

int rc_read_options(struct state *s, int argc, char **argv)
{
	int c, index, err;
	struct rc *rc = &s->rc;
	static const struct option options[] = {
		{ "help",    no_argument,       NULL, OPTION_HELP },
		{ "predict", required_argument, NULL, OPTION_PREDICT },
		{ "rank",    required_argument, NULL, OPTION_RANK },
		{ "scripts", required_argument, NULL, OPTION_SCRIPTS },
		{ "verbose", no_argument,       NULL, OPTION_VERBOSE },
		{ "version", no_argument,       NULL, OPTION_VERSION },
		{ NULL, 0, 0, 0 }
	};

	rc->name = argv[0];

	while ((c = getopt_long(argc, argv, "", options, &index)) != -1) {
		switch (c) {
		case OPTION_HELP:
			rc->action = ACTION_USAGE;
			break;
		case OPTION_PREDICT:
			rc->action = ACTION_PREDICT;
			err = week_parse_range(s, optarg,
					       &rc->action_begin,
					       &rc->action_end);
			if (err < 0)
				return -1;
			break;
		case OPTION_RANK:
			rc->action = ACTION_RANK;
			err = week_parse_range(s, optarg,
					       &rc->action_begin,
					       &rc->action_end);
			if (err < 0)
				return -1;
			break;
		case OPTION_SCRIPTS:
			list_add_front(&rc->script_dirs, optarg);
			break;
		case OPTION_VERBOSE:
			rc->verbose = true;
			break;
		case OPTION_VERSION:
			rc->action = ACTION_VERSION;
			break;
		default:
			return -1;
		}
	}

	if (rc->action == ACTION_NONE) {
		fprintf(stderr, "%s: no action specified; see --help for usage\n",
			rc->name);
		return -2;
	}

	if (optind >= argc) {
		fprintf(stderr, "%s: requires a run control string; see --help for usage\n",
			rc->name);
		return -3;
	}

	if (parse_control_string(s, argv[optind]) < 0)
		return -4;

	return 0;
}

static int parse_control_string(struct state *s, const char *cs)
{
	static const char *delim = ":";
	struct rc *rc = &s->rc;
	size_t len;
	char *local;
	char *sport, *dates, *algos;
	char *saveptr;
	struct list algorithm_list;
	struct week_id begin_date, end_date;
	int err;

	list_init(&algorithm_list);

	/* make a local copy of the control string */
	len = strlen(cs);
	local = alloca(len + 1);
	strcpy(local, cs);

	/* tokenize into components */
	sport = strtok_r(local, delim, &saveptr);
	dates = strtok_r(NULL, delim, &saveptr);
	algos = strtok_r(NULL, delim, &saveptr);

	/* parse date range */
	err = week_parse_range(s, dates, &begin_date, &end_date);
	if (err)
		return -1;

	/* parse the algortihm list */
	err = parse_algorithms(s, algos, &algorithm_list);
	if (err)
		return -2;

	/* update rc */
	rc->sport = strdup(sport);
	rc->data_begin = begin_date;
	rc->data_end = end_date;
	rc->user_algorithms = algorithm_list;

	/* do date range substitutions */
	err = update_data_range(s);
	if (err)
		return -3;

	/* make sure it is valid */
	if (!validate_rc(s))
		return -4;

	if (rc->verbose)
		print_rc(s);

	return 0;
}

static int parse_algorithms(struct state *s,
			    const char *algos,
			    struct list *list)
{
	static const char *delim = ",";
	size_t len;
	char *saveptr;
	char *local;
	char *a;
	int i = 0;

	/* make local copy of algos string */
	len = strlen(algos);
	local = alloca(len + 1);
	strcpy(local, algos);

	/* tokenize into list */
	a = strtok_r(local, delim, &saveptr);
	while (a) {
		list_add_back(list, strdup(a));
		a = strtok_r(NULL, delim, &saveptr);
		i++;
	}

	if (i == 0) {
		fprintf(stderr, "%s: no algorithms provided in run control\n",
			s->rc.name);
		return -1;
	}

	return 0;
}

static int update_data_range(struct state *s)
{
	struct rc *rc = &s->rc;

	if (rc->data_begin.week == WEEK_ID_ALL)
		rc->data_begin.week = 1;

	if (rc->data_end.year == rc->action_end.year &&
	    rc->data_end.week == WEEK_ID_ALL) {
		rc->data_end.week = rc->action_end.week;
	}

	return 0;
}

static bool validate_rc(const struct state *s)
{
	const struct rc *rc = &s->rc;

	if (rc->data_begin.year > rc->data_end.year) {
		fprintf(stderr, "%s: begin date is after end date\n", rc->name);
		return false;
	} else if (rc->data_begin.year == rc->data_end.year &&
		   rc->data_begin.week > rc->data_end.week) {
		fprintf(stderr, "%s: begin date is after end date\n", rc->name);
		return false;
	} else if ((rc->data_end.year == rc->action_end.year &&
		    rc->data_end.week > rc->action_end.week) ||
		   (rc->data_end.year > rc->action_end.year)) {
		fprintf(stderr, "%s: end date is after target date\n",
			rc->name);
		return false;
	}

	return true;
}

static void print_rc(const struct state *s)
{
	const struct rc *rc = &s->rc;

	const char *action = "default";

	/* heading */
	fputs("***** Run Control *****\n", stderr);

	/* print action */
	switch (rc->action) {
	case ACTION_PREDICT:
		action = "predict";
		break;
	case ACTION_RANK:
		action = "rank";
		break;
	default:
		break;
	}
	fprintf(stderr, "action: %s\n", action);

	/* print sport */
	fprintf(stderr, "sport:  %s\n", rc->sport);

	/* print beginning week */
	if (rc->data_begin.week == WEEK_ID_ALL) {
		fprintf(stderr, "begin:  %d\n",
			rc->data_begin.year);
	} else {
		fprintf(stderr, "begin:  %d week %d\n",
			rc->data_begin.year,
			rc->data_begin.week);
	}

	/* print ending week */
	if (rc->data_end.week == WEEK_ID_ALL) {
		fprintf(stderr, "end:    %d last week\n",
			rc->data_end.year);
	} else {
		fprintf(stderr, "end:    %d week %d\n",
			rc->data_end.year,
			rc->data_end.week);
	}

	/* print action week */
	if (rc->action_end.week == WEEK_ID_ALL) {
		fprintf(stderr, "target: %d last week\n",
			rc->action_end.year);
	} else {
		fprintf(stderr, "target: %d week %d\n",
			rc->action_end.year,
			rc->action_end.week);
	}

	/* print algos */
	fputs("algos:  [ ", stderr);
	struct list_iter iter;
	char *algo;
	list_iter_begin(&rc->user_algorithms, &iter);
	while (!list_iter_end(&iter)) {
		algo = list_iter_data(&iter);
		fprintf(stderr, "%s ", algo);
		list_iter_next(&iter);
	}
	fputs("]\n", stderr);

	/* footer */
	fputs("***********************\n", stderr);
}
