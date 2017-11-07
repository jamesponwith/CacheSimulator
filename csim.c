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
#include "cachelab.h"

//typedef struct Line Line;
typedef unsigned long int mem_addr;

typedef struct {
	unsigned int valid;
	mem_addr tag;
} Line;

typedef struct {
	Line* lines;
} Set;

typedef struct {
	Set* sets;
} Cache;

// forward declaration
void simulateCache(char *trace_file, int num_sets, int block_size, int lines_per_set, int verbose);
void createCache(Cache *cache, int num_sets, int block_size, int lines_per_set);
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
				num_sets = 1 << strtol(optarg, NULL, 10);
				break;
			case 'E':
				// specify number of lines per set		
				//lines_per_set = strtol(optarg, NULL, 10);
				break;
			case 'b':
				// specify the number of block bits	
				//block_size = 1 << strtol(optarg, NULL, 10);
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
	
	//simulateCache(trace_filename, num_sets, 2, li1, verbose_mode);
	simulateCache(trace_filename, num_sets, block_size, lines_per_set, verbose_mode);
	/**
	 * Sets: 2
	 * Lines: 1
	 * Block: 2
	 */
	//simulateCache(trace_filename, 2, 2, 1, verbose_mode);
	
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
	int size;
	char instr;
	mem_addr addr;

	// TODO: This is where you will fill in the code to perform the actual
	// cache simulation. Make sure you split your work into multiple functions
	// so that each function is as simple as possible.
	printf("Set Index Bits %d\n", num_sets);
	printf("Block Size %d\n", block_size);
	printf("Lines Per Set %d\n", lines_per_set);

	Cache *cache = malloc(sizeof(Cache));
	createCache(cache, num_sets, block_size, lines_per_set);

	FILE *fp = openFile(trace_file);
	
	while(fscanf(fp, " %c %lx,%d", &instr, &addr, &size) == 3) {
	}	
	
	printf("%lu\n", sizeof(cache));
	free(cache);
    printSummary(hit_count, miss_count, eviction_count);
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
	return fp;
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
void createCache(Cache *cache, int num_sets, int block_size, int lines_per_set) {
	//cache->sets = malloc(num_sets * sizeof(Set));
	
	cache->sets = calloc(num_sets, sizeof(Set));
	for (int i = 0; i < num_sets; i++) {
		//cache->sets[i].lines = malloc(lines_per_set * sizeof(Line));
		cache->sets[i].lines = calloc(lines_per_set, sizeof(Line));
	}
}
