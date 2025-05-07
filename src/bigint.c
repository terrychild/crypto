#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "utils.h"
#include "bigint.h"

uint64_t overflow_mask = 0x8000000000000000;

void resize(BigInt* bi, size_t new_size) {
	bi->size = new_size;
	bi->limbs = allocate(bi->limbs, sizeof(bi->limbs) * bi->size);
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

BigInt* bi_new(size_t size) {
	BigInt* bi = allocate(NULL, sizeof(*bi));
	bi->size = size==0 ? 1 : size;
	bi->neg = false;
	bi->limbs = allocate(NULL, sizeof(bi->limbs) * bi->size);
	return bi;
}

BigInt* bi_from_int(int64_t i) {
	BigInt* bi = bi_new(1);
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

	BigInt* bi = bi_new(bi_len);

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

void bi_free(BigInt* bi) {
	if (bi != NULL) {
		free(bi->limbs);
		free(bi);
	}
}

void bi_dump(BigInt* bi) {
	char sign = bi->neg ? '-' : '+';
	printf("BigInt %c\n", sign);
	uint64_t* ptr = bi->limbs + bi->size;
	while (ptr != bi->limbs) {
		ptr--;
		printf(" %01lb %016lx %063lb\n", (*ptr & overflow_mask) >> 63, *ptr & ~overflow_mask, *ptr & ~overflow_mask);
	}

	printf(" %c", sign);
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
		dest->limbs = allocate(dest->limbs, sizeof(dest->limbs) * size);
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