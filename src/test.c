#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "bigint.h"

int test() {
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
	//bi_free(b);

	return EXIT_SUCCESS;
}