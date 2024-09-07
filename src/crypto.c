#include <stdlib.h>
#include <stdio.h>

#define BLOCK_SIZE 128

char* encode(char* buffer, size_t len) {
	for(size_t i = 0; i<len; i++) {
		buffer[i] = buffer[i] ^ 0x20;
	}

	return buffer;
}

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
	char buffer[BLOCK_SIZE];
	size_t read;
	while ((read = fread(buffer, 1, BLOCK_SIZE, input_file)) > 0) {
		fwrite(encode(buffer, read), 1, read, output_file);
	}

	// close files
	fclose(input_file);
	fclose(output_file);

	return EXIT_SUCCESS;
}