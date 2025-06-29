#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

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

char get_hex_char(char value) {
	if (value < 0 || value > 15) {
		printf("Invalid hex value %d.\n", value);
		return 'X';
	}
	if (value < 10) {
		return '0' + value;
	} else {
		return 'a' + value - 10;
	}
}

char read_hex_char(char nibble) {
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

static void read_hex(uint8_t* dest, size_t dest_index, int dest_delta, char* source, size_t len) {
	size_t mod = (len - 1) % 2;
	uint8_t value=0;
	for (size_t i=0; i<len; i++) {
		value <<= 4;
		value += read_hex_char(source[i]);
		if (i % 2 == mod) {
			dest[dest_index] = value;
			dest_index += dest_delta;
			value = 0;
		}
	}
}

void read_hex_be(uint8_t* dest, char* source, size_t len) {
	read_hex(dest, 0, 1, source, len);
}
void read_hex_le(uint8_t* dest, char* source, size_t len) {
	read_hex(dest, ((len+1)/2)-1, -1, source, len);
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

ssize_t random(uint8_t* buffer, size_t max_len) {
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1) {
		puts("Unable to open /dev/urandom");
		return -1;
	}

	ssize_t rv = read(fd, buffer, max_len);

	close(fd);
	return rv;
}