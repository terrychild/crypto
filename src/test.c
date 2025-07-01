#include <stdlib.h>
#include <stdio.h>
#include "rand.h"

static void dump_hex(uint8_t* buffer, size_t len) {
	for (size_t i=0; i<len; i++) {
		printf("%02x", buffer[i]);
	}
	printf("\n");
}

int test() {
	uint8_t buffer[8];
	size_t read = random(buffer, 8);
	dump_hex(buffer, read);
	return EXIT_SUCCESS;
}