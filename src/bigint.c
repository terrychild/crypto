#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "bigint.h"

static uint64_t left_bit_mask = 0x8000000000000000;

static void resize(BigInt* bi, size_t new_size, bool zero_new) {
	bi->limbs = allocate(bi->limbs, sizeof(bi->limbs) * new_size);
	if (zero_new) {
		for (size_t limb = bi->size; limb < new_size; limb++) {
			bi->limbs[limb] = 0;
		}
	}
	bi->size = new_size;
}
static void ensure(BigInt* bi, size_t size) {
	if (bi->size < size) {
		resize(bi, size, true);
	} else if (bi->size > size) {
		for (size_t limb = size; limb < bi->size; limb++) {
			bi->limbs[limb] = 0;
		}
	}
}
static void clear(BigInt* bi, size_t size) {
	if (bi->size < size) {
		resize(bi, size, false);
	}
	for (size_t limb = 0; limb < bi->size; limb++) {
		bi->limbs[limb] = 0;
	}
}

static void update_len(BigInt* bi) {
	for (size_t limb = bi->size; limb-- > 0;) {
		if (bi->limbs[limb]) {
			bi->len = (limb * 64) + 64 - __builtin_clzll(bi->limbs[limb]);
			return;
		}
	}
	bi->len = 0;
}

static size_t normal_size(size_t len) {
	size_t s = (len + 63) / 64;
	return s ? s : 1;
}

static size_t max_normal_size(BigInt* a, BigInt* b) {
	size_t as = normal_size(a->len);
	size_t bs = normal_size(b->len);
	return as >= bs ? as : bs;
}

void bi_normalise(BigInt* bi) {
	size_t new_size = normal_size(bi->len);
	if (new_size < bi->size) {
		resize(bi, new_size, false);
	}
}

static bool is_zero(BigInt* bi) {
	return bi->len == 0;;
}

static void make_zero(BigInt* bi) {
	bi->neg = false;
	bi->len = 0;
	for (size_t limb = 0; limb < bi->size; limb++) {
		bi->limbs[limb] = 0;
	}
}

static BigInt* new(size_t size) {
	BigInt* bi = allocate(NULL, sizeof(*bi));
	bi->size = size;
	bi->len = 0;
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
		resize(dest, source->size, false);
	}
	copy_limbs(dest, source);
	dest->len = source->len;
	dest->neg = source->neg;
}
BigInt* bi_clone(BigInt* source) {
	BigInt* dest = new(source->size);
	copy_limbs(dest, source);
	dest->len = source->len;
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
	update_len(bi);
	
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
	update_len(bi);
	bi->neg = false;

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
	update_len(bi);
	bi->neg = false;

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
	if (a->len < b->len) {
		return -1;
	} else if (a->len > b->len) {
		return 1;
	} else {
		size_t limb = normal_size(a->len);
		while (limb-- > 0) {
			if (a->limbs[limb] < b->limbs[limb]) {
				return -1;
			} else if (a->limbs[limb] > b->limbs[limb]) {
				return 1;
			}
		}
		return 0;
	}
}

BigInt* bi_shift_left(BigInt* dest, BigInt* source, size_t count) {
	if (is_zero(source)) {
		make_zero(dest);
		return dest;
	}

	size_t move = count / 64;
	size_t shift = count % 64;
	size_t size = normal_size(source->len + count);
		
	ensure(dest, size);

	size_t limb=size;
	if (shift) {
		while (limb-- > move + 1) {
			dest->limbs[limb] = (source->limbs[limb-move] << shift) | (source->limbs[limb-move-1] >> (64 - shift));
		}
		dest->limbs[limb] = (source->limbs[limb-move] << shift);		
	} else {
		while (limb-- > move + 1) {
			dest->limbs[limb] = source->limbs[limb-move];
		}
		dest->limbs[limb] = source->limbs[limb-move];
	}
	if (move) {
		while (limb-- > 0) {
			dest->limbs[limb] = 0;
		}
	}

	dest->len = source->len + count;
	dest->neg = source->neg;

	return dest;
}

BigInt* bi_shift_right(BigInt* dest, BigInt* source) {
	if (is_zero(source)) {
		make_zero(dest);
		return dest;
	}

	uint64_t final_carry = 0;
	size_t size = normal_size(source->len);
	if (size > 1 && source->limbs[size-1] == 1) {
		final_carry = left_bit_mask;
		size--;
	}
	ensure(dest, size);

	for (size_t limb=0; limb < size-1; limb++) {
		dest->limbs[limb] = (source->limbs[limb] >> 1) | source->limbs[limb+1] << 63;
	}
	dest->limbs[size-1] = (source->limbs[size-1] >> 1) | final_carry;

	dest->len = source->len - 1;
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
			resize(dest, size+1, false);
		}
		dest->limbs[size] = 1;
	}
	update_len(dest);
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
	update_len(dest);
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

	clear(dest, normal_size(a->len*2));
	dest->neg = a->neg != b->neg;

	size_t limb = 0;
	uint64_t mask = 1;
	size_t shift = 0;
	while (limb < temp_b->size) {
		if (temp_b->limbs[limb] & mask) {
			bi_shift_left(temp_a, temp_a, shift);
			shift = 0;
			add(dest, dest, temp_a);
		}
		shift++;
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

	size_t shift = 0;
	if (r->len > temp_b->len) {
		shift = r->len - b->len;	
	}
	bi_shift_left(temp_b, temp_b, shift);
	if (bi_cmp_abs(temp_b, r) < 0) {
		bi_shift_left(temp_b, temp_b, 1);
		shift++;
	}
	bi_shift_left(mask, mask, shift);
	
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

BigInt* bi_mod(BigInt* r, BigInt* a, BigInt* b) {
	if (is_zero(b)) {
		puts("PANIC: Divide by zero!");
		return r;
	}

	copy(r, a);
	BigInt* orig_b = bi_clone(b);
	BigInt* temp_b = bi_clone(b);
	BigInt* mask = bi_from_int(1);

	size_t shift = 0;
	if (r->len > temp_b->len) {
		shift = r->len - b->len;	
	}
	bi_shift_left(temp_b, temp_b, shift);
	if (bi_cmp_abs(temp_b, r) < 0) {
		bi_shift_left(temp_b, temp_b, 1);
		shift++;
	}
	bi_shift_left(mask, mask, shift);
	
	while (bi_cmp_abs(r, orig_b) >= 0) {
		if (bi_cmp_abs(r, temp_b) >= 0) {
			sub(r, r, temp_b);
		}
		bi_shift_right(temp_b, temp_b);
		bi_shift_right(mask, mask);
	}

	bi_free(orig_b);
	bi_free(temp_b);
	bi_free(mask);

	return r;
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
				bi_mod(dest, dest, mod);
				if (temp_b->limbs[limb] & mask) {
					bi_mul(dest, dest, temp_a);
					bi_mod(dest, dest, mod);
				}
			}
			mask >>= 1;
		}

	} while (limb>0);

	bi_free(temp_a);
	bi_free(temp_b);

	return dest;
}

void bi_debug(BigInt* bi) {
	printf("%c %lu %lu/%lu\n", bi->neg ? '-' : '+', bi->len, normal_size(bi->len), bi->size);
	for (size_t limb = bi->size; limb-- > 0; ) {
		printf("  %016lx %064lb\n", bi->limbs[limb], bi->limbs[limb]);
	}
}