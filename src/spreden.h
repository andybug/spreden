#ifndef SPREDEN_H
#define SPREDEN_H

#include <stdbool.h>
#include "list.h"

#define SPREDEN_VERSION_MAJOR 0
#define SPREDEN_VERSION_MINOR 1

struct spreden_state {
	bool verbose;
	struct list script_dirs;
	struct list data_dirs;
};

#endif
