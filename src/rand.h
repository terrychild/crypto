#ifndef RAND_H
#define RAND_H

#include <stdint.h>

size_t random(uint8_t* buffer, size_t max_len);

int random_cmd(int argc, char* argv[]);

#endif