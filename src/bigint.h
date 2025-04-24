#ifndef BIGINT_H
#define BIGINT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	size_t size;
	size_t length;
	uint64_t* data;
} BigInt;

BigInt* bi_new(size_t size);
void bi_free(BigInt* bi);

void bi_dump(BigInt* bi);

void bi_from_hex(BigInt* bi, char* str);

#endif