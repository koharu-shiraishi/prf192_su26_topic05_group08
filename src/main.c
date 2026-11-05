#include "component_app.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	const char *csv_path = "data/components.csv";
	enum component_app_status status;

	if (argc > 2) {
		fprintf(stderr, "Usage: %s [path-to-components.csv]\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc == 2)
		csv_path = argv[1];

	status = component_app_run(csv_path);
	if (status == COMPONENT_APP_OK)
		return EXIT_SUCCESS;
	if (status == COMPONENT_APP_UNSAVED_ON_EOF)
		return 2;
	if (status == COMPONENT_APP_OUT_OF_MEMORY)
		fprintf(stderr, "Could not allocate the component manager.\n");
	else if (status == COMPONENT_APP_INVALID_ARGUMENT)
		fprintf(stderr, "Invalid argument for the component module.\n");
	return EXIT_FAILURE;
}
