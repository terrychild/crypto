#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "aes.h"
#include "rand.h"
#include "test.h"

static void printUsage() {
	puts("Usage: crypto <command> [<args>]");
	puts("\nCommands");
	puts("  {enc|dec}-aes-ecb  Encrypt or decrypt using AES in Electronic");
	puts("                     codebook mode.");
	puts("  {enc|dec}-aes-cbc  Encrypt or decrypt using AES in Cipher Block");
	puts("                     Chaining mode.");
	puts("  rand               Generate random data.");
	//puts("  test               Run test code.");
	puts("\nArguments");
	puts("  --key=<key>        Encryption key encoded in hex.");
	puts("  --iv=<key>         Initialisation Vector encoded in hex.");
	puts("  --fin=<path>       Path to input file. If not specified stdin");
	puts("                     will be used instead.");
	puts("  --fout=<path>      Path to output file to save result. If not");
	puts("                     specified stdout will be used instead.");
	puts("  --len=<num>        Length in decimal.");
	puts("  --hex              Output in hexidecimal.");
	puts("  --debug            Output debug information.");
}

int main(int argc, char* argv[]) {
	if (argc<2) {
		printUsage();	
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "enc-aes-ecb")==0) {
		return aes(true, "ecb", argc, argv);
	}
	if (strcmp(argv[1], "dec-aes-ecb")==0) {
		return aes(false, "ecb", argc, argv);
	}
	if (strcmp(argv[1], "enc-aes-cbc")==0) {
		return aes(true, "cbc", argc, argv);
	}
	if (strcmp(argv[1], "dec-aes-cbc")==0) {
		return aes(false, "cbc", argc, argv);
	}

	if (strcmp(argv[1], "rand")==0) {
		return random_cmd(argc, argv);
	}

	if (strcmp(argv[1], "test")==0) {
		return test();
	}

	printUsage();
	return EXIT_FAILURE;
}