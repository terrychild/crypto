#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

void* allocate(void* data, size_t size);
int arg(int argc, char* argv[], const char* name);

void rhex8(uint8_t* dest, char* source, size_t len);
void rhex64(uint64_t* dest, char* source, size_t len);
#define rhex(dest, source, len) _Generic((dest), \
		uint64_t*: rhex64, \
		default: rhex8 \
	)(dest, source, len)

void rhex64_le(uint64_t* dest, char* source, size_t len);
#define rhex_le(dest, source, len) _Generic((dest), \
		uint64_t*: rhex64_le \
	)(dest, source, len)

uint8_t rbit(unsigned int value, unsigned int bit);

uint8_t sbit8(uint8_t value, unsigned int bit, bool set);
#define sbit(value, bit, set) _Generic((value), \
		default: sbit8	\
	)(value, bit, set)

uint8_t rotl8(uint8_t value, unsigned int count);
#define rotl(value, count) _Generic((value),	\
		default: rotl8	\
	)(value, count)

#endif