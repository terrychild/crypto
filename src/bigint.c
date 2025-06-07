#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "utils.h"
#include "bigint.h"

uint64_t overflow_mask = 0x8000000000000000;

static void resize(BigInt* bi, size_t new_size) {
	bi->size = new_size;
	bi->limbs = allocate(bi->limbs, sizeof(bi->limbs) * new_size);
}

static void normalise(BigInt* bi) {
	size_t new_size = bi->size;
	for (size_t i=bi->size - 1; i>0 && bi->limbs[i] == 0; i--) {
		new_size--;
	}
	if (new_size < bi->size) {
		resize(bi, new_size);
	}
	size_t new_len = new_size * 63;
	uint64_t mask = overflow_mask >> 1;
	while (mask && !(bi->limbs[new_size-1] & mask)) {
		new_len--;
		mask >>= 1;
	}
	bi->len = new_len;
}

static bool is_zero(BigInt* bi) {
	return bi->size == 1 && bi->limbs[0] == 0;
}

static BigInt* new(size_t size) {
	BigInt* bi = allocate(NULL, sizeof(*bi));
	bi->size = size==0 ? 1 : size;
	bi->len = 0;
	bi->neg = false;
	bi->limbs = allocate(NULL, sizeof(bi->limbs) * bi->size);
	return bi;
}

BigInt* bi_new(size_t size) {
	BigInt* bi = new(size);
	for (size_t limb = 0; limb < bi->size; limb++) {
		bi->limbs[limb] = 0;
	}
	return bi;
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

BigInt* bi_from_hex(char* str) {
	size_t len = strlen(str);
	size_t bi_len = ((len*4)+62)/63;

	BigInt* bi = new(bi_len);

	size_t limb = 0;
	size_t bits = 0;
	uint64_t value=0;
	for (size_t i=0; i<len; i++) {
		uint64_t ch = read_hex_char(str[len-i-1]);
		value += ch << bits;
		bits += 4;
		
		if (bits >= 63) {
			bi->limbs[limb++] = value & ~overflow_mask;
			bits -= 63;
			value = ch >> (4 - bits);
		}
	}
	bi->limbs[limb] = value;

	normalise(bi);
	return bi;
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

void bi_free(BigInt* bi) {
	if (bi != NULL) {
		free(bi->limbs);
		free(bi);
	}
}

char* bi_to_hex(char* str, BigInt* bi) {
	size_t i = ((bi->len+3) / 4) + (bi->neg ? 1 : 0);
	str = allocate(str, i+1);
	if (bi->neg) {
		str[0] = '-';
	}
	str[i] = 0;

	char nibble = 0;
	for (size_t limb=0; limb<bi->size; limb++) {
		int shift = limb % 4;
		shift = shift == 0 ? shift : -4 + shift;

		if (shift < 0) {
			uint64_t mask = 0xf;
			mask >>= -shift;
			str[--i] = get_hex_char(nibble | ((bi->limbs[limb] & mask) << -shift));
			shift += 4;
			nibble = 0;
		}
		while (shift < 60 && i>0) {
			uint64_t mask = 0xf;
			mask <<= shift;
			str[--i] = get_hex_char((bi->limbs[limb] & mask) >> shift);
			shift += 4;
		}
		if (shift < 63) {
			uint64_t mask = 0xf;
			mask = (mask << shift) & ~overflow_mask;
			nibble = (bi->limbs[limb] & mask) >> shift;
		}
	}
	if (nibble && i>0) {
		str[--i] = get_hex_char(nibble);
	}
	return str;
}

void bi_debug(char* str, BigInt* bi) {
	printf("%s: (%ld) %c\n", str, bi->len, bi->neg ? '-' : '+');
	uint64_t* ptr = bi->limbs + bi->size;
	while (ptr != bi->limbs) {
		ptr--;
		printf(" %01lb %063lb\n", (*ptr & overflow_mask) >> 63, *ptr & ~overflow_mask);
	}
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

BigInt* bi_shift_left(BigInt* dest, BigInt* source, size_t count) {
	if (is_zero(source)) {
		if (dest->size != 0) {
			resize(dest, 1);
		}
		dest->len = 0;
		dest->neg = false;
		dest->limbs[0] = 0;
		return dest;
	}

	size_t new_limbs = count / 63;
	size_t shift = count % 63;
	size_t old_size = source->size;
	size_t new_size = source->size + new_limbs;

	if (source->limbs[old_size-1] >> (63 - shift)) {
		new_size++;
	}
	if (dest->size != new_size) {
		resize(dest, new_size);
	}

	if (new_size != old_size) {
		dest->limbs[new_size-1] = 0;
	}
	for (size_t i = old_size; i-- > 0; ) {
		uint64_t carry = source->limbs[i] >> (63 - shift);
		if (carry) {
			dest->limbs[i+new_limbs+1] |= carry;
		}
		dest->limbs[i+new_limbs] = (source->limbs[i] << shift) & ~overflow_mask;
	}
	for (size_t i = new_limbs; i-- > 0; ) {
		dest->limbs[i] = 0;
	}

	dest->len = source->len + 1;

	return dest;
}

BigInt* bi_shift_right(BigInt* dest, BigInt* source, size_t count) {
	size_t rm_limbs = count / 63;
	size_t shift = count % 63;

	if (source->size <= rm_limbs || is_zero(source)) {
		if (dest->size != 1) {
			resize(dest, 1);
		}
		dest->len = 0;
		dest->neg = false;
		dest->limbs[0] = 0;
		return dest;
	}

	size_t new_size = source->size - rm_limbs;

	dest->limbs[0] = source->limbs[rm_limbs] >> shift;
	for (size_t i=1; i<new_size; i++) {
		uint64_t carry = (source->limbs[i+rm_limbs] << (63 - shift)) & ~overflow_mask;
		if (carry) {
			dest->limbs[i-1] |= carry;
		}
		dest->limbs[i] = source->limbs[i+rm_limbs] >> shift;
	}

	if (new_size > 1 && dest->limbs[new_size-1] == 0) {
		new_size--;
	}
	if (dest->size != new_size) {
		resize(dest, new_size);
	}

	dest->len = dest->len - 1;

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
		if (i < a->size) {
			if (i < b->size) {
				dest->limbs[i] = a->limbs[i] + b->limbs[i] + carry;
			} else {
				dest->limbs[i] = a->limbs[i] + carry;
			}
		} else {
			dest->limbs[i] = b->limbs[i] + carry;
		}
		if (dest->limbs[i] & overflow_mask) {
			carry = 1;
			dest->limbs[i] &= ~overflow_mask;
		} else {
			carry = 0;
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
		if (i < a->size) {
			if (i < b->size) {
				dest->limbs[i] = a->limbs[i] - b->limbs[i] - borrow;
			} else {
				dest->limbs[i] = a->limbs[i] - borrow;
			}
		} else {
			dest->limbs[i] = b->limbs[i] - borrow;
		}
		if (dest->limbs[i] & overflow_mask) {
			borrow = 1;
			dest->limbs[i] &= ~overflow_mask;
		} else {
			borrow = 0;
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
	if (dest->neg && dest->size==1 && dest->limbs[0]==0) {
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
	if (dest->neg && dest->size==1 && dest->limbs[0]==0) {
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
		bi_shift_left(temp_a, temp_a, 1);
		mask <<= 1;
		if (mask & overflow_mask) {
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
		bi_shift_left(temp_b, temp_b, 1);
		bi_shift_left(mask, mask, 1);
	}
	
	while (bi_cmp_abs(r, orig_b) >= 0) {
		if (bi_cmp_abs(r, temp_b) >= 0) {
			add(dest, dest, mask);
			sub(r, r, temp_b);
		}
		bi_shift_right(temp_b, temp_b, 1);
		bi_shift_right(mask, mask, 1);
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
		uint64_t mask = overflow_mask >> 1;
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
	BigInt* temp = bi_new(0);
	BigInt* temp_a = bi_clone(a);
	BigInt* temp_b = bi_clone(b);

	copy(dest, a);

	bool go = false;
	size_t limb = temp_b->size;
	do {
		limb--;
		uint64_t mask = overflow_mask >> 1;
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