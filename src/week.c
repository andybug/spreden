#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#include "spreden.h"

int week_parse(const char *date, struct week_id *out)
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

int week_parse_range(const char *range,
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
