#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "utils.h"

static void mxv(size_t width, size_t height, int matrix[height][width], int vector[width], int result[height]) {
	for (size_t h=0; h<height; h++) {
		result[h] = 0;
		for (size_t w=0; w<width; w++) {
			result[h] += matrix[h][w] * vector[w];
		}
	}
}

static void dump_matrix(size_t width, size_t height, int matrix[height][width]) {
	for (size_t h=0; h<height; h++) {
		for (size_t w=0; w<width; w++) {
			printf("%d ", matrix[h][w]);
		}
		printf("\n");
	}
}

static void dump_vector(size_t size, int vector[size]) {
	for (size_t i=0; i<size; i++) {
		printf("%d\n", vector[i]);
	}
}

int test() {
	// vector test
	enum {width = 8, height = 8};

	int matrix[height][width] = {
		{0, 0, 1, 0, 0, 1, 0, 1},
		{1, 0, 0, 1, 0, 0, 1, 0},
		{0, 1, 0, 0, 1, 0, 0, 1},
		{1, 0, 1, 0, 0, 1, 0, 0},
		{0, 1, 0, 1, 0, 0, 1, 0},
		{0, 0, 1, 0, 1, 0, 0, 1},
		{1, 0, 0, 1, 0, 1, 0, 0},
		{0, 1, 0, 0, 1, 0, 1, 0} 
	};
	//int vector[width] = {1, 1, 0, 0, 0, 1, 1, 0};

	for (int j=0; j<256; j++) {
		int vector[width] = {};
		int mask = 0x80;
		for (int i=7; i>=0; i--) {
			vector[i] = (j & mask) >> i;
			mask >>= 1;
		}

		int result[height];
		mxv(width, height, matrix, vector, result);
		//dump_vector(height, result);

		int value = 0;
		for (int i=7; i>=0; i--) {
			value <<= 1;
			value |= result[i] & 0x01;
		}
		value ^= 0x05;
		printf("%02x %08b = %08b %02x\n", j, j, value, value);
	}

	/*uint8_t v = 0;
	printf("%08b\n", v);
	v = sbit(v, 1, true);
	printf("%08b\n", v);

	v = sbit(v, 8, true);
	printf("%.8b\n", v);
	printf("%b\n", rbit(v, 1));
	printf("%b\n", rbit(v, 2));

	v = sbit(v, 2, true);
	printf("%.8b\n", v);
	printf("%b\n", rbit(v, 1));
	printf("%b\n", rbit(v, 2));

	v = sbit(v, 2, false);
	printf("%.8b\n", v);
	printf("%b\n", rbit(v, 1));
	printf("%b\n", rbit(v, 2));*/

	return EXIT_SUCCESS;
}