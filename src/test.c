#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "bigint.h"

int test() {
	/*
	//BigInt* a = bi_from_hex("81223344556677ffffffffffffff");
	//BigInt* b = bi_from_hex("ffffffffffffff");
	BigInt* a = bi_from_hex("f");
	//BigInt* b = bi_from_hex("10");
	bi_dump(a);
	//bi_dump(b);
	bi_add(a, a, a);
	bi_dump(a);
	bi_normalise(a);
	bi_dump(a);

	bi_free(a);
	//bi_free(b);*/

	/*int8_t a = 15;
	int8_t b = 30;
	int8_t r = a - b;

	printf("%08b %08b %08b %d\n", a, b, (uint8_t)r, r);*/

	/*for (int8_t i=0; i<8; i++) {
		int8_t j = 0-i;
		printf("%d %08b %d %08b\n", i, (uint8_t)i, j, (uint8_t)j);
	}
	for (uint8_t i=0; i<8; i++) {
		uint8_t j = 0-i;
		printf("%d %08b %d %08b\n", i, i, (int8_t)j, j);
	}*/

	BigInt* a = bi_from_hex("18000000000000000");
	BigInt* b = bi_from_int(1);
			
	bi_debug(a);
	for (uint8_t i=0; i<3; i++) {
		bi_mul(a, a, a);
		bi_debug(a);
	}
	bi_add(a, a, b);
	bi_debug(a);
	bi_mul(a, a, a);
	bi_debug(a);

	/*BigInt* a = bi_from_hex("800000000000000000000000000000000000000000000000000000000000000");
	bi_debug(a);
	bi_shift_left(a, a, 1);
	bi_debug(a);*/
	
	return EXIT_SUCCESS;
}