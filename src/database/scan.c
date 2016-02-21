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

static int scan_weeks(struct state *s, const char *dirname,
		      int rc_begin, int rc_end)
{
	DIR *root;
	struct dirent ent;
	struct dirent *result;
	int week_num;
	int count = 0;
	int last = INT_MIN;
	int i;
	char filename[DB_MAX_PATH];
	char *filenames[DB_MAX_WEEKS_PER_YEAR];

	assert(rc_begin != WEEK_ID_BEGIN);

	/* open the year directory for reading */
	root = opendir(dirname);
	if (!root) {
		fprintf(stderr, "%s: error reading dir '%s': %s\n",
			progname, dirname, strerror(errno));
		return -1;
	}

	/* check each entry in the dir */
	while (readdir_r(root, &ent, &result) == 0) {
		if (!result)
			break;

		/* if the filename is weekXX.*, parse out week num */
		week_num = parse_week_num(result->d_name);
		if (week_num < 0)
			continue;

		/*
		 * if this week is in the range,
		 * add it to the filenames table
		 */
		if (week_num >= rc_begin && week_num <= rc_end) {
			/* create full path to file */
			snprintf(filename, DB_MAX_PATH, "%s/%s",
				 dirname, result->d_name);
			filename[DB_MAX_PATH-1] = '\0';

			/* add to table */
			filenames[week_num-rc_begin] = strdup(filename);

			/* keep track of last week */
			if (week_num > last)
				last = week_num;
			count++;
		}
	}

	closedir(root);

	/* update the end value since we didn't know it coming in */
	if (rc_end == WEEK_ID_END)
		rc_end = last;

	/* make sure that there are no holes in the data */
	if (count != (rc_end-rc_begin+1)) {
		fprintf(stderr, "%s: missing data in '%s'; expected %d weeks: %d-%d\n",
			progname, dirname, rc_end-rc_begin+1, rc_begin, rc_end);
		return -2;
	}

	/* finally, add the files to list */
	for (i = 0; i < count; i++)
		list_add_back(&s->db->game_files, filenames[i]);

	return 0;
}

static int scan_year(struct state *s, char *path,
		     int year, int rc_begin, int rc_end)
{
	if (rc_begin == WEEK_ID_BEGIN)
		rc_begin = 1;

	/* create path to year directory in db */
	snprintf(path, DB_MAX_PATH, "%s/%s/%d",
		 s->rc.data_dir, s->rc.sport, year);
	path[DB_MAX_PATH-1] = '\0';

	/* find the first and last weeks available */
	if (scan_weeks(s, path, rc_begin, rc_end) < 0)
		return -1;

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
			fprintf(stderr, "db: %s\n",
				(char *)list_iter_data(&iter));
			list_iter_next(&iter);
		}
		fprintf(stderr, "db: %d game files\n",
			s->db->game_files.length);
	}

	return 0;
}
