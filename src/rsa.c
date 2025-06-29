#include <stdlib.h>
#include <stdio.h>

#include "bigint.h"

static char* get_hex(char buffer[256], BigInt* bi) {
	size_t len = bi_to_hex(buffer, bi, 255);
	buffer[len] = '\0';
	return buffer;
}

int rsa() {
	/*BigInt* mod = bi_from_hex("C4F8E9E15DCADF2B96C763D981006A644FFB4415030A16ED1283883340F2AA0E2BE2BE8FA60150B9046965837C3E7D151B7DE237EBB957C20663898250703B3F");
	BigInt* private_key = bi_from_hex("8a7e79f3fbfea8ebfd18351cb9979136f705b4d9114a06d4aa2fd1943816677a5374661846a30c45b30a024b4d22b15ab323622b2de47ba29115f06ee42c41");
	BigInt* public_key = bi_from_hex("10001");*/
	BigInt* mod = bi_from_int(780);
	BigInt* private_key = bi_from_int(413);
	BigInt* public_key = bi_from_int(17);

	//BigInt* text = bi_from_char("ABC");
	BigInt* text = bi_from_int(20);
	BigInt* cipher = bi_new();

	char buf[256];

	printf("mod: %s\n", get_hex(buf, mod));
	printf("pri: %s\n", get_hex(buf, private_key));
	printf("pub: %s\n", get_hex(buf, public_key));

	printf("txt: %s\n", get_hex(buf, text));
	bi_pow_mod(cipher, text, public_key, mod);
	printf("enc: %s\n", get_hex(buf, cipher));
	bi_pow_mod(text, cipher, private_key, mod);
	printf("dec: %s\n", get_hex(buf, text));

	return EXIT_SUCCESS;
}