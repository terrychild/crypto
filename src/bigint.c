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

static void normalise(BigInt* bi) {
	size_t new_size = bi->size;
	for (size_t i=bi->size; i-->1 && bi->limbs[i]==0;) {
		new_size--;
	}
	if (new_size < bi->size) {
		resize(bi, new_size);
	}

	if (new_size==1 && bi->limbs[0]==0) {
		bi->len = 0;
	} else {
		size_t new_len = new_size * 64;
		uint64_t mask = left_bit_mask;
		while (!(bi->limbs[new_size-1] & mask)) {
			new_len--;
			mask >>= 1;
		}
		bi->len = new_len;
	}
}

static BigInt* new(size_t size) {
	BigInt* bi = allocate(NULL, sizeof(*bi));
	bi->size = size;
	bi->len = 0;
	bi->neg = false;
	if (size > 0) {
		bi->limbs = allocate(NULL, sizeof(bi->limbs) * bi->size);
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
		free(bi->limbs);
		free(bi);
	}
}

static void copy_limbs(BigInt* dest, BigInt* source) {
	for (size_t limb = 0; limb < dest->size; limb++) {
		dest->limbs[limb] = limb < source->size ? source->limbs[limb] : 0;
	}
}
static void copy(BigInt* dest, BigInt* source) {
	if (dest->size != source->size) {
		resize(dest, source->size);
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
	if (bi->size != 1) {
		resize(bi, 1);
	}
	if (i>=0) {
		bi->neg = false;
		bi->limbs[0] = (uint64_t)i;
	} else {
		bi->neg = true;
		bi->limbs[0] = (uint64_t)(0-i);
	}
	normalise(bi);
	return bi;
}
BigInt* bi_from_int(int64_t i) {
	return bi_set_int(new(1), i);
}

BigInt* bi_set_hex(BigInt* bi, char* hex) {
	size_t nibbles = strlen(hex);
	size_t len = (nibbles+15)/16;

	resize(bi, len);

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

	normalise(bi);

	return bi;
}
BigInt* bi_from_hex(char* hex) {
	return bi_set_hex(new(0), hex);
}
size_t bi_to_hex(char* out, BigInt* bi, size_t maxlen) {
	size_t chars = (bi->len+3)/4;
	if (maxlen < chars) {
		chars = maxlen;
	}
	
	size_t limb = 0;
	size_t bits = 0;
	uint64_t mask = 0xf;
	for (size_t i=0; i<chars; i++) {
		out[chars-i-1] = get_hex_char((bi->limbs[limb] & mask) >> bits);
		bits += 4;
		mask <<= 4;

		if (!mask) {
			limb++;
			bits = 0;
			mask = 0xf;
		}
	}

	return chars;
}

BigInt* bi_set_char(BigInt* bi, char* data) {
	size_t chars = strlen(data);
	size_t len = (chars+7)/8;

	resize(bi, len);

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

	normalise(bi);

	return bi;
}
BigInt* bi_from_char(char* data) {
	return bi_set_char(new(0), data);
}
size_t bi_to_char(char* out, BigInt* bi, size_t maxlen) {
	size_t chars = (bi->len+7)/8;
	if (maxlen < chars) {
		chars = maxlen;
	}
	
	size_t limb = 0;
	size_t bits = 0;
	uint64_t mask = 0xff;
	for (size_t i=0; i<chars; i++) {
		out[chars-i-1] = (bi->limbs[limb] & mask) >> bits;
		bits += 8;
		mask <<= 8;

		if (!mask) {
			limb++;
			bits = 0;
			mask = 0xff;
		}
	}

	return chars;
}

static bool is_zero(BigInt* bi) {
	return bi->len == 0;
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
		for (size_t limb = a->size; limb-- > 0; ) {
			if (a->limbs[limb] < b->limbs[limb]) {
				return -1;
			} else if (a->limbs[limb] > b->limbs[limb]) {
				return 1;
			}
		}
	}
	return 0;
}

BigInt* bi_shift_left(BigInt* dest, BigInt* source) {
	if (is_zero(source)) {
		if (dest->size != 0) {
			resize(dest, 1);
		}
		dest->len = 0;
		dest->neg = false;
		dest->limbs[0] = 0;
		return dest;
	}

	copy(dest, source);
	size_t size = dest->size;

	bool carry = false;
	for (size_t limb=0; limb<size; limb++) {
		bool new_carry = source->limbs[limb] & left_bit_mask;
		dest->limbs[limb] = (source->limbs[limb] << 1) + (carry ? 1 : 0);
		carry = new_carry;
	}
	if (carry) {
		resize(dest, size+1);
		dest->limbs[size] = 1;
	}

	dest->len++;

	return dest;
}

BigInt* bi_shift_right(BigInt* dest, BigInt* source) {
	if (is_zero(source)) {
		if (dest->size != 0) {
			resize(dest, 1);
		}
		dest->len = 0;
		dest->neg = false;
		dest->limbs[0] = 0;
		return dest;
	}

	copy(dest, source);
	size_t size = dest->size;

	bool carry = false;
	for (size_t limb=size; limb-- > 0; ) {
		bool new_carry = source->limbs[limb] & 0x1;
		dest->limbs[limb] = (source->limbs[limb] >> 1) + (carry ? left_bit_mask : 0);
		carry = new_carry;
	}
	if (size > 1 && dest->limbs[size-1] == 0) {
		resize(dest, size-1);
	}

	dest->len--;
	
	return dest;
}

static void add(BigInt* dest, BigInt* a, BigInt* b) {
	size_t size = a->size;
	if (b->size > size) {
		size = b->size;
	}
	if (dest->size < size) {
		resize(dest, size);
	}

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
		resize(dest, size+1);
		dest->limbs[size] = 1;
	}

	normalise(dest);
}

static void sub(BigInt* dest, BigInt* a, BigInt* b) {
	size_t size = a->size;
	if (b->size > size) {
		size = b->size;
	}
	if (dest->size < size) {
		resize(dest, size);
	}

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

	normalise(dest);
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

	size_t size = a->size * 2;
	if (dest->size < size) {
		resize(dest, size);
	}
	for (size_t limb = 0; limb < size; limb++) {
		dest->limbs[limb] = 0;
	}
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

	normalise(dest);
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