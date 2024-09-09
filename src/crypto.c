#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 128

char* encode(char* buffer, size_t len) {
	for(size_t i = 0; i<len; i++) {
		buffer[i] = buffer[i] ^ 0x20;
	}

	return buffer;
}

char to_hex_char(char nibble) {
	if (nibble < 10) {
		return nibble + 48;
	} else {
		return (nibble-10) + 97;
	}
}

char* to_hex(char* bytes, size_t len) {
	char* hex = malloc(len*2 + 1);

	for (size_t i=0; i<len; i++) {
		hex[i*2] = to_hex_char((bytes[i] & 0xF0) >> 4);
		hex[i*2 + 1] = to_hex_char(bytes[i] & 0x0F);
	}
	hex[len*2] = '\0';

	return hex;
}

char from_hex_char(char nibble) {
	char value;
	if (nibble >= 'a') {
		value = nibble - 'a' + 10;
	} else if (nibble >= 'A') {
		value = nibble - 'A' + 10;
	} else {
		value = nibble - '0';
	}
	if (value < 0 || value > 15) {
		printf("invalid hex char '%c'\n", nibble);
		return 0;
	}
	return value;
}

char* from_hex(char* hex, size_t len) {
	char* bytes = malloc(len);

	for (size_t i=0; i<len; i++) {
		bytes[i] = (from_hex_char(hex[i*2]) << 4) + from_hex_char(hex[i*2 + 1]);
	}

	return bytes;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		puts("expected 2 parametters");
		return EXIT_FAILURE;
	}

	// validate action
	if (argv[1][0]!='e' && argv[1][0]!='d') {
		printf("unknown action \"%s\"\n", argv[1]);
		return EXIT_FAILURE;
	}

	// validate key
	size_t key_len = strlen(argv[2])/2;
	if (key_len!=16 && key_len!=24 && key_len!=32) {
		puts("invalid key length");
		return EXIT_FAILURE;
	}
	char* key = from_hex(argv[2], key_len);

	// do input things
	char* input;
	size_t len;
	if (argc == 4) {
		len = strlen(argv[3])/2;
		input = from_hex(argv[3], len);
	} else {
		input = malloc(BLOCK_SIZE);
		len = fread(input, 1, BLOCK_SIZE, stdin);
	}

	puts(input);

	// open files
	/*
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
	fclose(output_file);*/

	return EXIT_SUCCESS;
}