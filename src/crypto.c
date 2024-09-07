#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	if (argc != 3) {
		puts("expected 2 parametters");
		return EXIT_FAILURE;
	}

	// open files
	FILE *input_file = fopen(argv[1], "r");
	if (input_file == NULL) {
		printf("unable to open file \"%s\"\n", argv[1]);
		return EXIT_FAILURE;
	}

	FILE *output_file = fopen(argv[2], "w");
	if (output_file == NULL) {
		printf("unable to open file \"%s\"\n", argv[2]);
		return EXIT_FAILURE;
	}

	// read write
	char buffer[128];
	size_t read = fread(buffer, 1, 128, input_file);
	while (read > 0) {
		fwrite(buffer, 1, read, output_file);
		read = fread(buffer, 1, 128, input_file);
	}

	// close files
	fclose(input_file);
	fclose(output_file);

	return EXIT_SUCCESS;
}