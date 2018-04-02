/*
 * CS351: System Programming
 * Machine Problem #4 :  Cache Simulation and Optimization
 * Jay Patel | A20328510
 * */

#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

//Creating a 64 bit address
typedef unsigned long long maddr_t;

typedef struct {
	int valid;	 
 	maddr_t tag;
	int timestamp;
} line;

typedef struct {
	line *lines;
} set;

typedef struct {
	set *sets;
} cache_t;

void parse_input(int argc, char **argv);
void read_exec_tracefile(); 
int evict_index(int idx); 
int empty_index(int idx);
void simulate_cmd(maddr_t addr); 
cache_t build_cache(int S, int E, int B); 
void clear_cache(cache_t cache_sim);
void print_usage(char **argv);

// Setting Up the Cache Parameters
// set index bits
int s;
// associativity (number of lines in a set)		 
int E;
// block bits		 
int b;	
// number of sets (2**s)	 
int S;
// size of block (2**b)		 
int B;	
//	hit count	 
int h_count; 
// miss count
int m_count; 
 // eviction count
int e_count;

// Tracefile
char *t;
//global Time
int time;

int verbose;


// the actual cache simulation
cache_t cache_sim; 




int main(int argc, char **argv) {
   	// parse the cmdline
	parse_input(argc, argv); 
	// build the cache
    cache_sim = build_cache(S, E, B);
	//initialize parameters
	h_count = 0; 
	m_count = 0; 
	e_count = 0; 
	time = 0;
	// read the trace file and execute simulation
	read_exec_tracefile();
	//clear_cache(&cache_sim);   
	printSummary(h_count, m_count, e_count);
    
	return 0;
}

void read_exec_tracefile() {
	
	FILE *tracefile;
	char command;
	maddr_t addr;
	int size;
	tracefile = fopen(t, "r");
	if(tracefile != NULL) {
		while(fscanf(tracefile, " %c %llx,%d", &command, &addr, &size) == 3) {
			switch(command) {
				case 'I': 
					break;
				case 'L':
					if(verbose) printf("%c %llx,%d ", command, addr, size);
					simulate_cmd(addr);
					if (verbose) printf("\n");
					break;
				case 'S':
					if(verbose) printf("%c %llx,%d ", command, addr, size);
					simulate_cmd(addr);
					if (verbose) printf("\n");
					break;
				case 'M':
					if(verbose) printf("%c %llx,%d ", command, addr, size);
					simulate_cmd(addr);
					
					//simulate_cmd(cache_sim, addr);
					h_count += 1; if(verbose) printf("hit\n"); //Hit is inevitable
					break;
			}
			
		}
	}

}

void simulate_cmd(maddr_t addr) {
	maddr_t mem_tag = addr >> (s + b);
	int idx = (unsigned int) ((addr << (64 - (s + b))) >> (64 - s));
	int full = 1;
	set set_curr = cache_sim.sets[idx];
	int curr_hits = h_count;

	//check address
	int li;
	for(li = 0; li < E; li++) {
		line line_curr = set_curr.lines[li];
		if(line_curr.valid && line_curr.tag == mem_tag) {
			set_curr.lines[li].timestamp = ++time;
			h_count++;
			if(verbose) printf("hit ");
		} else if(!(line_curr.valid)) {
			full = 0;
		}
	}
	
	//check if hit, if not update miss count
	if(h_count > curr_hits)
		return;
	else {
		m_count ++;
		if(verbose) printf("miss ");
	}

	
	if(full == 1) {
		
		e_count++;
		if(verbose) printf("eviction ");	
		int lru_idx = evict_index(idx);
		set_curr.lines[lru_idx].tag = mem_tag;
		set_curr.lines[lru_idx].timestamp = ++time;
	} else {
		
		int emt_idx = empty_index(idx);
		set_curr.lines[emt_idx].tag = mem_tag;
		set_curr.lines[emt_idx].valid = 1;
		set_curr.lines[emt_idx].timestamp = ++time;
	}  
}

int evict_index(int idx) {
	int min_idx = 0;
	int li;
	for(li = 0; li < E; li++) {
		
		if(cache_sim.sets[idx].lines[li].timestamp < cache_sim.sets[idx].lines[min_idx].timestamp) {
			min_idx = li;
		}
	}
	return min_idx;
}

int empty_index(int idx) {
	int li;
	for(li = 0; li < E; li++) {
		if(!cache_sim.sets[idx].lines[li].valid) 
			return li;
	}
	return -1;
}

cache_t build_cache(int S, int E, int B) {
	cache_t cache_sim;
	set cache_set;
	line cache_line;

	int si, li;
	
	// allocate sets
	cache_sim.sets = (set *) calloc(S, sizeof(set)); 
	
	for(si = 0; si < S; si++) {
		// allocate lines for each set
		cache_set.lines = (line *) calloc(E, sizeof(line)); 
		for(li = 0; li < E; li++) {
			cache_line.tag = 0;
			cache_line.valid = 0;
			cache_line.timestamp = 0;
			cache_set.lines[li] = cache_line;
		}
		cache_sim.sets[si] = cache_set;
	} 
	return cache_sim; 
}

void clear_cache(cache_t cache_sim) {
	int si;
	for(si = 0; s < S; si++) {
		// free all lines in the set
		free(cache_sim.sets[si].lines); 
	}	
	// free all sets in simulated cache
	free(cache_sim.sets); 
}

void parse_input(int argc, char **argv) {
	char flag;
	while((flag = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
		switch(flag) {
			case 's':
				s = atoi(optarg);
				S = (int)pow(2, s);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				B = (int)pow(2, b);
				break;
			case 't':
				t = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'h':
				print_usage(argv);
				exit(0);
			default:
				print_usage(argv);
				exit(1);
		}
	}	
}

void print_usage(char **argv){
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Help Message.\n");
    printf("  -v         Optional Verbose Flag.\n");
    printf("  -s <num>   Number of Set Index Bits.\n");
    printf("  -E <num>   Number of Lines Per Set.\n");
    printf("  -b <num>   Number of Block Offset Bits.\n");
    printf("  -t <file>  Trace File.\n");
    printf("\nExamples:\n");
    printf("  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

















