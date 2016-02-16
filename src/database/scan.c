#include <stdio.h>

#include <sys/types.h>
#include <dirent.h>

#include "../spreden.h"
#include "database.h"

int db_scan(struct state *s)
{
	DIR *root;
	struct dirent ent;
	struct dirent *result;

	root = opendir(s->rc.data_dir);
	if (!root) {
		perror(progname);
		return -1;
	}

	while (readdir_r(root, &ent, &result) == 0) {
		if (!result)
			break;

		if (result->d_type == DT_DIR) {
			if (strcmp(s->rc.sport, result->d_name) == 0)
				printf("found data for sport %s\n", s->rc.sport);
		}
	}

	closedir(root);
	return 0;
}
