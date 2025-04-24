#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "bigint.h"

int test() {
	BigInt* i = bi_new(0);
	bi_from_hex(i, "66778899aabbccddeeff");
	bi_dump(i);
	bi_free(i);

	return EXIT_SUCCESS;
}