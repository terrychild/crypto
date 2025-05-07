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

	BigInt* a = bi_from_int(0);
	BigInt* b = bi_from_int(1);
	BigInt* c = bi_from_int(-1);
	BigInt* d = bi_from_int(2);
	BigInt* z = bi_from_int(0);
	
	bi_dump(a);

	bi_add(a, a, b);
	bi_dump(a);
	bi_sub(a, a, b);
	bi_dump(a);

	bi_sub(a, a, c);
	bi_dump(a);
	bi_add(a, a, c);
	bi_dump(a);

	bi_sub(a, a, b);
	bi_dump(a);
	bi_add(a, a, b);
	bi_dump(a);

	bi_add(a, a, c);
	bi_dump(a);
	bi_sub(a, a, c);
	bi_dump(a);

	printf("cmp %d\n", bi_cmp_abs(a, z));

	return EXIT_SUCCESS;
}