#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
/*#include <string.h>*/

char to_hex_char(uint8_t nibble) {
	if (nibble < 10) {
		return nibble + 48;
	} else {
		return (nibble-10) + 97;
	}
}

void print_hex(uint8_t byte) {
	printf("%c%c", to_hex_char((byte & 0xF0) >> 4), to_hex_char(byte & 0x0F));
}
void print_bin(uint8_t byte) {
	for (int i=0; i<8; i++) {
		printf("%c", byte & 0x80 ? '1' : '0');
		byte <<= 1;
	}
}
void print_poly(uint8_t byte) {
	bool first = true;
	for (int i=7; i>=0; i--) {
		if (byte & 0x80) {
			if (first) {
				first = false;
			} else {
				printf("+");
			}

			switch(i) {				
				case 7: printf("x\u2077"); break;
				case 6: printf("x\u2076"); break;
				case 5: printf("x\u2075"); break;
				case 4: printf("x\u2074"); break;
				case 3: printf("x\u00B3"); break;
				case 2: printf("x\u00B2"); break;
				case 1: printf("x"); break;
				case 0: printf("1"); break;
			}
		}
		byte <<= 1;
	}
}

uint8_t rotl8(uint8_t value, unsigned int count) {
	const unsigned int mask = CHAR_BIT * sizeof(value) - 1;
	count &= mask;
	return (value << count) | (value >> (-count & mask));
}

// Perform AES multiplication, i.e. polynomial multiplication in the field
// GF(256) with the irreducible polynomial x^8 + x^4 + x^3 + x + 1.  Designed
// to work with 8 bit numbers despite the irreducible polynomial requiring 9
// bits.
uint8_t aes_mul(uint8_t a, uint8_t b) {
	uint8_t result = 0;

	for (int i=0; i<8; i++) {
		if (b & 0x01) {
			result ^= a;
		}
		a = (a << 1) ^ (a & 0x80 ? 0x1b : 0);
		b >>= 1;
	}

	return result;
}

// Perform polynomial division with 8 bit vectors.  
uint8_t aes_div(uint8_t a, uint8_t b, uint8_t* r) {
	if (b>a) {
		*r = a;
		return 0;
	}

	uint8_t mask = 0x80;
	while (mask && !(mask & a)) {
		mask >>= 1;
	}
	uint8_t mag = 0;
	while (mask && !(mask & b)) {
		mask >>= 1;
		mag++;
	}

	return (1 << mag) ^ aes_div(a ^ (b << mag), b, r);
}
// Perform polynomial AES irreducible polynomial (which would require 9 bits)
// with an 8 bit vector.
uint8_t aes_div_irreducible(uint8_t b, uint8_t* r) {
	uint8_t mask = 0x80;
	uint8_t mag = 1;
	while (mask && !(mask & b)) {
		mask >>= 1;
		mag++;
	}

	return (1 << mag) ^ aes_div(0x1b ^ (b << mag), b, r);
}

// Compute the AES multiplicative inverse using the Extended Euclidean Algorithm 
uint8_t aes_inverse(uint8_t r) {
	uint8_t old_t = 0;
	uint8_t t = 1;

	uint8_t new_r;
	uint8_t q = aes_div_irreducible(r, &new_r);

	while (new_r != 0) {		
		uint8_t new_t = old_t ^ aes_mul(q, t);
		old_t = t;
		t = new_t;

		uint8_t old_r = r;
		r = new_r;
		q = aes_div(old_r, r, &new_r);
	}

	return t;
}

// compute the AES S-box values
void aes_compute_sbox(uint8_t forward[256], uint8_t backward[256]) {
	uint8_t value = 0;

	forward[0] = 0x63;
	backward[0x63] = 0;

	do {
		uint8_t inverse = aes_inverse(++value);
		forward[value] = inverse ^ rotl8(inverse, 1) ^ rotl8(inverse, 2) ^ rotl8(inverse, 3) ^ rotl8(inverse, 4) ^ 0x63;
		backward[forward[value]] = value;
	} while (value != 255);
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

	uint8_t aes_forward_sbox[256];
	uint8_t aes_backward_sbox[256];
	aes_compute_sbox(aes_forward_sbox, aes_backward_sbox);

	for (int i=0; i<16; i++) {
		for (int j=0; j<16; j++) {
			print_hex(aes_forward_sbox[i*16 + j]);
			printf(" ");
		}
		printf("\n");
	}

	printf("\n");
	for (int i=0; i<16; i++) {
		for (int j=0; j<16; j++) {
			print_hex(aes_backward_sbox[i*16 + j]);
			printf(" ");
		}
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