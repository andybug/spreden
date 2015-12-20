#ifndef SPREDEN_H
#define SPREDEN_H

#include <stdbool.h>
#include "list.h"

#define SPREDEN_VERSION_MAJOR 0
#define SPREDEN_VERSION_MINOR 1

#define SPREDEN_DEFAULT_SCRIPT_DIR "/usr/share/spreden/scripts"
#define SPREDEN_DEFAULT_DATA_DIR   "/usr/share/spreden/data"

struct spreden_state {
	bool verbose;
	struct list script_dirs;
	struct list data_dirs;
};

#endif
