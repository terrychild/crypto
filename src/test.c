#include <stdlib.h>
#include <stdio.h>
#include "rsa.h"
#include "bigint.h"

static char* get_hex(char buffer[1024], BigInt* bi) {
	size_t len = bi_to_hex(buffer, bi, 1023);
	buffer[len] = '\0';
	return buffer;
}

int test() {
	return rsa();

	/*BigInt* mod = bi_from_hex("C4F8E9E15DCADF2B96C763D981006A644FFB4415030A16ED1283883340F2AA0E2BE2BE8FA60150B9046965837C3E7D151B7DE237EBB957C20663898250703B3F");
	BigInt* ref1 = bi_from_hex("2f4703fc652a7ee0933cbff7dcbcf74177850ff28e27c0ce76964fde7dc9fd8b3a37983ae93b2220e01fd6da1d2f5348042d58c51b7f104c751907d7963ecf1f511050418359969f36bf90cd7faf5beb8a332767d2178d9aebd81c9fcdc31f5c9c56df2b5db20f78f3546f9d56ef7f7d0f476debf7fd6ebe4cb2f7ba620eb99");
	BigInt* ref2 = bi_from_hex("61dfad60a082ce265aaf30bba67ed9f5c2b467db0efc2d7e411c6ec2830e94ad76c649a6bdb3301e7a78c97b1fe29bc7200061dc0b5e314ecef20fdf597447c");
	BigInt* a = bi_from_hex("1b80e09050afe126fad597e1fe3ebc68b625a3210ba5ac333dacd41ad8df0b9c61c9b0c1449bcacaab6ae83cd3bddb29a680666cf1574a8b3b212f98092422c5");
	BigInt* c = bi_new();
	bi_debug(a);
	bi_mul(a, a, a);
	bi_debug(a);
	printf("comp1: %d\n", bi_cmp(a, ref1));
	bi_div(c, a, mod, a);
	bi_debug(a);
	printf("comp2: %d\n", bi_cmp(a, ref2));*/

	/*BigInt* a = bi_from_hex("3c0000000000000000000000000000000");
	bi_debug(a);
	for (int i=1; i<4; i++) {
		bi_shift_right(a, a, 125);
		bi_debug(a);
	}

	return EXIT_SUCCESS;*/
}