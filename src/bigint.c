#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "bigint.h"

BigInt* bi_new(size_t size) {
	BigInt* bi = allocate(NULL, sizeof(*bi));
	bi->size = size;
	bi->length = 0;
	bi->data = allocate(NULL, sizeof(bi->data) * bi->size);
	return bi;
}

void bi_free(BigInt* bi) {
	if (bi != NULL) {
		free(bi->data);
		free(bi);
	}
}

void bi_dump(BigInt* bi) {
	printf("BigInt %ld\n", bi->size);
	uint64_t* ptr = bi->data + bi->size;
	while (ptr != bi->data) {
		printf(" %016lx\n", *(--ptr));
	}
	printf("\n");
}

static void ensure(BigInt* bi, size_t len) {
	size_t new_size = len; // / 64;
	if (new_size > bi->size) {
		bi->size = new_size;
		bi->data = allocate(bi->data, sizeof(bi->data) * new_size);
	}
}

void bi_from_hex(BigInt* bi, char* str) {
	size_t len = strlen(str);
	printf("len: %ld, ints: %ld, bytes: %ld\n", len, (len+15)/16, len/2);
	ensure(bi, (len+15)/16);
	rhex_le(bi->data, str, len/2);
}