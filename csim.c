/*	
 * csim.c
 *
 * Fill in file header comment with your name(s) and a short paragraph about
 * what your program does.
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
void simulateCache(char *trace_file, int num_sets, int block_size, int lines_per_set, int verbose);
void createCache(Cache *cache, int num_sets, int lines_per_set);
void freeCache(Cache *cache, int num_sets, int block_seize, int lines_per_set); 

void getLineInfo(mem_addr addr, mem_addr *tag, int *set_index, int *block, int block_size, int lines_per_set);
void checkIfValid(Cache *cache, int *hit_count, int *hit, int set_index, int *LRU, int lines_per_set, int tag);
void caseIfNotValid(Cache *cache, int *miss_count, int *eviction_count, int *eviction, int *LRU, int lines_per_set, int set_index, int tag);

void printVerbose(int verbose, char instr, mem_addr addr, int size, int hit, int eviction); 
FILE* openFile(char* trace_file);

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
	int num_sets = 2;
	int lines_per_set = 1;
	int block_size = 1;
	char *trace_filename = NULL;
	
	opterr = 0;
	// TODO: update this to support the h, b, and E options
	int c = -1;

	// Note: adding a colon after the letter states that this option should be
	// followed by an additional value (e.g. "-s 1")
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
				// Note: optarg is set by getopt to the string that follows
				// this option (e.g. "-s 2" would assign optarg to the string "2")
				num_sets = 1 << strtol(optarg, NULL, 10); // = Big S
				break;
			case 'E':
				// specify number of lines per set		
				//lines_per_set = strtol(optarg, NULL, 10);
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

	// TODO: When you are ready to start using the user defined options, you
	// should add some code here that makes sure they have actually specified
	// the options that are required (e.g. -t and -s are both required).

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
	char instr;
	mem_addr addr = 0;
	int size;

	// TODO: This is where you will fill in the code to perform the actual
	// cache simulation. Make sure you split your work into multiple functions
	// so that each function is as simple as possible.
	
	// Dynamically allocate space for the Cache object 
	Cache *cache = malloc(sizeof(Cache));
	createCache(cache, num_sets, lines_per_set);
	
	int hit = 0;
	//int empty = -1;
	int eviction = 0;
	mem_addr tag = 0;
	int set_index = 0;
	int block = 0;
	int LRU = lines_per_set;	

	FILE *fp = openFile(trace_file);
	while(fscanf(fp, " %c %lx,%d", &instr, &addr, &size) == 3) {
		if(instr == 'I')
			continue;

		getLineInfo(addr, &tag, &set_index, &block, block_size, lines_per_set);

		switch(instr) {
			case 'L':
				//call onece 
				break;
			case 'S':
				//call once
				break;
			case 'M':
				//call twice
				break;
			default:
				break;
		}
		if (instr == 'L') {

		}

		if (instr == 'S') {

		}

		if (instr == 'M') {

		}
		// get the set index, block offset, and tag
		checkIfValid(cache, &hit_count, &hit, set_index, &LRU, lines_per_set, tag);

		//not valid, go by LRU
		
		if (hit == 0) {
			caseIfNotValid(cache, &miss_count, &eviction_count, &eviction, &LRU, lines_per_set, set_index, tag);
		}
		if (instr == 'M') {
			checkIfValid(cache, &hit_count, &hit, set_index, &LRU, lines_per_set, tag);
		}

		printVerbose(verbose, instr, addr, size, hit, eviction);
		//empty = -1;
		hit = 0;
		eviction = 0;
	}	
	//printf("%lu\n", sizeof(cache));
    printSummary(hit_count, miss_count, eviction_count);
	fclose(fp);
	freeCache(cache, num_sets, block_size, lines_per_set);
}

void caseIfNotValid(Cache *cache, int *miss_count, int *eviction_count, int *eviction, int *LRU, int lines_per_set, int set_index, int tag){
	int lru_index = 0; //index to evict  
	int min = cache->sets[set_index].lines[0].lru; 
	for (int i = 1; i < lines_per_set; i++) {
		if (cache->sets[set_index].lines[i].lru < min) {
			min = cache->sets[set_index].lines[i].lru; 
			lru_index = i;
		}
	}	
	if (cache->sets[set_index].lines[lru_index].valid == 1) {
		*eviction_count += 1;
		*eviction = 1;
	}
	cache->sets[set_index].lines[lru_index].tag = tag;
	cache->sets[set_index].lines[lru_index].lru = *LRU;
	cache->sets[set_index].lines[lru_index].valid = 1;
	*miss_count += 1;
	*LRU += 1;
}

void checkIfValid(Cache *cache, int *hit_count, int *hit, int set_index, int *LRU, int lines_per_set, int tag){
	for (int i = 0; i < lines_per_set; i++) {
		if (cache->sets[set_index].lines[i].valid == 1) {
			if (cache->sets[set_index].lines[i].tag == tag) {
				*hit_count += 1;
				*hit = 1;
				cache->sets[set_index].lines[i].lru = *LRU;
				*LRU++ += 1;
				return;
			}
		}
	}
}

void getLineInfo(mem_addr addr, mem_addr *tag, int *set_index, int *block, int block_size, int lines_per_set) {
	int block_bits = log(block_size) / log(2);
	int set_bits = log(lines_per_set) / log(2);
	int tag_bits = 64 - (block_bits + set_bits);

	//claculate the tag
	*tag = addr >> (set_bits + block_bits);
	int temp = addr << tag_bits;
	*set_index = temp >> (tag_bits + block_bits);
	//*set_index = (addr << tag_bits) >> (tag_bits + block_bits);
	temp = addr << (tag_bits + set_bits);
	*block = temp >> (tag_bits + set_bits);
	//*block = (addr << tag_bits + set_bits) >> (tag_bits + set_bits);
}

/**
 * Dynamically creates a cache with the specified organization (S, E, B)
 * from the given inputs
 *
 * @param cache A pointer to a Cache struct
 * @param num_sets Number of sets in the simulator.
 * @param block_size Number of bytes in each cache block.
 * @param lines_per_set Number of lines in each cache set.
 */
void createCache(Cache *cache, int num_sets, int lines_per_set) {
	cache->sets = calloc(num_sets, sizeof(Set));
	for (int i = 0; i < num_sets; i++) {
		cache->sets[i].lines = calloc(lines_per_set, sizeof(Line));
		for (int j = 0; j < lines_per_set; j++) {
			//set up lru super arbitrarily
			cache->sets[i].lines[j].lru = j;
			cache->sets[i].lines[j].tag = 0;
		}	
	}
}

void freeCache(Cache *cache, int num_sets, int block_seize, int lines_per_set) {
	for (int i = 0; i < num_sets; i++) {
		free(cache->sets[i].lines);
	}
	free(cache->sets);
	free(cache);
}
/**
 * Open a file and return a poniter to it
 * Has logic to exit program if file does not exist
 *
 * @param trace_file The file to read/write from
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

void printVerbose(int verbose, char instr, mem_addr addr, int size, int hit, int eviction) {
	if (verbose != 1) {
		return;
	}
	printf("%c %lx,%d ", instr, addr, size);
	if (hit == 1) {
	   printf("hit ");
   	}
	else {
		printf("miss ");
   	}
	if (eviction == 1) {
	   	printf("eviction ");
   	}
	printf("\n");
}	
