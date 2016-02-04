#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <stdbool.h>

#include <getopt.h>

#include "spreden.h"
#include "list.h"

enum command {
	COMMAND_ANALYZE,
	COMMAND_HELP,
	COMMAND_PREDICT,
	COMMAND_RANK,
	COMMAND_VERSION,
	/* error */
	COMMAND_ERROR
};

enum options {
	OPTION_DATA,
	OPTION_SCRIPTS,
	OPTION_START,
	OPTION_VERBOSE
};


/* program name from the command line */
const char *prog_name = "spreden";

/* verbose mode from the command line */
bool verbose = false;


/* week_id functions */

static int week_parse(const char *date, struct week_id *out)
{
	static const char *week_delim = "rw";
	static const char *end_delim = "\0";
	size_t len;
	char *local;
	char *saveptr;
	char *endptr;
	char *year, *week;

	/* make local copy of date string */
	len = strlen(date);
	local = alloca(len);
	strcpy(local, date);

	/* tokenize */
	year = strtok_r(local, week_delim, &saveptr);
	week = strtok_r(NULL, end_delim, &saveptr);

	/* convert year string to integer */
	out->year = strtol(year, &endptr, 10);
	if (*endptr != '\0') {
		fprintf(stderr, "%s: '%s' is not a valid year\n",
			prog_name, year);
		return -1;
	}

	/* if a week was provided, convert it to an integer */
	if (week) {
		out->week = strtol(week, &endptr, 10);
		if (*endptr != '\0') {
			fprintf(stderr, "%s: '%s' is not a valid week\n",
				prog_name, week);
			return -2;
		}
	} else {
		out->week = WEEK_ID_NONE;
	}

	return 0;
}

static int week_parse_range(const char *range,
			    struct week_id *begin,
			    struct week_id *end)
{
	static const char *range_delim = "-";
	size_t len;
	char *local;
	char *date1, *date2;
	char *saveptr;
	int err;

	if (!range) {
		fprintf(stderr, "%s: no date range provided in run control\n",
			prog_name);
		return -1;
	}

	/* make local copy of date range string */
	len = strlen(range);
	local = alloca(len);
	strcpy(local, range);

	/* tokenize */
	date1 = strtok_r(local, range_delim, &saveptr);
	date2 = strtok_r(NULL, range_delim, &saveptr);

	/* read begin date */
	err = week_parse(date1, begin);
	if (err)
		return -2;

	/* if end date provided, read that too */
	if (date2) {
		err = week_parse(date2, end);
		if (err)
			return -3;
	} else {
		/* otherwise, read the entire year */
		end->year = begin->year;
		end->week = WEEK_ID_END;
	}

	return 0;
}

/* misc helpers */

static int update_data_range(struct rc *rc)
{
	if (rc->data_begin.week == WEEK_ID_BEGIN)
		rc->data_begin.week = 1;

	if (rc->data_end.year == rc->action_end.year &&
	    rc->data_end.week == WEEK_ID_END) {
		rc->data_end.week = rc->action_end.week;
	}

	return 0;
}

static bool validate_rc(const struct rc *rc)
{
	if (rc->data_begin.year > rc->data_end.year) {
		fprintf(stderr, "%s: begin date is after end date\n",
			prog_name);
		return false;
	} else if (rc->data_begin.year == rc->data_end.year &&
		   rc->data_begin.week > rc->data_end.week) {
		fprintf(stderr, "%s: begin date is after end date\n",
			prog_name);
		return false;
	} else if ((rc->data_end.year == rc->action_end.year &&
		    rc->data_end.week > rc->action_end.week) ||
		   (rc->data_end.year > rc->action_end.year)) {
		fprintf(stderr, "%s: end date is after target date\n",
			prog_name);
		return false;
	}

	return true;
}

static void print_rc(const struct rc *rc)
{
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
	if (rc->data_begin.week == WEEK_ID_BEGIN) {
		fprintf(stderr, "begin:  %d\n",
			rc->data_begin.year);
	} else {
		fprintf(stderr, "begin:  %d week %d\n",
			rc->data_begin.year,
			rc->data_begin.week);
	}

	/* print ending week */
	if (rc->data_end.week == WEEK_ID_END) {
		fprintf(stderr, "end:    %d last week\n",
			rc->data_end.year);
	} else {
		fprintf(stderr, "end:    %d week %d\n",
			rc->data_end.year,
			rc->data_end.week);
	}

	/* print action week */
	if (rc->action_end.week == WEEK_ID_END) {
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


/* command line parsing */

static enum command get_command(const char *cmd)
{
	enum command ret = COMMAND_ERROR;

	if (strcmp(cmd, "analyze") == 0)
		ret = COMMAND_ANALYZE;
	else if (strcmp(cmd, "help") == 0)
		ret = COMMAND_HELP;
	else if (strcmp(cmd, "predict") == 0)
		ret = COMMAND_PREDICT;
	else if (strcmp(cmd, "rank") == 0)
		ret = COMMAND_RANK;
	else if (strcmp(cmd, "version") == 0)
		ret = COMMAND_VERSION;
	else
		fprintf(stderr, "%s: unknown command '%s'; run \"spreden help\" for usage\n",
			prog_name, cmd);

	return ret;
}

static int parse_algorithms(const char *algos, struct list *list)
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
			prog_name);
		return -1;
	}

	return 0;
}

/* handle <sport> <round> <algorithms> argument chain */
static int parse_rc_args(struct state *s, int argc, char **argv)
{
	int err;
	const char *sport;
	struct week_id begin_date, end_date;
	struct list algorithm_list;
	struct rc *rc = &s->rc;

	/* make sure there are enough arguments */
	if (argc < 3) {
		fprintf(stderr, "%s: command requires three arguments (sport, round, and algorithms)\n",
			prog_name);
		return -1;
	}

	/* the first argument is sport; just copy it */
	sport = strdup(argv[0]);

	/* parse action range */
	err = week_parse_range(argv[1], &begin_date, &end_date);
	if (err)
		return -2;

	/* parse algortihm list */
	list_init(&algorithm_list);
	err = parse_algorithms(argv[2], &algorithm_list);
	if (err)
		return -2;

	/* update rc */
	rc->sport = strdup(sport);
	rc->action_begin = begin_date;
	rc->action_end = end_date;
	rc->user_algorithms = algorithm_list;

	/* fix data ranges for action dates */
	if (!update_data_range(rc))
		return -3;

	/* make sure it is valid */
	if (!validate_rc(rc))
		return -4;

	if (verbose)
		print_rc(rc);

	return 0;
}


/* extern functions */

void rc_init(struct state *s)
{
	static const struct week_id BEGIN_WEEK = {
		.year = WEEK_ID_BEGIN,
		.week = WEEK_ID_BEGIN
	};
	static const struct week_id END_WEEK = {
		.year = WEEK_ID_END,
		.week = WEEK_ID_END
	};
	struct rc *rc = &s->rc;

	rc->action = ACTION_NONE;
	rc->sport = NULL;
	rc->data_begin = BEGIN_WEEK;
	rc->data_end = END_WEEK;
	rc->action_begin = BEGIN_WEEK;
	rc->action_end = END_WEEK;
	list_init(&rc->user_algorithms);

	list_init(&rc->script_dirs);
	list_add_back(&rc->script_dirs, DEFAULT_SCRIPTS_DIR);

	list_init(&rc->data_dirs);
	list_add_back(&rc->data_dirs, DEFAULT_DATA_DIR);
}

int rc_read_options(struct state *s, int argc, char **argv)
{
	struct rc *rc = &s->rc;
	enum command cmd = COMMAND_HELP;
	int err;

	/* save off how the program was called for error displays */
	prog_name = argv[0];

	/*
	 * if a command was provided, try to parse it
	 * otherwise, fall through to help
	 */
	if (argc >= 2)
		cmd = get_command(argv[1]);

	switch (cmd) {
	case COMMAND_ANALYZE:
		rc->action = ACTION_ANALYZE;
		break;
	case COMMAND_HELP:
		rc->action = ACTION_USAGE;
		break;
	case COMMAND_PREDICT:
		rc->action = ACTION_PREDICT;
		break;
	case COMMAND_RANK:
		rc->action = ACTION_RANK;
		break;
	case COMMAND_VERSION:
		rc->action = ACTION_VERSION;
		break;
	case COMMAND_ERROR:
		return -1;
	}

	/* handle <sport> <round> <algorithms> */
	if (rc->action == ACTION_ANALYZE ||
	    rc->action == ACTION_PREDICT ||
	    rc->action == ACTION_RANK) {
		err = parse_rc_args(s, argc-2, argv+2);
		if (err)
			return -2;
	}

	return 0;
}
