#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"

size_t random(uint8_t* buffer, size_t max_len) {
	FILE* fd = fopen("/dev/urandom", "r");
	if (fd == NULL) {
		puts("Unable to open /dev/urandom");
		return 0;
	}

	size_t rv = fread(buffer, 1, max_len, fd);

	fclose(fd);
	return rv;
}

static void hex_chars(char* dest, uint8_t* source, size_t len) {
	for (size_t i=0; i<len; i++) {
		dest[i*2] = get_hex_char((source[i] & 0xF0) >> 4);
		dest[(i*2)+1] = get_hex_char(source[i] & 0x0F);
	}
}

int random_cmd(int argc, char* argv[]) {
	int len_arg = arg(argc, argv, "--len=");
	if (len_arg<0) {
		puts("Missing len argument.");
		return EXIT_FAILURE;
	}

	size_t len = read_dec(argv[len_arg]+6, strlen(argv[len_arg]+6));
	uint8_t* buffer = allocate(NULL, len);
	if (random(buffer, len) != len) {
		free(buffer);
		return EXIT_FAILURE;
	}

	FILE *output_file;
	int fout_arg = arg(argc, argv, "--fout=");
	if (fout_arg<0) {
		output_file = stdout;
	} else {
		output_file = fopen(argv[fout_arg]+7, "w");
		if (output_file == NULL) {
			printf("unable to open file \"%s\"\n", argv[fout_arg]+7);
			return EXIT_FAILURE;
		}
	}

	if (arg(argc, argv, "--hex")>=0) {
		char* hex = allocate(NULL, len*2);
		hex_chars(hex, buffer, len);
		fwrite(hex, 1, len*2, output_file);
		free(hex);
	} else {
		fwrite(buffer, 1, len, output_file);
	}
	if (fout_arg>=0) {
		fclose(output_file);
	}
	free(buffer);
	return EXIT_SUCCESS;
}