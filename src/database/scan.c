#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "../spreden.h"
#include "../dstruct/list.h"
#include "database.h"

#define DB_MAX_PATH 1024


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

static int scan_weeks(const char *path, int *firstw, int *lastw)
{
	DIR *root;
	struct dirent ent;
	struct dirent *result;
	int first = INT_MAX, last = INT_MIN, count = 0;
	int num;

	/* open the year directory for reading */
	root = opendir(path);
	if (!root) {
		fprintf(stderr, "%s: error reading dir '%s': %s\n",
			progname, path, strerror(errno));
		return -1;
	}

	/* check each entry in the dir */
	while (readdir_r(root, &ent, &result) == 0) {
		if (!result)
			break;

		/* if the filename is weekXX.*, parse out week num */
		num = parse_week_num(result->d_name);
		if (num < 0)
			continue;

		if (num < first)
			first = num;

		if (num > last)
			last = num;

		count++;
	}

	closedir(root);

	/* make sure that there are no holes in the data */
	if (count != (last-first+1)) {
		fprintf(stderr, "%s: data hole at '%s'\n", progname, path);
		return -2;
	}

	/* set outputs */
	*firstw = first;
	*lastw = last;

	return 0;
}

static void add_to_game_files(struct state *s, const char *base,
			     int begin, int end)
{
	char path[DB_MAX_PATH];
	int i;

	for (i = begin; i <= end; i++) {
		snprintf(path, DB_MAX_PATH, "%s/week%02d.json", base, i);
		list_add_back(&s->db->game_files, strdup(path));
	}
}

static int scan_year(struct state *s, char *path,
		     int year, int rc_begin, int rc_end)
{
	int firstw, lastw;

	if (rc_begin == WEEK_ID_BEGIN)
		rc_begin = 1;

	/* create path to year directory in db */
	snprintf(path, DB_MAX_PATH, "%s/%s/%d",
		 s->rc.data_dir, s->rc.sport, year);
	path[DB_MAX_PATH-1] = '\0';

	/* find the first and last weeks available */
	if (scan_weeks(path, &firstw, &lastw) < 0)
		return -1;

	/* validate available data against rc */
	if (rc_begin < firstw) {
		fprintf(stderr, "%s: data begin week %d is before available week %d\n",
			progname, rc_begin, firstw);
		return -2;
	} else if (rc_end != WEEK_ID_END && rc_end > lastw) {
		fprintf(stderr, "%s: data end week %d is after available week %d\n",
			progname, rc_end, lastw);
		return -3;
	}

	/* add the path to each week to the game files list */
	add_to_game_files(s, path, rc_begin,
			  rc_end == WEEK_ID_END ? lastw : rc_end);

	return 0;
}

int db_scan(struct state *s)
{
	char path[DB_MAX_PATH];
	int year, begin, end;

	snprintf(path, DB_MAX_PATH, "%s/%s", s->rc.data_dir, s->rc.sport);
	path[DB_MAX_PATH-1] = '\0';

	if (access(path, R_OK)) {
		fprintf(stderr, "%s: error reading dir '%s': %s\n",
			progname, path, strerror(errno));
		return -1;
	}

	year = s->rc.data_begin.year;
	for (; year <= s->rc.data_end.year; year++) {
		begin = 1;
		end = WEEK_ID_END;
		if (year == s->rc.data_begin.year)
			begin = s->rc.data_begin.week;
		if (year == s->rc.data_end.year)
			end = s->rc.data_end.week;

		if (scan_year(s, path, year, begin, end) < 0)
			return -2;
	}

	if (verbose) {
		struct list_iter iter;
		list_iter_begin(&s->db->game_files, &iter);
		while (!list_iter_end(&iter)) {
			fprintf(stderr, "db: %s\n", list_iter_data(&iter));
			list_iter_next(&iter);
		}
		fprintf(stderr, "db: %d game files\n",
			s->db->game_files.length);
	}

	return 0;
}
