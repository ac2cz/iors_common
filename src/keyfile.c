/*
 * keyfile.c
 *
 *  Created on: Nov 27, 2023
 *      Author: g0kla
 */

#include "stdlib.h"
#include <string.h>
#include "common_config.h"
#include "keyfile.h"


/* The default key, overwritten from keyfile if it exists, at startup. */
uint8_t hmac_sha_key[AUTH_KEY_SIZE] = {
    0x49, 0xc2, 0x90, 0x2e, 0x9d, 0x99, 0x32,
    0xf0, 0x9a, 0x09, 0x32, 0xb9, 0x8c, 0x09,
    0x8e, 0x98, 0xa9, 0x80, 0xd0, 0x98, 0x92,
    0xc8, 0x9e, 0x98, 0xd7, 0x9f, 0x98, 0x7e
};

/* Test key 1 */
uint8_t hmac_sha_key1[AUTH_KEY_SIZE] = {
    0x19, 0x72, 0x20, 0x2e, 0xbd, 0x49, 0x12,
    0x10, 0x8a, 0x03, 0x62, 0xb9, 0x5c, 0x19,
    0x1e, 0x18, 0xa3, 0x50, 0xb0, 0x48, 0x42,
    0x18, 0x2e, 0x28, 0xd7, 0xbf, 0x38, 0x2e
};

uint32_t key_checksum(uint8_t *key) {
    unsigned int i;
    uint32_t checksum = 0;

    for(i=0; i<AUTH_KEY_SIZE; i++)
        checksum += key[i];
    return checksum;
}


/**
 * Load the key from a file.  If the key is corrupt or we can not load it
 * then use the default key
 *
 * TODO - add a checksum to make sure the key has integrity - could be part of more general check for our files
 *
 * TODO - If the file is corrupt or does not work and we were trying to use file on the
 * USB drive then fall back to the SD card copy
 */
int key_load(char * key_path) {
	debug_print("Loading keyfile: %s\n",key_path);
    uint8_t key[AUTH_KEY_SIZE];

    FILE * f = fopen(key_path, "r");
    if (f == NULL) {
    	debug_print("No keyfile, using default\n");
    	return EXIT_FAILURE;
    }
    int num = fread(key, sizeof(char), AUTH_KEY_SIZE, f);
    if (num == 0) {
    	fclose(f);
    	debug_print("Empty keyfile, using default\n");
    	return EXIT_FAILURE; // nothing was read
    }

    memcpy(hmac_sha_key, key, AUTH_KEY_SIZE);
    return EXIT_SUCCESS;
}

int test_key_save(char * key_path) {
	FILE * outfile = fopen(key_path, "wb");
	if (outfile == NULL) return EXIT_FAILURE;

	for (int i=0; i<AUTH_KEY_SIZE; i++) {
		int c = fputc(hmac_sha_key1[i],outfile);
		if (c == EOF) {
			fclose(outfile);
			return EXIT_FAILURE; // we could not write to the file
		}
	}
	fclose(outfile);
	return EXIT_SUCCESS;
}
