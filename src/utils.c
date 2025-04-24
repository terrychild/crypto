#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

void* allocate(void* data, size_t size) {
	void* new_data = realloc(data, size);
	if (new_data == NULL) {
		puts("Unable to allocate memory.");
		exit(EXIT_FAILURE);
	}
	return new_data;
}

int arg(int argc, char* argv[], const char* name) {
	size_t len = strlen(name);
	for (int i=0; i<argc; i++) {
		if (strncmp(argv[i], name, len)==0) {
			return i;
		}
	}
	return -1;
}

static char rhex_char(char nibble) {
	char value;
	if (nibble >= 'a') {
		value = nibble - 'a' + 10;
	} else if (nibble >= 'A') {
		value = nibble - 'A' + 10;
	} else {
		value = nibble - '0';
	}
	if (value > 15) {
		printf("Invalid hex char '%c'.\n", nibble);
		return 0;
	}
	return value;
}

void rhex8(uint8_t* dest, char* source, size_t len) {
	for (size_t i=0; i<len; i++) {
		dest[i] = (rhex_char(source[i*2]) << 4) + rhex_char(source[i*2 + 1]);
	}
}

void rhex64(uint64_t* dest, char* source, size_t len) {
	size_t desti = 0;
	size_t mod = ((len % 8) - 1) % 8;
	printf("len: %ld, break on: %ld\n", len, mod);
	uint64_t value=0;
	for (size_t i=0; i<len; i++) {
		value <<= 8;
		value += (rhex_char(source[i*2]) << 4) + rhex_char(source[i*2 + 1]);
		printf("i: %ld, i%8: %ld, dest: %ld, value: %016lx\n", i, i%8, desti, value);
		if (i % 8 == mod) {
			dest[desti++] = value;
			value = 0;
		}
	}
}

void rhex64_le(uint64_t* dest, char* source, size_t len) {
	size_t desti = len/8;
	size_t mod = ((len % 8) - 1) % 8;
	printf("len: %ld, break on: %ld\n", len, mod);
	uint64_t value=0;
	for (size_t i=0; i<len; i++) {
		value <<= 8;
		value += (rhex_char(source[i*2]) << 4) + rhex_char(source[i*2 + 1]);
		printf("i: %ld, i%8: %ld, dest: %ld, value: %016lx\n", i, i%8, desti, value);
		if (i % 8 == mod) {
			dest[desti--] = value;
			value = 0;
		}
	}
}

bool rbit(unsigned int value, unsigned int bit) {
	return (value >> (bit)) & 1;
}
uint8_t sbit8(uint8_t value, unsigned int bit, bool set) {
	const uint8_t mask = 1 << (bit);
	if (set) {
		return value | mask;
	} else {
		return value & ~mask;
	}
}

uint8_t rotl8(uint8_t value, unsigned int count) {
	const unsigned int mask = CHAR_BIT * sizeof(value) - 1;
	count &= mask;
	return (value << count) | (value >> (-count & mask));
}