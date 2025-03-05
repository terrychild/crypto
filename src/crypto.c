#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char to_hex_char(unsigned char nibble) {
	if (nibble < 10) {
		return nibble + 48;
	} else {
		return (nibble-10) + 97;
	}
}

void print_hex(unsigned char byte) {
	printf("%c%c", to_hex_char((byte & 0xF0) >> 4), to_hex_char(byte & 0x0F));
}
void print_bin(unsigned char byte) {
	for (int i=0; i<8; i++) {
		printf("%c", byte & 0x80 ? '1' : '0');
		byte <<= 1;
	}
}

unsigned char aes_mul(unsigned char a, unsigned char b) {
	unsigned char result = 0;

	for (int i=0; i<8; i++) {
		if (b & 0x01) {
			result ^= a;
		}
		a = (a << 1) ^ (a & 0x80 ? 0x1b : 0);
		b >>= 1;
	}

	return result;
}

/*unsigned char aes_div(unsigned char a, unsigned char b, unsigned char* r) {

}*/

int gcd(int a, int b) {
	if (b == 0) return a;
	return gcd(b, a % b);
}

struct eea_terms {
	int r;
	int s;
	int t;
};

struct eea_terms eea_recurse(struct eea_terms i2, struct eea_terms i1) {
	
	if (i1.r==0) return i2;

	int q = i2.r / i1.r;
	return eea_recurse(i1, (struct eea_terms) {
		.r = i2.r % i1.r,
		.s = i2.s - q*i1.s,
		.t = i2.t - q*i1.t
	});
}

struct eea_terms eea(int r0, int r1) {
	return eea_recurse((struct eea_terms) {r0, 1, 0}, (struct eea_terms) {r1, 0, 1});
}

int inverse(int a, int mod) {
	struct eea_terms result = eea(mod, a);

	return result.t<0 ? result.t+mod : result.t;
}

unsigned int gf2_mul(unsigned int a, unsigned int b) {
	unsigned int result = 0;

	size_t len = sizeof(int)*8;
	for (size_t i=0; i<len; i++) {
		if (b & 0x01) {
			result ^= a;
		}
		a = a << 1;
		b >>= 1;
	}

	return result;
}

unsigned int gf2_div(unsigned int a, unsigned int b, unsigned int* r) {
	if (b>a) {
		*r = a;
		return 0;
	}

	unsigned int mask = 1 << ((sizeof(int)*8) - 1);
	while (mask && !(mask & a)) {
		mask >>= 1;
	}
	unsigned int mag = 0;
	while (mask && !(mask & b)) {
		mask >>= 1;
		mag++;
	}

	return (1 << mag) ^ gf2_div(a ^ (b << mag), b, r);
}

struct eea_terms gf2_eea_recurse(struct eea_terms i2, struct eea_terms i1) {
	
	if (i1.r==0) return i2;

	unsigned int r;
	unsigned int q = gf2_div(i2.r, i1.r, &r);
	return gf2_eea_recurse(i1, (struct eea_terms) {
		.r = r,
		.s = i2.s ^ gf2_mul(q, i1.s),
		.t = i2.t ^ gf2_mul(q, i1.t)
	});
}

struct eea_terms gf2_eea(unsigned int r0, unsigned int r1) {
	return gf2_eea_recurse((struct eea_terms) {r0, 1, 0}, (struct eea_terms) {r1, 0, 1});
}

unsigned int gf2_inverse(unsigned int a, unsigned int mod) {
	struct eea_terms result = gf2_eea(mod, a);

	return result.t;
}

/*int gf2_eea(int a, int b, int* s, int* t) {
	*s = 1;
	*t = 0;

	if (b == 0) return a;

	int r2 = a;
	int r1 = b;
	int s1 = 0;
	int t1 = 1;

	do {
		int r0;
		int q1 = gf2_div(r2, r1, &r0);

		r2 = r1;
		r1 = r0;

		int s0 = (*s) ^ gf2_mul(q1, s1);
		*s = s1;
		s1 = s0;

		int t0 = (*t) ^ gf2_mul(q1, t1);
		*t = t1;
		t1 = t0;
	} while (r1 != 0);

	return r2;
}

int gf2_inverse(int a, int mod) {
	int s, t;
	gf2_eea(mod, a, &s, &t);

	//return (t<0 ? t+mod : t) % mod;
	return t;
}*/

/*unsigned char poly_gcd(unsigned char a, unsigned char b) {
	if (b==0) return a;
	if (b>a) return gcd(b, a);
	return poly_gcd(b, )
}*/

void compute_sbox() {
	unsigned char p = 1;
	unsigned char q = 1;

	do {
		p = p ^ (p << 1) ^ (p & 0x80 ? 0x1b : 0);
		q = aes_mul(q, 3);

		print_hex(p);
		printf(" ");
		print_hex(q);
		printf(" ");
		print_bin(q);
		printf("\n");
	} while (p != 1);
}

int main(/*int argc, char* argv[]*/) {
	/*if (argc < 3) {
		puts("expected 2 parametters");
		return EXIT_FAILURE;
	}

	// validate action
	if (argv[1][0]!='e' && argv[1][0]!='d') {
		printf("unknown action \"%s\"\n", argv[1]);
		return EXIT_FAILURE;
	}

	// validate key
	size_t key_len = strlen(argv[2])/2;
	if (key_len!=16 && key_len!=24 && key_len!=32) {
		puts("invalid key length");
		return EXIT_FAILURE;
	}
	char* key = from_hex(argv[2], key_len);

	// do input things
	char* input;
	size_t len;
	if (argc == 4) {
		len = strlen(argv[3])/2;
		input = from_hex(argv[3], len);
	} else {
		input = malloc(BLOCK_SIZE);
		len = fread(input, 1, BLOCK_SIZE, stdin);
	}

	puts(input);*/

	//compute_sbox();

	/*unsigned char max = 6;
	for (unsigned char i=0; i<max; i++) {
		unsigned char ai = max-i;
		unsigned char mi = gcd(max, i);
		printf("%d %d %d %d %d\n", i, ai, (i+ai) % max, mi, (i*mi) % max);
	}*/

	//printf("%d\n", gcd(25, 15));
	//struct eea_terms eear = eea(973, 301);
	//printf("%d x %d + %d x %d = %d\n", eear.s, 973, eear.t, 301, eear.r);

	/*int max = 7;
	for (int i=0; i<max; i++) {
		struct eea_terms eear = eea(max, i);
		printf("%d:  r =%2d, s =%2d, t =%2d, inverse =%2d\n", i, eear.r, eear.s, eear.t, inverse(i, max));
	}*/

	unsigned int r;
	for(unsigned int i=1; i<8; i++) {
		print_bin(0x0b);
		printf(" / ");
		print_bin(i);
		printf(" => ");
		print_bin(gf2_div(0x0b, i, &r));
		printf(" r ");
		print_bin(r);
		printf(", inverse =  ");
		print_bin(gf2_inverse(i, 0x0b));
		printf("\n");
	}

	// open files
	/*
	FILE *input_file = fopen(argv[1], "r");
	if (input_file == NULL) {
		printf("unable to open file \"%s\"\n", argv[1]);
		return EXIT_FAILURE;
	}

	FILE *output_file = fopen(argv[2], "w");
	if (output_file == NULL) {
		printf("unable to open file \"%s\"\n", argv[2]);
		return EXIT_FAILURE;
	}

	// read write
	char buffer[BLOCK_SIZE];
	size_t read;
	while ((read = fread(buffer, 1, BLOCK_SIZE, input_file)) > 0) {
		fwrite(encode(buffer, read), 1, read, output_file);
	}

	// close files
	fclose(input_file);
	fclose(output_file);*/

	return EXIT_SUCCESS;
}