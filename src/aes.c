#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

// Perform AES multiplication, i.e. polynomial multiplication in the field
// GF(256) with the irreducible polynomial x^8 + x^4 + x^3 + x + 1.  Designed
// to work with 8 bit numbers despite the irreducible polynomial requiring 9
// bits.
static uint8_t aes_mul(uint8_t a, uint8_t b) {
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
static uint8_t poly_div(uint8_t a, uint8_t b, uint8_t* r) {
	uint8_t mask = 0x80;
	while (mask && !(mask & a) && !(mask & b)) {
		mask >>= 1;
	}

	if (!(mask & a)) {
		*r = a;
		return 0;
	}
	
	uint8_t mag = 0;
	while (mask && !(mask & b)) {
		mask >>= 1;
		mag++;
	}

	return (1 << mag) ^ poly_div(a ^ (b << mag), b, r);
}
// Perform polynomial division with the AES irreducible polynomial (which
// would normally require 9 bits) with an 8 bit vector.
static uint8_t aes_div(uint8_t b, uint8_t* r) {
	uint8_t mask = 0x80;
	uint8_t mag = 1;
	while (mask && !(mask & b)) {
		mask >>= 1;
		mag++;
	}

	return (1 << mag) ^ poly_div(0x1b ^ (b << mag), b, r);
}

// Compute the AES multiplicative inverse using the Extended Euclidean Algorithm 
static uint8_t aes_inverse(uint8_t r) {
	uint8_t old_t = 0;
	uint8_t t = 1;

	uint8_t new_r;
	uint8_t q = aes_div(r, &new_r);

	while (new_r != 0) {		
		uint8_t new_t = old_t ^ aes_mul(q, t);
		old_t = t;
		t = new_t;

		uint8_t old_r = r;
		r = new_r;
		q = poly_div(old_r, r, &new_r);
	}

	return t;
}

// AES S-box values
static uint8_t forward_sbox[256] = {};
static uint8_t inverse_sbox[256] = {};

static void compute_sbox() {
	forward_sbox[0] = 0x63;
	inverse_sbox[0x63] = 0;

	uint8_t value = 0;	
	do {
		uint8_t inverse = aes_inverse(++value);
		forward_sbox[value] = inverse ^ rotl(inverse, 1) ^ rotl(inverse, 2) ^ rotl(inverse, 3) ^ rotl(inverse, 4) ^ 0x63;
		inverse_sbox[forward_sbox[value]] = value;
	} while (value != 255);
}

// Key expansion
static void copy_word(uint8_t dest[4], uint8_t src[4]) {
	for (size_t i=0; i<4; i++) {
		dest[i] = src[i];
	}
}
static void add_word(uint8_t dest[4], uint8_t word[4]) {
	for (size_t i=0; i<4; i++) {
		dest[i] ^= word[i];
	}
}
static void rotate_word(uint8_t word[4]) {
	uint8_t temp = word[0];
	for (size_t i=0; i<3; i++) {
		word[i] = word[i+1];
	}
	word[3] = temp;
}
static void sub_word(uint8_t word[4]) {
	for (size_t i=0; i<4; i++) {
		word[i] = forward_sbox[word[i]];
	}
}

static uint8_t* expand_key(uint8_t* key, uint8_t* initial_key, size_t starting_words, size_t total_words) {
	// initial key
	for (size_t i=0; i<starting_words; i++) {
		copy_word(key + (i*4), initial_key + (i*4));
	}

	// expand
	uint8_t rc = 0x01;
	for (size_t i=starting_words; i<total_words; i++) {
		uint8_t* current_word = key + (i*4);
		uint8_t* prev_word = key + ((i-1)*4);
		uint8_t* prev_round = key + ((i-starting_words)*4);

		copy_word(current_word, prev_word);
		if (i%starting_words == 0) {
			rotate_word(current_word);
			sub_word(current_word);
			current_word[0] ^= rc;
			rc = aes_mul(rc, 0x02);
		} else if (starting_words == 8 && i%4 == 0) {
			sub_word(current_word);
		}
		add_word(current_word, prev_round);
	}

	return key;
}

// encryption/decryption
static void add_blocks(uint8_t a[16], uint8_t b[16]) {
	for (size_t i=0; i<16; i++) {
		a[i] ^= b[i];
	}
}
static void sub_bytes(uint8_t block[16], uint8_t sbox[256]) {
	for (size_t i=0; i<16; i++) {
		block[i] = sbox[block[i]];
	}
}

static void shift_rows(uint8_t block[16]) {
	// row 2
	uint8_t temp = block[1];
	block[1] = block[5];
	block[5] = block[9];
	block[9] = block[13];
	block[13] = temp;

	// row 3
	temp = block[2];
	block[2] = block[10];
	block[10] = temp;
	temp = block[6];
	block[6] = block[14];
	block[14] = temp;

	// row 4
	temp = block[15];
	block[15] = block[11];
	block[11] = block[7];
	block[7] = block[3];
	block[3] = temp;
}
static void inverse_shift_rows(uint8_t block[16]) {
	// row 1
	uint8_t temp = block[13];
	block[13] = block[9];
	block[9] = block[5];
	block[5] = block[1];
	block[1] = temp;

	// row 2
	temp = block[2];
	block[2] = block[10];
	block[10] = temp;
	temp = block[6];
	block[6] = block[14];
	block[14] = temp;

	// row 3
	temp = block[3];
	block[3] = block[7];
	block[7] = block[11];
	block[11] = block[15];
	block[15] = temp;
}

static uint8_t columns_matrix[4][4] = {
	{2, 3, 1, 1},
	{1, 2, 3, 1},
	{1, 1, 2, 3},
	{3, 1, 1, 2}
};
static uint8_t inverse_columns_matrix[4][4] = {
	{14, 11, 13, 9},
	{9, 14, 11, 13},
	{13, 9, 14, 11},
	{11, 13, 9, 14}
};
static void mix_column(uint8_t block[4], uint8_t matrix[4][4]) {
	uint8_t result[4];
	
	for (size_t h=0; h<4; h++) {
		result[h] = 0;
		for (size_t w=0; w<4; w++) {
			result[h] ^= aes_mul(matrix[h][w], block[w]);
		}
	}

	for (size_t i=0; i<4; i++) {
		block[i] = result[i];
	}
}
static void mix_columns(uint8_t block[16], uint8_t matrix[4][4]) {
	for (size_t i=0; i<4; i++) {
		mix_column(block + (i*4), matrix);
	}
}

static void encrypt(uint8_t block[16], size_t rounds, uint8_t* key) {
	add_blocks(block, key);
	for (size_t round=1; round<=rounds; round++) {
		sub_bytes(block, forward_sbox);
		shift_rows(block);
		if (round<rounds) {
			mix_columns(block, columns_matrix);
		}
		add_blocks(block, key+(round*16));
	}
}

static void decrypt(uint8_t block[16], size_t rounds, uint8_t* key) {	
	for (size_t round=rounds; round>=1; round--) {
		add_blocks(block, key+(round*16));
		if (round<rounds) {
			mix_columns(block, inverse_columns_matrix);
		}
		inverse_shift_rows(block);
		sub_bytes(block, inverse_sbox);
	}
	add_blocks(block, key);
}

// modes of opperation
static void pkcs_pad(uint8_t block[16], size_t len) {
	uint8_t pad = 16 - len;
	for (size_t i=len; i<16; i++) {
		block[i] = pad;
	}
}

static void ecb_encrypt(size_t rounds, uint8_t* key, FILE *input_file, FILE *output_file) {
	uint8_t block[16];
	size_t read;

	do {
		read = fread(block, 1, 16, input_file);
		if (read<16) {
			pkcs_pad(block, read);
		}

		encrypt(block, rounds, key);

		fwrite(block, 1, 16, output_file);
	} while (read==16);
}

static void ecb_decrypt(size_t rounds, uint8_t* key, FILE *input_file, FILE *output_file) {
	uint8_t block[16];
	uint8_t next_block[16];
	size_t read;

	read = fread(block, 1, 16, input_file);
	while (read == 16) {
		decrypt(block, rounds, key);

		read = fread(next_block, 1, 16, input_file);
		if (read == 16) {
			fwrite(block, 1, 16, output_file);
		} else {
			fwrite(block, 1, 16-block[15], output_file);
		}
		memcpy(block, next_block, 16);
	}
}

static void cbc_encrypt(size_t rounds, uint8_t* key, uint8_t iv[16], FILE *input_file, FILE *output_file) {
	uint8_t block[16];
	uint8_t cb[16];
	size_t read;

	memcpy(cb, iv, 16);

	do {
		read = fread(block, 1, 16, input_file);
		if (read<16) {
			pkcs_pad(block, read);
		}

		add_blocks(block, cb);
		encrypt(block, rounds, key);
		memcpy(cb, block, 16);

		fwrite(block, 1, 16, output_file);
	} while (read==16);
}

static void cbc_decrypt(size_t rounds, uint8_t* key, uint8_t iv[16], FILE *input_file, FILE *output_file) {
	uint8_t block[16];
	uint8_t next_block[16];
	uint8_t cb[16];
	uint8_t next_cb[16];
	size_t read;

	memcpy(next_cb, iv, 16);

	read = fread(block, 1, 16, input_file);
	while (read == 16) {
		memcpy(cb, next_cb, 16);
		memcpy(next_cb, block, 16);

		decrypt(block, rounds, key);
		add_blocks(block, cb);

		read = fread(next_block, 1, 16, input_file);
		if (read == 16) {
			fwrite(block, 1, 16, output_file);
		} else {
			fwrite(block, 1, 16-block[15], output_file);
		}
		memcpy(block, next_block, 16);
	}
}

// do something with AES
int aes(bool enc, const char* mode, int argc, char* argv[]) {

	bool debug = arg(argc, argv, "--debug")>=0;

	// compute s-boxes if required
	if (forward_sbox[0] == 0) {
		compute_sbox();

		if (debug) {
			printf("s-box:\n");
			for (size_t i=0; i<16; i++) {
				for (size_t j=0; j<16; j++) {
					printf(" %02x", forward_sbox[i*16 + j]);
				}
				printf("\n");
			}
		}
	}

	// wrangle key
	int key_arg = arg(argc, argv, "--key=");
	if (key_arg<0) {
		puts("Missing key argument.");
		return EXIT_FAILURE;
	}

	size_t key_len = strlen(argv[key_arg]+6);
	if (key_len!=32 && key_len!=48 && key_len!=64) {
		puts("Invalid key length.");
		return EXIT_FAILURE;
	}

	size_t initial_key_words = key_len / 8;
	size_t rounds = initial_key_words + 6;
	size_t total_key_words = (rounds + 1) * 4;
	if (debug) {
		printf("key_len: %ld, initial_key_words: %ld, rounds: %ld, total_key_words: %ld\n", key_len*4, initial_key_words, rounds, total_key_words);
	}

	uint8_t* initial_key = allocate(NULL, sizeof(*initial_key) * key_len / 2);
	uint8_t* key = allocate(NULL, sizeof(*key) * total_key_words * 4);
	read_hex(initial_key, (uint8_t*)(argv[key_arg]+6), key_len / 2);
	expand_key(key, initial_key, initial_key_words, total_key_words);
		
	if (debug) {
		printf("initial key:\n");
		for (size_t i=0; i<initial_key_words; i++) {
			printf(" ");
			for (size_t j=0; j<4; j++) {
				printf("%02x", key[i*4 + j]);
			}
		}
		printf("\n");
		printf("expanded key:");
		for (size_t i=0; i<total_key_words; i++) {
			if (i%initial_key_words == 0) {
				printf("\n");
			}
			printf(" ");
			for (size_t j=0; j<4; j++) {
				printf("%02x", key[i*4 + j]);
			}
		}
		printf("\n");
	}

	free(initial_key);

	// wrangle IV
	uint8_t* iv = NULL;
	if (strcmp(mode, "cbc")==0) {
		int iv_arg = arg(argc, argv, "--iv=");
		if (iv_arg<0) {
			puts("Missing iv argument.");
			return EXIT_FAILURE;
		}

		if (strlen(argv[iv_arg]+5)!=32) {
			puts("invalid iv length");
			return EXIT_FAILURE;
		}

		iv = allocate(NULL, sizeof(*iv) * 16);
		read_hex(iv, (uint8_t*)(argv[iv_arg]+5), 16);
	}

	// open files
	FILE *input_file;
	int fin_arg = arg(argc, argv, "--fin=");
	if (fin_arg<0) {
		input_file = stdin;
	} else {
		input_file = fopen(argv[fin_arg]+6, "r");
		if (input_file == NULL) {
			printf("unable to open file \"%s\"\n", argv[fin_arg]+6);
			return EXIT_FAILURE;
		}
	}

	FILE *output_file;
	int fout_arg = arg(argc, argv, "--fout=");
	if (fout_arg<0) {
		output_file = stdout;
	} else {
		output_file = fopen(argv[fout_arg]+7, "w");
		if (output_file == NULL) {
			printf("unable to open file \"%s\"\n", argv[fout_arg]+7);
			return EXIT_FAILURE;
		}
	}

	// do the actual work
	if (strcmp(mode, "ecb")==0) {
		if (enc) {	
			ecb_encrypt(rounds, key, input_file, output_file);
		} else {
			ecb_decrypt(rounds, key, input_file, output_file);
		}
	} else if (strcmp(mode, "cbc")==0) {
		if (enc) {	
			cbc_encrypt(rounds, key, iv, input_file, output_file);
		} else {
			cbc_decrypt(rounds, key, iv, input_file, output_file);
		}
	}

	// close files
	if (fin_arg>=0) {
		fclose(input_file);
	}
	if (fout_arg>=0) {
		fclose(output_file);
	}

	// tidy up
	free(key);
	free(iv);

	return EXIT_SUCCESS;
}