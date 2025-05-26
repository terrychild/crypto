#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "utils.h"
#include "bigint.h"

uint64_t overflow_mask = 0x8000000000000000;

void resize(BigInt* bi, size_t new_size) {
	bi->size = new_size;
	bi->limbs = allocate(bi->limbs, sizeof(bi->limbs) * new_size);
}

void reduce(BigInt* bi) {
	size_t new_size = bi->size;
	for (size_t i=bi->size - 1; i>0 && bi->limbs[i] == 0; i--) {
		new_size--;
	}
	if (new_size < bi->size) {
		resize(bi, new_size);
	}
}

bool is_zero(BigInt* bi) {
	return bi->size == 1 && bi->limbs[0] == 0;
}

BigInt* new(size_t size) {
	BigInt* bi = allocate(NULL, sizeof(*bi));
	bi->size = size==0 ? 1 : size;
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

BigInt* bi_from_int(int64_t i) {
	BigInt* bi = new(1);
	if (i>=0) {
		bi->limbs[0] = (uint64_t)i;
	} else {
		bi->neg = true;
		bi->limbs[0] = (uint64_t)(0-i);
	} 
	return bi;
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

	reduce(bi);
	return bi;
}

void copy(BigInt* dest, BigInt* source) {
	for (size_t limb = 0; limb < dest->size; limb++) {
		dest->limbs[limb] = limb < source->size ? source->limbs[limb] : 0;
	}
}

BigInt* bi_clone(BigInt* source) {
	BigInt* dest = new(source->size);
	copy(dest, source);
	return dest;
}

void bi_free(BigInt* bi) {
	if (bi != NULL) {
		free(bi->limbs);
		free(bi);
	}
}

void bi_dump(char* str, BigInt* bi) {
	printf("%s: %c", str, bi->neg ? '-' : '+');
	char nibble = 0;
	for (size_t limb = bi->size; limb-- > 0; ) {
		printf(" ");
		int shift = 60 + (limb % 4);
		while (shift >= 0) {
			uint64_t mask = 0xf;
			mask <<= shift;
			nibble += (bi->limbs[limb] & mask) >> shift;
			printf("%01x", nibble);
			nibble = 0;
			shift -= 4;
		}
		if (shift < 0) {
			uint64_t mask = 0xf;
			mask >>= -shift;
			nibble = (bi->limbs[limb] & mask) << -shift;
		}
	}
	printf("\n");
}

void bi_debug(BigInt* bi) {
	printf("BigInt %c\n", bi->neg ? '-' : '+');
	uint64_t* ptr = bi->limbs + bi->size;
	while (ptr != bi->limbs) {
		ptr--;
		printf(" %01lb %016lx %063lb\n", (*ptr & overflow_mask) >> 63, *ptr & ~overflow_mask, *ptr & ~overflow_mask);
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
	if (a->size < b->size) {
		return -1;
	} else if (a->size > b->size) {
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

	return dest;
}

BigInt* bi_shift_right(BigInt* dest, BigInt* source, size_t count) {
	size_t rm_limbs = count / 63;
	size_t shift = count % 63;

	if (source->size <= rm_limbs || is_zero(source)) {
		if (dest->size != 1) {
			resize(dest, 1);
		}
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

	reduce(dest);
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

	BigInt* tempA = bi_clone(a);
	BigInt* tempB = bi_clone(b);

	size_t size = a->size * 2;
	if (dest->size < size) {
		resize(dest, size);
	}
	for (size_t limb = 0; limb < size; limb++) {
		dest->limbs[limb] = 0;
	}
	
	size_t limb = 0;
	uint64_t mask = 1;
	while (limb < tempB->size) {
		if (tempB->limbs[limb] & mask) {
			add(dest, dest, tempA);
		}
		bi_shift_left(tempA, tempA, 1);
		mask <<= 1;
		if (mask & overflow_mask) {
			limb++;
			mask = 1;
		}
	}
	
	bi_free(tempA);
	bi_free(tempB);

	reduce(dest);
	return dest;
}