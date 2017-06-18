#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <math.h>
#include <map>
#include <unordered_set>
#include <string>
#include "lucky777.h"

static uint32_t seed[3] = {1,2,3};
static int custom_seed = 0;

#define hashsum(str, len) lucky777_string_hash3((str), (len), seed[0], seed[1], seed[2])
#include "check.inc.cpp"

static char buffer[1024*1024];

int main(int argc, char** argv) {
	unsigned i;
	int hex = 0;
	int use_std = 0;
	int check = 0;
	int binary = 0;
	int only_hash = 2;
	uint32_t sum = 0;
	ssize_t lsize;
	char *lbuf = NULL;
	size_t lcapa = 0;
	FILE* r;
	table tbl = {0, 0, NULL};
	std::unordered_set<std::string> set;

	for (i=1; i<argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			goto usage;
		} else if (strcmp(argv[i], "-s") == 0) {
			use_std = 1;
		} else if (strcmp(argv[i], "-c") == 0) {
			check = 1;
		} else if (strcmp(argv[i], "-x") == 0) {
			hex = 1;
			fill_dehex();
		} else if (strcmp(argv[i], "-r") == 0) {
			custom_seed = -1;
		} else if (strcmp(argv[i], "-speed") == 0) {
			only_hash = 1;
		} else if (strcmp(argv[i], "-table") == 0) {
			only_hash = 0;
		} else if (strcmp(argv[i], "-seed") == 0) {
			uint32_t s;
			i++;
			if (i == argc)
				goto usage;
			if (custom_seed == sizeof(seed)/sizeof(seed[0]))
				goto usage;
			s = strtol(argv[i], NULL, 0); /* strtol recognize hex */
			seed[custom_seed++] = s;
		} else if (strcmp(argv[i], "-b") == 0) {
			i++;
			if (i == argc)
				goto usage;
			binary = atoi(argv[i]);
		} else {
			goto usage;
		}
	}

	if (only_hash != 2) {
		setbuffer(stdin, buffer, sizeof(buffer));
	}

	if (custom_seed == 0 && !only_hash || custom_seed == -1) {
		r = fopen("/dev/urandom", "rb");
		if (r == NULL) {
			perror("open /dev/urandom");
			return 1;
		}
		if (fread(&seed, sizeof(seed), 1, r) != 1) {
			perror("Could not read seed from /dev/urandom");
			return 1;
		}
		fclose(r);
	} else if (custom_seed == 1) {
		const uint32_t c = 0xeb0d2f41;
		seed[1] = seed[0] ^ c;
		seed[2] = ((seed[0]<<15)|(seed[0]>>17)) ^ c;
	} else if (custom_seed == 2) {
		seed[2] = seed[0] ^ seed[1];
	}
	

	if (binary) {
		lbuf = (char*)malloc(binary+1);
		while ((lsize = fread(lbuf, 1, binary, stdin)) > 0) {
			if (only_hash) {
				uint32_t hash;
				hash = lucky777_string_hash2(lbuf, lsize, seed[0], seed[1]);
				if (only_hash == 2) {
					printf("%08u\n", hash);
				}
				sum += hash;
			} else if (use_std == 0) {
				table_insert(&tbl, lbuf, lsize);
			} else {
				set.insert(std::string(lbuf, lsize));
			}
		}
	} else {
		char* copy;
		while ((lsize = getline(&lbuf, &lcapa, stdin)) != -1) {
			lsize--;
			if (only_hash == 2) {
				copy = strndup(lbuf, lsize);
			}
			if (hex) {
				assert((lsize&1) == 0);
				dehexify(lbuf, lsize);
				lsize /= 2;
			}
			if (only_hash) {
				uint32_t hash;
				hash = lucky777_string_hash2(lbuf, lsize, seed[0], seed[1]);
				if (only_hash == 2) {
					printf("%08x\t%s\n", hash, copy);
					free(copy);
				}
				sum += hash;
			} else if (use_std == 0) {
				table_insert(&tbl, lbuf, lsize);
			} else {
				set.insert(std::string(lbuf, lsize));
			}
		}
	}
	if (only_hash) {
		if (only_hash == 1)
			printf("Sum = %08x\n", sum);
		return 0;
	}

	if (use_std == 0) {
		if (tbl.size == 0) {
			goto usage;
		}

		printf("tbl->size == %u\n", tbl.size);
		if (check) {
			return check_table(&tbl, 1);
		}
	} else {
		if (set.size() == 0) {
			goto usage;
		}
		printf("tbl->size == %zu\n", set.size());
		if (check) {
			for (auto& s: set) {
				checksum_add(s.data(), s.size());
			}
			printf("checksum %08x %08x\n", checksum[0], checksum[1]);
		}
	}
	return 0;
usage:
	printf("Usage: %s [-seed N] [-x] [-b N] [-speed|-table] [-s] [-c] < input_file\n%s", argv[0],
	"  Default, \"test vector mode\": reads lines from stdin and report checksum for every line.\n" \
	"  With -speed , only checksumming performed.\n" \
	"  With -table , puts them into chained hash table, on exit it \n" \
	"    calculates and outputs checksum and table size.\n" \
	"\t--help  - this help.\n" \
	"\t-seed N - use custom seed instead of random.\n" \
	"\t          Every invocation fills next seed array element,\n" \
	"\t          so you may give this option 1 or 2 times.\n" \
	"\t-b N    - input is binary, slice it to chunks of fixed size N\n" \
	"\t-x      - input lines are hexified, so de-hexify it\n" \
	"\t-r      - force random seed for \"test vector\"\n" \
	"\t-speed  - test speed only\n" \
	"\t-table  - test hash table (options below for this mode only)\n" \
	"\t-s      - use c++ set to verify hash table implementation\n" \
	"\t-c      - at the end, detect collisions and chain length, and\n" \
	"\t          calculate independent checksum for all inserted strings\n");
	printf("  example for \"test vector\":\n" \
		"\t%s < test_lucky777.cpp\n" \
		"\techo 'a1b2bc3def' | %s \n" \
		"\techo 'a1b2bc3def' | %s -seed 2 -seed 3 -seed 4\n" \
		"\techo 'a1b2bc3def' | %s -x -seed 2 -seed 3 -seed 4\n",
		argv[0], argv[0], argv[0], argv[0]
		);
	return 0;
}
