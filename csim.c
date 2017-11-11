/*	
 * csim.c
 *
 * Fill in file header comment with your name(s) and a short paragraph about
 * what your program does.
 * This program simulates the behavior of a cache, specified by the command
 * line arguments. 
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include "cachelab.h"

//typedef struct Line Line;
typedef unsigned long int mem_addr;

typedef struct {
	unsigned int valid;
	mem_addr tag;
	int lru;
} Line;

typedef struct {
	Line* lines;
} Set;

typedef struct {
	Set* sets;
} Cache;

// forward declaration
FILE* openFile(char* trace_file);
void createCache(Cache *cache, int num_sets, int lines_per_set);
void freeCache(Cache *cache, int num_sets, int lines_per_set); 
void getLineInfo(mem_addr addr, mem_addr *tag, int *set_index,
	   	int block_size, int num_sets);
void simulateCache(char *trace_file, int num_sets, int block_size, 
		int lines_per_set, int verbose);
void cacheOp(Cache *cache, int lines_per_set, int set_index, int tag, 
		int *LRU, int *hit_count, int *miss_count, int *eviction_count);   

/**
 * Prints out a reminder of how to run the program.
 *
 * @param executable_name Strign containing the name of the executable.
 */
void usage(char *executable_name) {
	printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", executable_name);
}

int main(int argc, char *argv[]) {
	int verbose_mode = 0;
	int num_sets = -1;
	int lines_per_set = -1;
	int block_size = -1;
	char *trace_filename = NULL;
	
	opterr = 0;
	int c = -1;

	while ((c = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (c) {
			case 'h':
				// enable helper mode
				usage("csim");
				break;
			case 'v':
				// enable verbose mode
				verbose_mode = 1;
				break;
			case 's':
				// specify the number of sets
				num_sets = 1 << strtol(optarg, NULL, 10); // = Big S
				break;
			case 'E':
				// specify number of lines per set		
				lines_per_set = strtol(optarg, NULL, 10);
				break;
			case 'b':
				// specify the number of block bits	
				block_size = 1 << strtol(optarg, NULL, 10); // = Big B 
				break;
			case 't':
				// specify the trace filename
				trace_filename = optarg;
				break;
			case '?':
			default:
				usage(argv[0]);
				exit(1);
		}
	}

	if (verbose_mode) {
		printf("Verbose mode enabled.\n");
		printf("Trace filename: %s\n", trace_filename);
		printf("Number of sets: %d\n", num_sets);
	}
	
	simulateCache(trace_filename, num_sets, block_size, lines_per_set, verbose_mode);
    return 0;
}

/**
 * Simulates cache with the specified organization (S, E, B) on the given
 * trace file.
 *
 * @param trace_file Name of the file with the memory addresses.
 * @param num_sets Number of sets in the simulator.
 * @param block_size Number of bytes in each cache block.
 * @param lines_per_set Number of lines in each cache set.
 * @param verbose Whether to print out extra information about what the
 *   simulator is doing (1 = yes, 0 = no).
 */
void simulateCache(char *trace_file, int num_sets, int block_size, 
						int lines_per_set, int verbose) {
	// Variables to track how many hits, misses, and evictions we've had so
	// far during simulation.
	int hit_count = 0;
	int miss_count = 0;
	int eviction_count = 0;

	// Variables for reading in the lines	
	mem_addr addr = 0;
	int size;
	char instr[2];

	Cache *cache = malloc(sizeof(Cache));
	createCache(cache, num_sets, lines_per_set);
	
	mem_addr tag = 0;
	int set_index = 0;
	int LRU = lines_per_set;	

	FILE *fp = openFile(trace_file);
	while(fscanf(fp, "%s %lx,%d", instr, &addr, &size) == 3) {
		if(instr[0] == 'I') {
			continue;
		}
		
		getLineInfo(addr, &tag, &set_index, block_size, num_sets);
		switch(instr[0]) {
			case 'L':
				cacheOp(cache, lines_per_set, set_index, tag, 
						&LRU, &hit_count, &miss_count, &eviction_count);
				break;
			case 'S':
				cacheOp(cache, lines_per_set, set_index, tag, 
						&LRU, &hit_count, &miss_count, &eviction_count);
				break;
			case 'M':
				for (int i = 0; i < 2; i++, 
					cacheOp(cache, lines_per_set, set_index, tag, 
						&LRU, &hit_count, &miss_count, &eviction_count));
				break;
			default:
				break;
		}
	}	
    printSummary(hit_count, miss_count, eviction_count);
	fclose(fp);
	freeCache(cache, num_sets, lines_per_set);
}
void cacheOp(Cache *cache, int lines_per_set, int set_index, int tag, int *LRU, int *hit_count, int *miss_count, int *eviction_count) {  
		int hit = 0;
		int miss = 0;
		int eviction = 0;
		for (int i = 0; i < lines_per_set; i++) {
			if (cache->sets[set_index].lines[i].valid == 1) {
				if (cache->sets[set_index].lines[i].tag == tag) {
					hit += 1;
					*hit_count += 1;
					cache->sets[set_index].lines[i].lru = *LRU;
					*LRU += 1;
					break;
				}
			}
		}
		if (hit < 1 ) {
			miss += 1;
			*miss_count += 1;
			int min = INT_MAX;
			int to_evict = 0;
			for (int i = 0; i < lines_per_set; i++) {
				if (cache->sets[set_index].lines[i].lru < min) {
					min = cache->sets[set_index].lines[i].lru;
					to_evict = i;
				}
			}
			if (cache->sets[set_index].lines[to_evict].valid == 1) {
				eviction += 1;
				*eviction_count += 1;
			}
			cache->sets[set_index].lines[to_evict].tag = tag;
			cache->sets[set_index].lines[to_evict].valid = 1;
			cache->sets[set_index].lines[to_evict].lru = *LRU;
			*LRU += 1;
		}
}

/**
 * Gets the tag, set_index and block offset of the valgrind instruction.
 *
 * @param addr Read in from the input file. Preform instruciton on this
 * address.
 * @param *tag A pointer to the tag of the current address.
 * @param *set_index A pointer to specify which set of the cache the address
 * is located in.
 */ 
void getLineInfo(mem_addr addr, mem_addr *tag, int *set_index, 
		int block_size, int num_sets) {
	int block_bits = log(block_size) / log(2);
	int set_bits = log(num_sets) / log(2);
	int tag_bits = 64 - (block_bits + set_bits);

	*tag = addr >> (set_bits + block_bits);
	mem_addr temp = addr << tag_bits;
	*set_index = temp >> (tag_bits + block_bits);
	temp = addr << (tag_bits + set_bits);
}

/**
 * Dynamically creates a cache with the specified organization (S, E, B)
 * from the given inputs.
 *
 * @param cache A pointer to a Cache struct.
 * @param num_sets Number of sets in the simulator.
 * @param block_size Number of bytes in each cache block.
 * @param lines_per_set Number of lines in each cache set.
 */
void createCache(Cache *cache, int num_sets, int lines_per_set) {
	cache->sets = calloc(num_sets, sizeof(Set));
	for (int i = 0; i < num_sets; i++) {
		cache->sets[i].lines = calloc(lines_per_set, sizeof(Line));
		for (int j = 0; j < lines_per_set; j++) {
			cache->sets[i].lines[j].lru = j;
		}	
	}
}

/**
 * Frees all allocated memory to the cache and the lines within.  
 *
 * @param *cache A pointer to the Cache struct. 
 * @param num_sets Number of sets in the simulator.
 * @param lines_per_set Number of lines in each cache set.  
 */
void freeCache(Cache *cache, int num_sets, int lines_per_set) {
	for (int i = 0; i < num_sets; i++) {
		free(cache->sets[i].lines);
	}
	free(cache->sets);
	free(cache);
}

/**
 * Open a file and return a poniter to it.
 * Has logic to exit program if file does not exist.
 *
 * @param trace_file The file to read/write from.
 */
FILE* openFile(char* trace_file) {
	FILE *fp = fopen(trace_file, "r");
  	if ((fp) == NULL) {
		printf("No such file\n");
		exit(1);
		return fp;
	}
	printf("trace file read\n");	
	return fp;
}
