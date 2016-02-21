#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "../spreden.h"
#include "../dstruct/list.h"
#include "database.h"

#define DB_MAX_PATH            1024
#define DB_MAX_WEEKS_PER_YEAR    52

/*
 * scan_state holds all the info needed to scan
 * for weeks in a year
 *
 * db_scan() fills it out for each year and passes
 * it to scan_year()
 */
struct scan_state {
	struct state *state;
	int year;
	int begin_week;
	int end_week;
	/* last_week - last scanned week */
	int last_week;
	/* pathbuf - buffer to carry the current dir/filename around */
	char pathbuf[DB_MAX_PATH];
	/* filenames - list of sorted game files to use for year */
	char *filenames[DB_MAX_WEEKS_PER_YEAR];
};


static int parse_week_num(const char *filename)
{
	const char *week_num_str;
	char *endptr;
	int week_num = -1;

	/* search in filename for weekXX\..* */
	if (strncmp("week", filename, 4) == 0) {
		week_num_str = filename + 4;
		week_num = (int)strtol(week_num_str, &endptr, 10);
		if (*endptr != '.')
			week_num = -1;
	}

	return week_num;
}

static int find_earliest_year(const char *path)
{
	DIR *dir;
	struct dirent ent;
	struct dirent *result;
	char *endptr;
	int earliest = INT_MAX, year;

	/* open the year directory for reading */
	dir = opendir(path);
	if (!dir) {
		fprintf(stderr, "%s: error reading dir '%s': %s\n",
			progname, path, strerror(errno));
		return -1;
	}

	/* check each entry in the dir */
	while (readdir_r(dir, &ent, &result) == 0) {
		if (!result)
			break;

		if (result->d_type == DT_DIR) {
			/* convert file name to int */
			year = (int)strtol(result->d_name, &endptr, 10);
			/* if this year is earliest, update it */
			if (*endptr == '\0' && year < earliest)
				earliest = year;
		}
	}

	closedir(dir);
	return earliest;
}

static int scan_week(struct scan_state *ss, const char *filename)
{
	int week_num;
	int index;

	/* if the filename is weekXX.*, parse out week num */
	week_num = parse_week_num(filename);
	if (week_num < 0)
		return 1;

	/*
	 * if this week is in the range,
	 * add it to the filenames table
	 */
	if (week_num >= ss->begin_week && week_num <= ss->end_week) {
		/* build full path to file */
		snprintf(ss->pathbuf, DB_MAX_PATH, "%s/%s/%d/%s",
			 ss->state->rc.data_dir,
			 ss->state->rc.sport,
			 ss->year,
			 filename);
		ss->pathbuf[DB_MAX_PATH-1] = '\0';

		/* add to table */
		index = week_num - ss->begin_week;
		ss->filenames[index] = strdup(ss->pathbuf);

		/* keep track of last week */
		if (week_num > ss->last_week)
			ss->last_week = week_num;
	}

	return 0;
}

static int scan_year(struct scan_state *ss)
{
	DIR *root;
	struct dirent ent;
	struct dirent *result;
	int scan_result;
	int num_weeks = 0;
	int i;

	assert(ss->begin_week != WEEK_ID_BEGIN);

	/* create path to year directory in db */
	snprintf(ss->pathbuf, DB_MAX_PATH, "%s/%s/%d",
		 ss->state->rc.data_dir, ss->state->rc.sport, ss->year);
	ss->pathbuf[DB_MAX_PATH-1] = '\0';

	/* open the year directory for reading */
	root = opendir(ss->pathbuf);
	if (!root) {
		fprintf(stderr, "%s: error reading dir '%s': %s\n",
			progname, ss->pathbuf, strerror(errno));
		return -1;
	}

	/* check each entry in the dir */
	while (readdir_r(root, &ent, &result) == 0) {
		if (!result)
			break;

		/* detect if valid week and add to filenames */
		scan_result = scan_week(ss, result->d_name);
		if (scan_result < 0) {
			closedir(root);
			return -2;
		} else if (scan_result == 0) {
			num_weeks++;
		}
	}

	closedir(root);

	/* update the end value since we didn't know it coming in */
	if (ss->end_week == WEEK_ID_END)
		ss->end_week = ss->last_week;

	/* make sure that there are no holes in the data */
	if (num_weeks != (ss->end_week - ss->begin_week + 1)) {
		fprintf(stderr, "%s: missing data in '%s/%s/%d'; expected %d weeks: %d-%d\n",
			progname,
			ss->state->rc.data_dir,
			ss->state->rc.sport,
			ss->year,
			ss->end_week - ss->begin_week + 1,
			ss->begin_week,
			ss->end_week);
		return -1;
	}

	/* finally, add the files to list */
	for (i = 0; i < num_weeks; i++)
		list_add_back(&ss->state->db->game_files, ss->filenames[i]);

	return 0;
}

int db_scan(struct state *s)
{
	struct scan_state ss;

	/* init scan_state */
	ss.state = s;
	ss.year = s->rc.data_begin.year;
	ss.begin_week = WEEK_ID_BEGIN;
	ss.end_week = WEEK_ID_END;
	ss.last_week = INT_MIN;
	memset(ss.filenames, 0, DB_MAX_WEEKS_PER_YEAR);

	/* build path to sport in database file layout */
	snprintf(ss.pathbuf, DB_MAX_PATH, "%s/%s", s->rc.data_dir, s->rc.sport);
	ss.pathbuf[DB_MAX_PATH-1] = '\0';

	/* make sure the expected dir exists */
	if (access(ss.pathbuf, R_OK)) {
		fprintf(stderr, "%s: error reading dir '%s': %s\n",
			progname, ss.pathbuf, strerror(errno));
		return -1;
	}

	/* if begin year is unset, find the earliest */
	if (s->rc.data_begin.year == WEEK_ID_BEGIN) {
		ss.year = find_earliest_year(ss.pathbuf);
		if (ss.year == INT_MAX)
			return -2;
	}

	/* for each year given in the rc,
	 * build scan_state and call scan_year()
	 */
	for (; ss.year <= s->rc.data_end.year; ss.year++) {
		/* set begin week */
		if (ss.year == s->rc.data_begin.year)
			ss.begin_week = s->rc.data_begin.week;
		else
			ss.begin_week = 1;

		/* set end week */
		if (ss.year == s->rc.data_end.year)
			ss.end_week = s->rc.data_end.week;
		else
			ss.end_week = WEEK_ID_END;

		ss.last_week = INT_MIN;

		/* scan files for year */
		if (scan_year(&ss) < 0)
			return -3;
	}

	/* print scanned files */
	if (verbose) {
		struct list_iter iter;
		list_iter_begin(&s->db->game_files, &iter);
		while (!list_iter_end(&iter)) {
			fprintf(stderr, "db: %s\n",
				(char *)list_iter_data(&iter));
			list_iter_next(&iter);
		}
		fprintf(stderr, "db: %d game files\n",
			s->db->game_files.length);
	}

	return 0;
}
