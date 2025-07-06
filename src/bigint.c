#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "bigint.h"

static uint64_t left_bit_mask = 0x8000000000000000;

static void resize(BigInt* bi, size_t new_size) {
	bi->size = new_size;
	bi->limbs = allocate(bi->limbs, sizeof(bi->limbs) * new_size);
}
static void ensure(BigInt* bi, size_t size) {
	if (bi->size < size) {
		resize(bi, size);
	} else if (bi->size > size) {
		for (size_t limb = size; limb < bi->size; limb++) {
			bi->limbs[limb] = 0;
		}
	}
}
static void clear(BigInt* bi, size_t size) {
	if (bi->size < size) {
		resize(bi, size);
	}
	for (size_t limb = 0; limb < bi->size; limb++) {
		bi->limbs[limb] = 0;
	}
}

void bi_normalise(BigInt* bi) {
	size_t new_size = bi->size;
	while (new_size>1 && bi->limbs[new_size-1]==0) {
		new_size--;
	}
	if (new_size < bi->size) {
		resize(bi, new_size);
	}
}

static size_t normal_size(BigInt* bi) {
	size_t size = bi->size;
	while (size>1 && bi->limbs[size-1]==0) {
		size--;
	}
	return size;
}

static size_t max_normal_size(BigInt* a, BigInt* b) {
	size_t as = normal_size(a);
	size_t bs = normal_size(b);
	return as >= bs ? as : bs;
}

static bool is_zero(BigInt* bi) {
	for (size_t limb=0; limb < bi->size; limb++) {
		if (bi->limbs[limb] != 0) {
			return false;
		}
	}
	return true;
}

static void make_zero(BigInt* bi) {
	bi->neg = false;
	for (size_t limb = 0; limb < bi->size; limb++) {
		bi->limbs[limb] = 0;
	}
}

static BigInt* new(size_t size) {
	BigInt* bi = allocate(NULL, sizeof(*bi));
	bi->size = size;
	bi->neg = false;
	if (size > 0) {
		bi->limbs = allocate(NULL, sizeof(bi->limbs) * bi->size);
	} else {
		bi->limbs = NULL;
	}
	return bi;
}

BigInt* bi_new() {
	BigInt* bi = new(1);
	bi->limbs[0] = 0;
	return bi;
}

void bi_free(BigInt* bi) {
	if (bi != NULL) {
		if (bi->limbs != NULL) {
			free(bi->limbs);
		}
		free(bi);
	}
}

static void copy_limbs(BigInt* dest, BigInt* source) {
	for (size_t limb = 0; limb < dest->size; limb++) {
		dest->limbs[limb] = limb < source->size ? source->limbs[limb] : 0;
	}
}
static void copy(BigInt* dest, BigInt* source) {
	if (dest->size < source->size) {
		resize(dest, source->size);
	}
	copy_limbs(dest, source);
	dest->neg = source->neg;
}
BigInt* bi_clone(BigInt* source) {
	BigInt* dest = new(source->size);
	copy_limbs(dest, source);
	dest->neg = source->neg;
	return dest;
}

BigInt* bi_set_int(BigInt* bi, int64_t i) {
	ensure(bi, 1);

	if (i>=0) {
		bi->neg = false;
		bi->limbs[0] = (uint64_t)i;
	} else {
		bi->neg = true;
		bi->limbs[0] = (uint64_t)(0-i);
	}
	
	return bi;
}
BigInt* bi_from_int(int64_t i) {
	return bi_set_int(new(1), i);
}

BigInt* bi_set_hex(BigInt* bi, char* hex) {
	size_t nibbles = strlen(hex);
	size_t len = (nibbles+15)/16;

	ensure(bi, len);

	size_t limb = 0;
	size_t bits = 0;
	uint64_t value=0;
	for (size_t i = nibbles; i-- > 0;) {
		value += ((uint64_t)read_hex_char(hex[i])) << bits;
		bits += 4;

		if (bits == 64) {
			bi->limbs[limb++] = value;
			bits = 0;
			value = 0;
		}
	}
	bi->limbs[limb] = value;

	return bi;
}
BigInt* bi_from_hex(char* hex) {
	return bi_set_hex(new(0), hex);
}
size_t bi_to_hex(char* out, BigInt* bi, size_t maxlen) {
	if (maxlen > 0 && is_zero(bi)) {
		out[0] = '0';
		return 1;
	}

	size_t chars = 0;	
	bool go = false;
	for (size_t limb = bi->size; limb-- > 0 && chars<maxlen; ) {
		uint64_t mask = 0xf000000000000000;
		size_t bits = 60;
		while (mask && chars<maxlen) {
			if (!go && (bi->limbs[limb] & mask)) {
				go = true;
			}
			if (go) {
				out[chars] = get_hex_char((bi->limbs[limb] & mask) >> bits);
				chars++;
			}
			bits -= 4;
			mask >>= 4;
		}
	}

	return chars;
}

BigInt* bi_set_char(BigInt* bi, char* data) {
	size_t chars = strlen(data);
	size_t len = (chars+7)/8;

	ensure(bi, len);

	size_t limb = 0;
	size_t bits = 0;
	uint64_t value=0;
	for (size_t i = chars; i-- > 0; ) {
		value += ((uint64_t)data[i]) << bits;
		bits += 8;

		if (bits == 64) {
			bi->limbs[limb++] = value;
			bits = 0;
			value = 0;
		}
	}
	bi->limbs[limb] = value;

	return bi;
}
BigInt* bi_from_char(char* data) {
	return bi_set_char(new(0), data);
}
size_t bi_to_char(char* out, BigInt* bi, size_t maxlen) {
	size_t chars = 0;	
	bool go = false;
	for (size_t limb = bi->size; limb-- > 0 && chars<maxlen; ) {
		uint64_t mask = 0xff00000000000000;
		size_t bits = 56;
		while (mask && chars<maxlen) {
			if (!go && (bi->limbs[limb] & mask)) {
				go = true;
			}
			if (go) {
				out[chars] = (bi->limbs[limb] & mask) >> bits;
				chars++;
			}
			bits -= 8;
			mask >>= 8;
		}
	}

	return chars;
}

int bi_cmp(BigInt* a, BigInt* b) {
	if (a->neg == b->neg) {
		if (a->neg) {
			return bi_cmp_abs(b, a);
		}
		return bi_cmp_abs(a, b);
	} else if (a->neg) {
		return -1;
	}
	return 1;
}

int bi_cmp_abs(BigInt* a, BigInt* b) {
	size_t limb = a->size;
	if (b->size > limb) {
		limb = b->size;
	}

	while(limb-- > 0) {
		uint64_t ai = limb < a->size ? a->limbs[limb] : 0;
		uint64_t bi = limb < b->size ? b->limbs[limb] : 0;
		if (ai < bi) {
			return -1;
		} else if (ai > bi) {
			return 1;
		}
	}
	return 0;
}

BigInt* bi_shift_left(BigInt* dest, BigInt* source) {
	if (is_zero(source)) {
		make_zero(dest);
		return dest;
	}

	size_t ssize = normal_size(source);
	size_t tsize = ssize;
	if (source->limbs[ssize-1] & left_bit_mask) {
		tsize++;
	}
	if (dest->size < tsize) {
		resize(dest, tsize);
	}
	for (size_t limb=dest->size; limb-- > 0; ) {
		bool carry = limb>0 && limb<=ssize ? source->limbs[limb-1] & left_bit_mask : false;
		dest->limbs[limb] = (limb < ssize ? (source->limbs[limb] << 1) : 0) + (carry ? 1 : 0);
	}

	dest->neg = source->neg;

	return dest;
}

BigInt* bi_shift_right(BigInt* dest, BigInt* source) {
	if (is_zero(source)) {
		make_zero(dest);
		return dest;
	}

	size_t ssize = normal_size(source);
	size_t tsize = ssize;
	if (ssize > 1 && source->limbs[ssize-1] == 1) {
		tsize--;
	}
	if (dest->size < tsize) {
		resize(dest, tsize);
	}
	for (size_t limb=0; limb < dest->size; limb++) {
		bool carry = limb<ssize-1 ? source->limbs[limb+1] & 0x1 : false;
		dest->limbs[limb] = (limb < ssize ? (source->limbs[limb] >> 1) : 0) + (carry ? left_bit_mask : 0);
	}

	dest->neg = source->neg;
	
	return dest;
}

static void add(BigInt* dest, BigInt* a, BigInt* b) {
	size_t size = max_normal_size(a, b);
	ensure(dest, size);

	uint64_t carry = 0;
	for (size_t i=0; i < size; i++) {
		uint64_t ai = 0;
		uint64_t bi = 0;
		if (i < a->size) {
			ai = a->limbs[i];
		}
		if (i < b->size) {
			bi = b->limbs[i];
		}
		dest->limbs[i] = ai + bi + carry;
		if (carry) {
			carry = dest->limbs[i] <= ai ? 1 : 0;
		} else {
			carry = dest->limbs[i] < ai ? 1 : 0;
		}
	}

	if (carry) {
		if (dest->size < size+1) {
			resize(dest, size+1);
		}
		dest->limbs[size] = 1;
	}
}

static void sub(BigInt* dest, BigInt* a, BigInt* b) {
	size_t size = max_normal_size(a, b);
	ensure(dest, size);

	uint64_t borrow = 0;
	for (size_t i=0; i < size; i++) {
		uint64_t ai = 0;
		uint64_t bi = 0;
		if (i < a->size) {
			ai = a->limbs[i];
		}
		if (i < b->size) {
			bi = b->limbs[i];
		}
		dest->limbs[i] = ai - bi - borrow;
		if (borrow) {
			borrow = bi >= ai ? 1 : 0;
		} else {
			borrow = bi > ai ? 1 : 0;
		}
	}

	if (borrow) {
		puts("PANIC: borrow left over!");
	}
}

BigInt* bi_add(BigInt* dest, BigInt* a, BigInt* b) {
	if (a->neg == b->neg) {
		add(dest, a, b);
		dest->neg = a->neg;
	} else {
		if (bi_cmp_abs(a, b) >= 0) {
			sub(dest, a, b);
			dest->neg = a->neg;
		} else {
			sub(dest, b, a);
			dest->neg = b->neg;
		}
	}
	if (is_zero(dest)) {
		dest->neg = false;
	}
	return dest;
}

BigInt* bi_sub(BigInt* dest, BigInt* a, BigInt* b) {
	if (a->neg != b->neg) {
		add(dest, a, b);
		dest->neg = a->neg;
	} else {
		if (bi_cmp_abs(a, b) >= 0) {
			sub(dest, a, b);
			dest->neg = a->neg;
		} else {
			sub(dest, b, a);
			dest->neg = !b->neg;
		}
	}
	if (is_zero(dest)) {
		dest->neg = false;
	}
	return dest;
}

BigInt* bi_mul(BigInt* dest, BigInt* a, BigInt* b) {
	if (bi_cmp_abs(a, b) < 0) {
		return bi_mul(dest, b, a);
	}

	BigInt* temp_a = bi_clone(a);
	BigInt* temp_b = bi_clone(b);

	clear(dest, normal_size(a) * 2);
	dest->neg = a->neg != b->neg;

	size_t limb = 0;
	uint64_t mask = 1;
	while (limb < temp_b->size) {
		if (temp_b->limbs[limb] & mask) {
			add(dest, dest, temp_a);
		}
		bi_shift_left(temp_a, temp_a);
		mask <<= 1;
		if (!mask) {
			limb++;
			mask = 1;
		}
	}

	bi_free(temp_a);
	bi_free(temp_b);

	return dest;
}

BigInt* bi_div(BigInt* dest, BigInt* a, BigInt* b, BigInt* r) {
	if (is_zero(b)) {
		puts("PANIC: Divide by zero!");
		return dest;
	}

	copy(r, a);
	BigInt* orig_b = bi_clone(b);
	BigInt* temp_b = bi_clone(b);
	BigInt* mask = bi_from_int(1);

	bi_set_int(dest, 0);
	dest->neg = a->neg != b->neg;

	while (bi_cmp_abs(temp_b, r) < 0) {
		bi_shift_left(temp_b, temp_b);
		bi_shift_left(mask, mask);
	}
	
	while (bi_cmp_abs(r, orig_b) >= 0) {
		if (bi_cmp_abs(r, temp_b) >= 0) {
			add(dest, dest, mask);
			sub(r, r, temp_b);
		}
		bi_shift_right(temp_b, temp_b);
		bi_shift_right(mask, mask);
	}

	bi_free(orig_b);
	bi_free(temp_b);
	bi_free(mask);

	return dest;
}

BigInt* bi_pow(BigInt* dest, BigInt* a, BigInt* b) {
	BigInt* temp_a = bi_clone(a);
	BigInt* temp_b = bi_clone(b);

	copy(dest, a);

	bool go = false;
	size_t limb = temp_b->size;
	do {
		limb--;
		uint64_t mask = left_bit_mask;
		while (mask) {
			if (!go) {
				if (temp_b->limbs[limb] & mask) {
					go = true;
				}
			} else {
				bi_mul(dest, dest, dest);
				if (temp_b->limbs[limb] & mask) {
					bi_mul(dest, dest, temp_a);
				}
			}
			mask >>= 1;
		}

	} while (limb>0);

	bi_free(temp_a);
	bi_free(temp_b);

	return dest;
}

BigInt* bi_pow_mod(BigInt* dest, BigInt* a, BigInt* b, BigInt* mod) {
	BigInt* temp = bi_new();
	BigInt* temp_a = bi_clone(a);
	BigInt* temp_b = bi_clone(b);

	copy(dest, a);

	bool go = false;
	size_t limb = temp_b->size;
	do {
		limb--;
		uint64_t mask = left_bit_mask;
		while (mask) {
			if (!go) {
				if (temp_b->limbs[limb] & mask) {
					go = true;
				}
			} else {
				bi_mul(dest, dest, dest);
				bi_div(temp, dest, mod, dest);
				if (temp_b->limbs[limb] & mask) {
					bi_mul(dest, dest, temp_a);
					bi_div(temp, dest, mod, dest);
				}
			}
			mask >>= 1;
		}

	} while (limb>0);

	bi_free(temp);
	bi_free(temp_a);
	bi_free(temp_b);

	return dest;
}

void bi_debug(BigInt* bi) {
	size_t limb = bi->size - 1;
	printf("%c %016lx %064lb\n", bi->neg ? '-' : '+', bi->limbs[limb], bi->limbs[limb]);
	while (limb-- > 0) {
		printf("  %016lx %064lb\n", bi->limbs[limb], bi->limbs[limb]);
	}
}