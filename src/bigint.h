#ifndef BIGINT_H
#define BIGINT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	size_t size;
	uint64_t* limbs;
} BigInt;

BigInt* bi_new(size_t size);
BigInt* bi_from_int(uint64_t i);
BigInt* bi_from_hex(char* str);

void bi_normalise(BigInt* bi);

void bi_free(BigInt* bi);

void bi_dump(BigInt* bi);

BigInt* bi_add(BigInt* dest, BigInt* a, BigInt* b);

#endif