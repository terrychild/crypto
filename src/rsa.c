#include <stdlib.h>
#include <stdio.h>

#include "bigint.h"

int rsa() {
	BigInt* mod = bi_from_hex("C4F8E9E15DCADF2B96C763D981006A644FFB4415030A16ED1283883340F2AA0E2BE2BE8FA60150B9046965837C3E7D151B7DE237EBB957C20663898250703B3F");
	BigInt* private_key = bi_from_hex("8a7e79f3fbfea8ebfd18351cb9979136f705b4d9114a06d4aa2fd1943816677a5374661846a30c45b30a024b4d22b15ab323622b2de47ba29115f06ee42c41");
	BigInt* public_key = bi_from_hex("10001");

	BigInt* text = bi_from_hex("414243");
	BigInt* cipher = bi_new(0);

	char* temp = NULL;

	printf("mod: %s\n", bi_to_hex(temp, mod));
	printf("pri: %s\n", bi_to_hex(temp, private_key));
	printf("pub: %s\n", bi_to_hex(temp, public_key));

	printf("txt: %s\n", bi_to_hex(temp, text));
	bi_pow_mod(cipher, text, public_key, mod);
	printf("enc: %s\n", bi_to_hex(temp, cipher));
	bi_pow_mod(text, cipher, private_key, mod);
	printf("txt: %s\n", bi_to_hex(temp, text));

	return EXIT_SUCCESS;
}