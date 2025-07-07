#include <stdlib.h>
#include <stdio.h>

#include "bigint.h"

static char* get_hex(char buffer[1024], BigInt* bi) {
	size_t len = bi_to_hex(buffer, bi, 1023);
	buffer[len] = '\0';
	return buffer;
}
static char* get_char(char buffer[1024], BigInt* bi) {
	size_t len = bi_to_char(buffer, bi, 1023);
	buffer[len] = '\0';
	return buffer;
}

int rsa() {
	//BigInt* mod = bi_from_hex("C4F8E9E15DCADF2B96C763D981006A644FFB4415030A16ED1283883340F2AA0E2BE2BE8FA60150B9046965837C3E7D151B7DE237EBB957C20663898250703B3F");
	//BigInt* private_key = bi_from_hex("8a7e79f3fbfea8ebfd18351cb9979136f705b4d9114a06d4aa2fd1943816677a5374661846a30c45b30a024b4d22b15ab323622b2de47ba29115f06ee42c41");
	//BigInt* public_key = bi_from_hex("10001");
	
	BigInt* mod = bi_from_hex("c5062b58d8539c765e1e5dbaf14cf75dd56c2e13105fecfd1a930bbb5948ff328f126abe779359ca59bca752c308d281573bc6178b6c0fef7dc445e4f826430437b9f9d790581de5749c2cb9cb26d42b2fee15b6b26f09c99670336423b86bc5bec71113157be2d944d7ff3eebffb28413143ea36755db0ae62ff5b724eecb3d316b6bac67e89cacd8171937e2ab19bd353a89acea8c36f81c89a620d5fd2effea896601c7f9daca7f033f635a3a943331d1b1b4f5288790b53af352f1121ca1bef205f40dc012c412b40bdd27585b946466d75f7ee0a7f9d549b4bece6f43ac3ee65fe7fd37123359d9f1a850ad450aaf5c94eb11dea3fc0fc6e9856b1805ef");
	BigInt* private_key = bi_from_hex("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000086c94f");
	BigInt* public_key = bi_from_hex("49e5786bb4d332f94586327bde088875379b75d128488f08e574ab4715302a87eea52d4c4a23d8b97af7944804337c5f55e16ba9ffafc0c9fd9b88eca443f39b7967170ddb8ce7ddb93c6087c8066c4a95538a441b9dc80dc9f7810054fd1e5c9d0250c978bb2d748abe1e9465d71a8165d3126dce5db2adacc003e9062ba37a54b63e5f49a4eafebd7e4bf5b0a796c2b3a950fa09c798d3fa3e86c4b62c33ba9365eda054e5fe74a41f21b595026acf1093c90a8c71722f91af1ed29a41a2449a320fc7ba3120e3e8c3e4240c04925cc698ecd66c7c906bdf240adad972b4dff4869d400b5d13e33eeba38e075e872b0ed3e91cc9c283867a4ffc3901d2069f");

	//BigInt* mod = bi_from_int(3233);
	//BigInt* private_key = bi_from_int(413);
	//BigInt* public_key = bi_from_int(17);

	char buf1[1024];
	char buf2[1024];
	char buf3[1024];

	printf("mod: %s\n", get_hex(buf1, mod));
	printf("pri: %s\n", get_hex(buf1, private_key));
	printf("pub: %s\n", get_hex(buf1, public_key));

	/*for (int i=1; i<16; i++) {
		BigInt* message_in = bi_from_int(i);
		BigInt* cipher = bi_new();
		BigInt* message_ou = bi_new();

		bi_pow_mod(cipher, message_in, public_key, mod);
		bi_pow_mod(message_out, cipher, private_key, mod);

		printf("%s -> %s -> %s\n", get_hex(buf1, message_in), get_hex(buf2, cipher), get_hex(buf3, message_out));
	}*/

	BigInt* message_in = bi_from_char("ABC");
	BigInt* cipher = bi_new();
	BigInt* message_out = bi_new();

	bi_pow_mod(cipher, message_in, public_key, mod);
	bi_pow_mod(message_out, cipher, private_key, mod);

	printf("%s -> %s -> %s\n", get_char(buf1, message_in), get_hex(buf2, cipher), get_char(buf3, message_out));


	/*printf("mod: %s\n", get_hex(buf1, mod));

	for (int i=0xa00; i<=0xa01; i++) {
		BigInt* e = bi_from_int(i);
		BigInt* message = bi_from_int(2);
		BigInt* cipher = bi_new();

		bi_pow_mod(cipher, message, e, mod);

		printf("%s -> %s (%s)\n", get_hex(buf1, message), get_hex(buf2, cipher), get_hex(buf3, e));
	}*/

	/*BigInt* a = bi_from_hex("2");
	BigInt* b = bi_from_hex("2");
	BigInt* c = bi_new();
	BigInt* r = bi_new();

	bi_debug(a);
	
	puts(">1");
	bi_mul(a, a, a);
	bi_div(c, a, mod, a);
	bi_debug(a);
	bi_mul(a, a, b);
	bi_div(c, a, mod, a);
	bi_debug(a);

	for (int i=0; i<15; i++) { //15
		puts(">0");
		bi_mul(a, a, a);
		bi_div(c, a, mod, a);
		bi_debug(a);
	}*/

	/*puts("slow");
	BigInt* ref0 = bi_from_hex("1b80e09050afe126fad597e1fe3ebc68b625a3210ba5ac333dacd41ad8df0b9c61c9b0c1449bcacaab6ae83cd3bddb29a680666cf1574a8b3b212f98092422c5");
	BigInt* ref1 = bi_from_hex("2f4703fc652a7ee0933cbff7dcbcf74177850ff28e27c0ce76964fde7dc9fd8b3a37983ae93b2220e01fd6da1d2f5348042d58c51b7f104c751907d7963ecf1f511050418359969f36bf90cd7faf5beb8a332767d2178d9aebd81c9fcdc31f5c9c56df2b5db20f78f3546f9d56ef7f7d0f476debf7fd6ebe4cb2f7ba620eb99");
	BigInt* ref2 = bi_from_hex("61dfad60a082ce265aaf30bba67ed9f5c2b467db0efc2d7e411c6ec2830e94ad76c649a6bdb3301e7a78c97b1fe29bc7200061dc0b5e314ecef20fdf597447c");

	printf("comp0: %d\n", bi_cmp(a, ref0));
	bi_mul(a, a, a);
	bi_debug(a);
	printf("comp1: %d\n", bi_cmp(a, ref1));
	bi_div(c, a, mod, a);
	bi_debug(a);
	printf("comp2: %d\n", bi_cmp(a, ref2));*/

	/*puts(">1");
	bi_mul(a, a, a);
	bi_div(c, a, mod, a);
	bi_debug(a);
	bi_mul(a, a, b);
	bi_div(c, a, mod, a);
	bi_debug(a);*/

	/*puts("test");
	bi_set_hex(a, "1b80e09050afe126fad597e1fe3ebc68b625a3210ba5ac333dacd41ad8df0b9c61c9b0c1449bcacaab6ae83cd3bddb29a680666cf1574a8b3b212f98092422c5");
	bi_debug(a);
	bi_mul(a, a, a);
	bi_debug(a);
	bi_div(c, a, mod, r);
	bi_debug(mod);
	bi_debug(a);
	bi_debug(c);
	bi_debug(r);*/

	/*puts("test 2");
	bi_debug(mod);
	BigInt* a = bi_from_hex("2f4703fc652a7ee0933cbff7dcbcf74177850ff28e27c0ce76964fde7dc9fd8b3a37983ae93b2220e01fd6da1d2f5348042d58c51b7f104c751907d7963ecf1f511050418359969f36bf90cd7faf5beb8a332767d2178d9aebd81c9fcdc31f5c9c56df2b5db20f78f3546f9d56ef7f7d0f476debf7fd6ebe4cb2f7ba620eb99");
	bi_debug(a);

	BigInt* temp_mod = bi_clone(mod);
	BigInt* mask = bi_from_int(1);
	while (bi_cmp_abs(temp_mod, a) < 0) {
		bi_shift_left(temp_mod, temp_mod);
		bi_shift_left(mask, mask);
	}

	BigInt* c = bi_new();

	while (bi_cmp_abs(a, mod) >= 0) {
		if (bi_cmp_abs(a, temp_mod) >= 0) {
			uadd(c, c, mask);
			usub(a, a, temp_mod);
		}
		bi_shift_right(temp_mod, temp_mod);
		bi_shift_right(mask, mask);
	}

	puts("a");
	bi_debug(a);
	puts("c");
	bi_debug(c);

	puts("test 3");
	bi_set_hex(a, "1b80e09050afe126fad597e1fe3ebc68b625a3210ba5ac333dacd41ad8df0b9c61c9b0c1449bcacaab6ae83cd3bddb29a680666cf1574a8b3b212f98092422c5");
	puts("a");
	bi_debug(a);
	bi_mul(a, a, a);
	puts("a");
	bi_debug(a);
	bi_div(c, a, mod, a);

	puts("a");
	bi_debug(a);
	puts("c");
	bi_debug(c);*/

	//bi_div(c, a, b, r);
	//bi_mul(c, a, b);
	
	/*puts("a");
	bi_debug(a);
	puts("b");
	bi_debug(b);
	puts("c");
	bi_debug(c);
	puts("r");
	bi_debug(r);*/

	return EXIT_SUCCESS;
}