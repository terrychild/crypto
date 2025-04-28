#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "bigint.h"

uint64_t overflow_mask = 0xff00000000000000;

BigInt* bi_new(size_t size) {
	BigInt* bi = allocate(NULL, sizeof(*bi));
	bi->size = size;
	bi->limbs = allocate(NULL, sizeof(bi->limbs) * bi->size);
	return bi;
}

BigInt* bi_from_int(uint64_t i) {
	BigInt* bi = bi_new(1);
	bi->limbs[0] = i;
	return bi;
}

BigInt* bi_from_hex(char* str) {
	size_t len = strlen(str);
	size_t bi_len = (len+13)/14;
	size_t index = bi_len - 1;
	size_t mod = (len - 1) % 14;

	printf("len: %ld, bi_len: %ld, mod:%ld\n", len, bi_len, mod);

	BigInt* bi = bi_new(bi_len);
	uint64_t value=0;
	for (size_t i=0; i<len; i++) {
		value <<= 4;
		value += read_hex_char(str[i]);
		if (i % 14 == mod) {
			bi->limbs[index--] = value;
			value = 0;
		}
	}

	return bi;
}

void bi_normalise(BigInt* bi) {
	uint64_t carry = 0;
	for (size_t i=0; i<bi->size; i++) {
		if (carry) {
			bi->limbs[i] += carry;
		}
		carry = (bi->limbs[i] & overflow_mask) >> 56;
		if (carry) {
			bi->limbs[i] &= ~overflow_mask;
		}
	}
	if (carry) {
		bi->size++;
		bi->limbs = allocate(bi->limbs, sizeof(bi->limbs) * bi->size);
		bi->limbs[bi->size - 1] = carry;
	}
}

void bi_free(BigInt* bi) {
	if (bi != NULL) {
		free(bi->limbs);
		free(bi);
	}
}

void bi_dump(BigInt* bi) {
	printf("BigInt %ld\n", bi->size);
	uint64_t* ptr = bi->limbs + bi->size;
	while (ptr != bi->limbs) {
		ptr--;
		printf(" %02lx %014lx  %08lb %056lb\n", (*ptr & overflow_mask) >> 56, *ptr & ~overflow_mask, (*ptr & overflow_mask) >> 56, *ptr & ~overflow_mask);
	}
	printf("\n");
}

/*static void ensure(BigInt* bi, size_t len) {
	size_t new_size = len; // / 64;
	if (new_size > bi->size) {
		bi->size = new_size;
		bi->limbs = allocate(bi->limbs, sizeof(bi->limbs) * new_size);
	}
}*/

BigInt* bi_add(BigInt* dest, BigInt* a, BigInt* b) {	
	size_t max = a->size;
	if (b->size > max) {
		max = b->size;
	}
	if (dest->size < max) {
		dest->limbs = allocate(dest->limbs, sizeof(dest->limbs) * max);
	}

	for (size_t i=0; i < max; i++) {
		if (i < a->size) {
			if (i < b->size) {
				dest->limbs[i] = a->limbs[i] + b->limbs[i];
			} else {
				dest->limbs[i] = a->limbs[i];
			}
		} else {
			dest->limbs[i] = b->limbs[i];
		}
	}
	return dest;
}