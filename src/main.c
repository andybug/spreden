#include <stdlib.h>
#include <stdio.h>

#include "spreden.h"

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	printf("spreden %d.%d\n", SPREDEN_VERSION_MAJOR, SPREDEN_VERSION_MINOR);

	return EXIT_SUCCESS;
}
