#ifndef UTILS_H
#define UTILS_H

void* allocate(void* data, size_t size);
int arg(int argc, char* argv[], const char* name);
void read_hex(uint8_t* dest, uint8_t* source, size_t len);

uint8_t rbit(unsigned int value, unsigned int bit);

uint8_t sbit8(uint8_t value, unsigned int bit, bool set);
#define sbit(value, bit, set) _Generic((value), \
		default : sbit8	\
	)(value, bit, set)

uint8_t rotl8(uint8_t value, unsigned int count);
#define rotl(value, count) _Generic((value),	\
		default : rotl8	\
	)(value, count)

#endif