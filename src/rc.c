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
	OPTION_DATA_START,
	OPTION_SCRIPTS,
	OPTION_VERBOSE
};


/* program name from the command line */
const char *progname = "spreden";

/* verbose mode from the command line */
bool verbose = false;


/* week_id functions */

static void print_week(FILE *stream, const struct week_id *w)
{
	if (w->year == WEEK_ID_BEGIN) {
		fputs("earliest", stream);
	} else if (w->week == WEEK_ID_END) {
		fprintf(stream, "%d last week", w->year);
	} else {
		fprintf(stream, "%d week %d", w->year, w->week);
	}
}

static int parse_week(const char *str, struct week_id *out)
{
	static const char *week_delim = "rw";
	static const char *end_delim = "\0";
	size_t len;
	char *local;
	char *saveptr;
	char *endptr;
	char *year, *week;

	/* make local copy of date string */
	len = strlen(str);
	local = alloca(len);
	strcpy(local, str);

	/* tokenize */
	year = strtok_r(local, week_delim, &saveptr);
	week = strtok_r(NULL, end_delim, &saveptr);

	/* convert year string to integer */
	out->year = strtol(year, &endptr, 10);
	if (*endptr != '\0') {
		fprintf(stderr, "%s: '%s' is not a valid year\n",
			progname, year);
		return -1;
	}

	/* if a week was provided, convert it to an integer */
	if (week) {
		out->week = strtol(week, &endptr, 10);
		if (*endptr != '\0') {
			fprintf(stderr, "%s: '%s' is not a valid week\n",
				progname, week);
			return -2;
		}
	} else {
		out->week = WEEK_ID_NONE;
	}

	return 0;
}

static int parse_week_range(const char *range,
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
			progname);
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
	err = parse_week(date1, begin);
	if (err)
		return -2;

	/* if end date provided, read that too */
	if (date2) {
		err = parse_week(date2, end);
		if (err)
			return -3;

		/* if begin does not have a week set, make it begin */
		if (begin->week == WEEK_ID_NONE)
			begin->week = WEEK_ID_BEGIN;

		/* if end does not have a week set, make it end */
		if (end->week == WEEK_ID_NONE)
			end->week = WEEK_ID_END;
	} else {
		/* if not end date (aka not a range), unset end */
		end->year = WEEK_ID_NONE;
		end->week = WEEK_ID_NONE;

		/* and make begin's week END if not set */
		if (begin->week == WEEK_ID_NONE)
			begin->week = WEEK_ID_END;
	}

	return 0;
}

/* misc helpers */

static int update_data_range(struct rc *rc)
{
	if (rc->data_begin.week == WEEK_ID_BEGIN)
		rc->data_begin.week = 1;

	if (rc->target_begin.week == WEEK_ID_BEGIN)
		rc->target_begin.week = 1;

	if (rc->data_end.year == WEEK_ID_END)
		rc->data_end.year = rc->target_end.year;

	if (rc->data_end.year == rc->target_end.year &&
	    rc->data_end.week == WEEK_ID_END) {
		rc->data_end.week = rc->target_end.week;
	}

	return 0;
}

static bool validate_rc(const struct rc *rc)
{
	if (rc->data_begin.year > rc->data_end.year) {
		fprintf(stderr, "%s: data begin date is after end date\n",
			progname);
		return false;
	} else if (rc->data_begin.year == rc->data_end.year &&
		   rc->data_begin.week > rc->data_end.week) {
		fprintf(stderr, "%s: data begin date is after end date\n",
			progname);
		return false;
	} else if (rc->target_begin.year > rc->target_end.year) {
		fprintf(stderr, "%s: target begin date is after end date\n",
			progname);
		return false;
	} else if (rc->target_begin.year == rc->target_end.year &&
		   rc->target_begin.week > rc->target_end.week) {
		fprintf(stderr, "%s: target begin date is after end date\n",
			progname);
		return false;
	} else if (rc->data_end.year != rc->target_end.year ||
		   rc->data_end.week != rc->target_end.week) {
		fprintf(stderr, "%s: data end date does not equal target end date\n",
			progname);
		return false;
	} else if (rc->data_begin.year > rc->target_begin.year) {
		fprintf(stderr, "%s: data begin date is after target begin date\n",
			progname);
		return false;
	} else if (rc->data_begin.year == rc->target_begin.year &&
		   rc->data_begin.week > rc->target_begin.week) {
		fprintf(stderr, "%s: data begin date is after target begin date\n",
			progname);
		return false;
	}

	return true;
}

static void print_rc(const struct rc *rc)
{
	const char *action = "unknown";

	/*
	 * map action to string
	 * if not analyze, predict, or rank, then exit
	 */
	switch (rc->action) {
	case ACTION_ANALYZE:
		action = "analyze";
		break;
	case ACTION_PREDICT:
		action = "predict";
		break;
	case ACTION_RANK:
		action = "rank";
		break;
	default:
		return;
	}

	/* heading */
	fputs("***** run control *****\n", stderr);

	/* print action */
	fprintf(stderr, "action:       %s\n", action);

	/* print sport */
	fprintf(stderr, "sport:        %s\n", rc->sport);

	/* print data-begin week */
	fputs("data-begin:   ", stderr);
	print_week(stderr, &rc->data_begin);
	fputc('\n', stderr);

	/* print end week */
	fputs("data-end:     ", stderr);
	print_week(stderr, &rc->data_end);
	fputc('\n', stderr);

	/* print target week/range */
	fputs("target-begin: ", stderr);
	print_week(stderr, &rc->target_begin);
	fputc('\n', stderr);

	fputs("target-end:   ", stderr);
	print_week(stderr, &rc->target_end);
	fputc('\n', stderr);

	/* print algos */
	fputs("algos:        [ ", stderr);
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

static int parse_options(struct rc *rc, int argc, char **argv)
{
	static struct option options[] = {
		{ "data",       required_argument, NULL, OPTION_DATA },
		{ "data-begin", required_argument, NULL, OPTION_DATA_START },
		{ "scripts",    required_argument, NULL, OPTION_SCRIPTS },
		{ "verbose",    no_argument,       NULL, OPTION_VERBOSE }
	};
	int c;
	int index = 0;
	int err;

	/*
	 * parse out all of the long options - the other
	 * options will be moved to the end of argv
	 */
	while ((c = getopt_long(argc, argv, "", options, &index))) {
		if (c == -1)
			break;

		switch (c) {
		case OPTION_DATA:
			list_add_front(&rc->data_dirs, optarg);
			break;
		case OPTION_DATA_START:
			err = parse_week(optarg, &rc->data_begin);
			if (err < 0)
				return -1;
			/* if a week was not specified, start at beginning */
			if (rc->data_begin.week == WEEK_ID_NONE)
				rc->data_begin.week = WEEK_ID_BEGIN;
			break;
		case OPTION_SCRIPTS:
			list_add_front(&rc->script_dirs, optarg);
			break;
		case OPTION_VERBOSE:
			verbose = true;
			break;
		case '?':
			break;
		}
	}

	/*
	 * return the index into argv that contains
	 * the rest of the options
	 */
	return optind;
}

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
			progname, cmd);

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
			progname);
		return -1;
	}

	return 0;
}

/* handle <sport> <target week(s)> <algorithms> argument chain */
static int parse_rc_args(struct rc *rc, int argc, char **argv)
{
	int err;
	const char *sport;
	struct week_id begin_date, end_date;
	struct list algorithm_list;

	/* make sure there are enough arguments */
	if (argc < 3) {
		fprintf(stderr, "%s: command requires three arguments (sport, target week(s), and algorithms)\n",
			progname);
		return -1;
	}

	/* the first argument is sport; just copy it */
	sport = strdup(argv[0]);

	/* parse action range */
	err = parse_week_range(argv[1], &begin_date, &end_date);
	if (err)
		return -2;

	/* if not a range, set end = begin */
	if (end_date.year == WEEK_ID_NONE)
		end_date = begin_date;

	/* parse algortihm list */
	list_init(&algorithm_list);
	err = parse_algorithms(argv[2], &algorithm_list);
	if (err)
		return -2;

	/* update rc */
	rc->sport = strdup(sport);
	rc->target_begin = begin_date;
	rc->target_end = end_date;
	rc->user_algorithms = algorithm_list;

	/* fix data ranges for action dates */
	err = update_data_range(rc);
	if (err)
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
	static const struct week_id NONE_WEEK = {
		.year = WEEK_ID_NONE,
		.week = WEEK_ID_NONE
	};
	struct rc *rc = &s->rc;

	rc->action = ACTION_NONE;
	rc->sport = NULL;
	rc->data_begin = BEGIN_WEEK;
	rc->data_end = END_WEEK;
	rc->target_begin = NONE_WEEK;
	rc->target_end = NONE_WEEK;
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
	int cmd_index;

	/* save off how the program was called for error displays */
	progname = argv[0];

	/*
	 * parse all long options
	 * cmd_index will contain the index into argc
	 * that will hold the command
	 */
	if ((cmd_index = parse_options(rc, argc, argv)) < 0)
		return -1;

	/*
	 * if a command was provided, try to parse it
	 * otherwise, fall through to help
	 */
	if ((argc-cmd_index) > 0)
		cmd = get_command(argv[cmd_index]);

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
		return -2;
	}

	/* handle <sport> <target week(s)> <algorithms> */
	if (rc->action == ACTION_ANALYZE ||
	    rc->action == ACTION_PREDICT ||
	    rc->action == ACTION_RANK) {
		/*
		 * calculate new argc from cmd_index to the
		 * end of the argument vector
		 * shift the argument vector so that argv[0]
		 * is the first argument after the command from
		 * the command line
		 */
		int new_argc = argc - cmd_index - 1;
		char **new_argv = argv + (cmd_index + 1);

		/*
		 * parse the special sequence of arguments
		 * for these commands
		 */
		err = parse_rc_args(rc, new_argc, new_argv);
		if (err)
			return -3;
	}

	return 0;
}
