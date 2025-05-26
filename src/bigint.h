#ifndef BIGINT_H
#define BIGINT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	size_t size;
	uint64_t* limbs;
	bool neg;
} BigInt;

BigInt* bi_new(size_t size);
BigInt* bi_from_int(int64_t i);
BigInt* bi_from_hex(char* str);
BigInt* bi_clone(BigInt* source);

void bi_free(BigInt* bi);

void bi_dump(char* str, BigInt* bi);
void bi_debug(BigInt* bi);

int bi_cmp(BigInt* a, BigInt* b);
int bi_cmp_abs(BigInt* a, BigInt* b);

BigInt* bi_shift_left(BigInt* dest, BigInt* source, size_t count);
BigInt* bi_shift_right(BigInt* dest, BigInt* source, size_t count);

BigInt* bi_add(BigInt* dest, BigInt* a, BigInt* b);
BigInt* bi_sub(BigInt* dest, BigInt* a, BigInt* b);
BigInt* bi_mul(BigInt* dest, BigInt* a, BigInt* b);

#endif