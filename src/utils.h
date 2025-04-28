#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

void* allocate(void* data, size_t size);
int arg(int argc, char* argv[], const char* name);

char read_hex_char(char nibble);
void read_hex_be(uint8_t* dest, char* source, size_t len);
void read_hex_le(uint8_t* dest, char* source, size_t len);

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