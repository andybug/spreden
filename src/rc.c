#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include "spreden.h"

static int parse_date(const char *rc_date_, struct spreden_round *date)
{
	static const char *round_delim = "rw";
	static const char *end_delim = "\0";
	size_t len;
	char *rc_date;
	char *saveptr;
	char *endptr;
	char *year, *round;

	len = strlen(rc_date_);
	rc_date = alloca(len);
	strcpy(rc_date, rc_date_);

	year = strtok_r(rc_date, round_delim, &saveptr);
	round = strtok_r(NULL, end_delim, &saveptr);

	date->year = strtol(year, &endptr, 10);
	if (*endptr != '\0') {
		fprintf(stderr, "%s: '%s' is not a valid year\n",
			/* FIXME */ __func__, year);
		return -1;
	}

	if (round) {
		date->round = strtol(round, &endptr, 10);
		if (*endptr != '\0') {
			fprintf(stderr, "%s: '%s' is not a valid round\n",
				/* FIXME */ __func__, round);
			return -2;
		}
	} else {
		date->round = SPREDEN_ROUND_ALL;
	}

	return 0;
}

static int parse_dates(const char *rc_dates_,
			   struct spreden_round *begin,
			   struct spreden_round *end)
{
	static const char *range_delim = "-";
	size_t len;
	char *rc_dates;
	char *date1, *date2;
	char *saveptr;
	int err;

	if (!rc_dates_) {
		fprintf(stderr, "%s: no date provided in run control\n",
			/* FIXME */ __func__);
		return -1;
	}

	len = strlen(rc_dates_);
	rc_dates = alloca(len);
	strcpy(rc_dates, rc_dates_);

	date1 = strtok_r(rc_dates, range_delim, &saveptr);
	date2 = strtok_r(NULL, range_delim, &saveptr);

	err = parse_date(date1, begin);
	if (err)
		return -2;

	if (date2) {
		err = parse_date(date2, end);
		if (err)
			return -3;
	}

	return 0;
}

static void print_rc(struct spreden_state *state)
{
	const char *action = "default";

	/* heading */
	fputs("***** Run Control *****\n", stderr);

	/* print action */
	switch (state->rc.action) {
	case SPREDEN_ACTION_PREDICT:
		action = "predict";
		break;
	case SPREDEN_ACTION_RANK:
		action = "rank";
		break;
	default:
		break;
	}
	fprintf(stderr, "action: %s\n", action);

	/* print sport */
	fprintf(stderr, "sport:  %s\n", state->rc.sport);

	/* print beginning round */
	if (state->rc.data_begin.round == SPREDEN_ROUND_ALL) {
		fprintf(stderr, "begin:  %d\n",
			state->rc.data_begin.year);
	} else {
		fprintf(stderr, "begin:  %d round %d\n",
			state->rc.data_begin.year,
			state->rc.data_begin.round);
	}

	/* print ending round */
	if (state->rc.data_end.round == SPREDEN_ROUND_ALL) {
		fprintf(stderr, "end:    %d\n",
			state->rc.data_end.year);
	} else {
		fprintf(stderr, "end:    %d round %d\n",
			state->rc.data_end.year,
			state->rc.data_end.round);
	}
	//fprintf(stderr, "algos: %s\n", algos);

	/* footer */
	fputs("***********************\n", stderr);
}

int rc_parse(struct spreden_state *state, const char *rc_)
{
	static const char *delim = ":";
	size_t len;
	char *rc;
	char *sport, *dates, *algos;
	char *saveptr;
	struct spreden_round begin_date, end_date;
	int err;

	len = strlen(rc_);
	rc = alloca(len + 1);
	strcpy(rc, rc_);

	sport = strtok_r(rc, delim, &saveptr);
	dates = strtok_r(NULL, delim, &saveptr);
	algos = strtok_r(NULL, delim, &saveptr);

	err = parse_dates(dates, &begin_date, &end_date);
	if (err)
		return -1;

	/* update rc */
	state->rc.sport = strdup(sport);
	state->rc.data_begin = begin_date;
	state->rc.data_end = end_date;
	/* action_round will be set by --predict or --rank */

	if (state->verbose)
		print_rc(state);

	return 0;
}
