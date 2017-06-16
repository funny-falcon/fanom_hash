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

static uint64_t seed[2];

struct entry {
	uint64_t hash;
	struct entry *next;
	uint32_t count;
	size_t len;
	char str[1];
};

struct table {
	uint32_t size;
	uint32_t bins;
	struct entry **entries;
};

//typedef struct entry entry;
//typedef struct table table;

void table_rehash(table *table) {
	uint32_t old_bins = table->bins;
	uint32_t i, mask, pos;
	entry **pe, *e;
	table->bins = old_bins == 0 ? 8 : old_bins * 2;
	mask = table->bins - 1;
	table->entries = (entry**)realloc(table->entries, table->bins*sizeof(entry*));
	memset(table->entries+old_bins, 0, sizeof(entry*)*(table->bins - old_bins));
	assert(table->entries != NULL);
	for (i = 0; i < old_bins; i++) {
		pe = &table->entries[i];
		while (*pe != NULL) {
			e = *pe;
			pos = e->hash & mask;
			if (pos != i) {
				*pe = e->next;
				e->next = table->entries[pos];
				table->entries[pos] = e;
			} else {
				pe = &e->next;
			}
		}
	}
}

void table_insert(table *table, const char* str, size_t len) {
	uint64_t hash;
	uint32_t pos;
	entry* en;
	/* lets fill factor to be 3x, so it will be easier to get collisions */
	if (table->size == table->bins) {
		table_rehash(table);
	}
	hash = lucky777_string_hash2(str, len, seed[0], seed[1]);
	pos = (uint32_t)hash & (table->bins - 1);
	en = table->entries[pos];
	while (en != NULL) {
		if (en->hash == hash && en->len == len && memcmp(en->str, str, len) == 0) {
			/* found match */
			return;
		}
		en = en->next;
	}
	en = (entry*)malloc(sizeof(entry) + len);
	en->hash = hash;
	en->next = table->entries[pos];
	en->len = len;
	memcpy(en->str, str, len);
	en->str[len] = 0;
	table->entries[pos] = en;
	table->size++;
}

uint32_t checksum[2] = {0, 0};
#define rotl(x, n) (((x) << (n)) | ((x) >> (sizeof(x)*8 - (n))))
void checksum_add(const char* p, size_t len) {
	size_t i;
	uint64_t h0 = 0x100, h1 = 0x200;
	for (i = 0; i < len; i++) {
		h0 += (uint8_t)p[i];
		h1 += (uint8_t)p[i];
		h0 *= 0x1234567;
		h1 *= 0xabcdef9;
		h0 = rotl(h0, 13);
		h1 = rotl(h1, 19);
	}
	checksum[0] ^= h0;
	checksum[1] ^= h1;
}

static char dehex[256];
void fill_dehex() {
	int i;
	for (i=0; i<255; i++) dehex[i] = -1;
	for (i='0'; i<='9'; i++) dehex[i] = i-'0';
	for (i='a'; i<='f'; i++) dehex[i] = i-'a'+10;
	for (i='A'; i<='F'; i++) dehex[i] = i-'A'+10;
}
void dehexify(char* s, ssize_t len) {
	ssize_t i, j;
	for (i=0,j=0; i<len; i+=2, j++) {
		char c1 = s[i];
		char c2 = s[i+1];
		assert(c1 != -1 && c2 != -1);
		s[j] = (c1<<4) + c2;
	}
}

int main(int argc, char** argv) {
	unsigned i;
	int hex = 0;
	int use_std = 0;
	int check = 0;
	int binary = 0;
	int only_hash = 0;
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
		} else if (strcmp(argv[i], "-o") == 0) {
			only_hash = 1;
		} else if (strcmp(argv[i], "-b") == 0) {
			i++;
			if (i == argc)
				goto usage;
			binary = atoi(argv[i]);
		} else {
			goto usage;
		}
	}
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
	

	if (binary) {
		lbuf = (char*)malloc(binary+1);
		while ((lsize = fread(lbuf, 1, binary, stdin)) > 0) {
			if (only_hash) {
				sum += lucky777_string_hash2(lbuf, lsize, seed[0], seed[1]);
			} else if (use_std == 0) {
				table_insert(&tbl, lbuf, lsize);
			} else {
				set.insert(std::string(lbuf, lsize));
			}
		}
	} else {
		while ((lsize = getline(&lbuf, &lcapa, stdin)) != -1) {
			lsize--;
			if (hex) {
				assert((lsize&1) == 0);
				dehexify(lbuf, lsize);
				lsize /= 2;
			}
			if (only_hash) {
				sum += lucky777_string_hash2(lbuf, lsize, seed[0], seed[1]);
			} else if (use_std == 0) {
				table_insert(&tbl, lbuf, lsize);
			} else {
				set.insert(std::string(lbuf, lsize));
			}
		}
	}
	if (only_hash) {
		printf("Sum = %u\n", sum);
		return 0;
	}

	if (use_std == 0) {
		if (tbl.size == 0) {
			goto usage;
		}

		printf("tbl->size == %u\n", tbl.size);
		if (check) {
			uint32_t maxchain = 0;
			double expectedmaxchain = 2 * log(tbl.bins) / log(log(tbl.bins));
			std::map<uint64_t, uint32_t> counter;
			int ret = 0;
			for (i = 0; i < tbl.bins; i++) {
				uint32_t chain = 0;
				entry* e = tbl.entries[i];
				while (e != NULL) {
					chain++;
					checksum_add(e->str, e->len);
					counter[e->hash]++;
					e = e->next;
				}
				if (chain > maxchain)
					maxchain = chain;
			}
			printf("checksum %08x %08x\n", checksum[0], checksum[1]);
			printf("maxchain: %u (expected %f)", maxchain, expectedmaxchain);
			if (maxchain > expectedmaxchain*2) {
				printf("(!!! maxchain overflow)");
				ret = 1;
			}
			printf("\n");
			if (counter.size() != tbl.size) {
				double ncoll = tbl.size - (uint32_t)counter.size();
				double expected = 0;
				uint32_t maxcnt = 0;
				double d = pow(2,32);
				double n = tbl.size;
				/* https://en.wikipedia.org/wiki/Birthday_problem#Collision_counting */
				expected = n - d + d*pow((d-1)/d, n);
				if (expected > 0.01 && ncoll <= 2) {
					ret = 0;
				} else {
					ret = expected*4 < ncoll;
				}
				for (auto& pair: counter) {
					if (pair.second-1 > maxcnt) {
						maxcnt = pair.second-1;
					}
				}
				printf("Has full hash collision: %u same hashes for %u values (expected %f)\n",
						(uint32_t)ncoll, tbl.size, expected);
				if (maxcnt > 1) {
					printf("Max full collision: %u\n", maxcnt);
					ret = 1;
				}
			}
			return ret;
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
	printf("Usage: %s [-x]\n%s", argv[0],
		"  reads lines from stdin and puts them into chained hash table.\n" \
		"  on exit it calculates and outputs checksum and table size.\n" 
		"\t-b N   - input is binary, slice it to chunks of fixed size N\n" \
		"\t-x     - input lines are hexified, so de-hexify it\n" \
		"\t-c     - at the end, compute checksum for all inserted strings\n" \
		"\t-s     - use c++ set to check hash table implementation\n" \
		"\t-o     - only hash, do not hash table\n" \
		"\t--help - this help.\n");
	return 0;
}
