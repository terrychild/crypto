#ifndef BIGINT_H
#define BIGINT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	size_t size;
	size_t len;
	uint64_t* limbs;
	bool neg;
} BigInt;

BigInt* bi_new();
void bi_free(BigInt* bi);
void bi_normalise(BigInt* bi);

BigInt* bi_clone(BigInt* source);

BigInt* bi_set_int(BigInt* bi, int64_t i);
BigInt* bi_from_int(int64_t i);

BigInt* bi_set_hex(BigInt* bi, char* hex);
BigInt* bi_from_hex(char* hex);
size_t bi_to_hex(char* out, BigInt* bi, size_t maxlen);

BigInt* bi_set_char(BigInt* bi, char* data);
BigInt* bi_from_char(char* data);
size_t bi_to_char(char* out, BigInt* bi, size_t maxlen);

int bi_cmp(BigInt* a, BigInt* b);
int bi_cmp_abs(BigInt* a, BigInt* b);

BigInt* bi_shift_left(BigInt* dest, BigInt* source, size_t count);
BigInt* bi_shift_right(BigInt* dest, BigInt* source, size_t count);

BigInt* bi_add(BigInt* dest, BigInt* a, BigInt* b);
BigInt* bi_sub(BigInt* dest, BigInt* a, BigInt* b);
BigInt* bi_mul(BigInt* dest, BigInt* a, BigInt* b);
BigInt* bi_div(BigInt* dest, BigInt* a, BigInt* b, BigInt* r);
BigInt* bi_pow(BigInt* dest, BigInt* a, BigInt* b);
BigInt* bi_pow_mod(BigInt* dest, BigInt* a, BigInt* b, BigInt* mod);

void bi_debug(BigInt* bi);

#endif