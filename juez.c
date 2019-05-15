#include <fcntl.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define EXIT_EQUAL 0
#define EXIT_DIFF 1
#define EXIT_FAIL 255

#define eexit(msg) perror(msg); return EXIT_FAIL;


int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
		return EXIT_FAIL;
	}

	int fh1 = open(argv[1], O_RDWR, 0);
	if (fh1 < 0) { eexit("Cannot open first file"); }

	int fh2 = open(argv[2], O_RDWR, 0);
	if (fh2 < 0) { eexit("Cannot open second file"); }

	
	/**************************************************************
	*                  Do files differ in size?                  *
	**************************************************************/
	long f1s = lseek(fh1, 0L, SEEK_END);
	long f2s = lseek(fh2, 0L, SEEK_END);
	if (f1s != f2s) return EXIT_DIFF;


	/**************************************************************
	*                Do files differ in content?                 *
	**************************************************************/
	void *f1 = mmap(NULL, f1s, PROT_READ, MAP_PRIVATE, fh1, 0);
	if (f1 == MAP_FAILED) { eexit("Cannot map first file"); }

	void *f2 = mmap(NULL, f2s, PROT_READ, MAP_PRIVATE, fh2, 0);
	if (f2 == MAP_FAILED) { eexit("Cannot map second file"); }

	// One thread per processor core and one file chunk per thread
	omp_set_num_threads(omp_get_num_procs());
	size_t chunk = f1s/omp_get_num_procs();

	#pragma omp parallel for
	for (size_t s=0; s<f1s; s+=chunk) {
		int offset = s+chunk > f1s ? chunk-((s+chunk)-f1s) : chunk;

		// Force exit if the current chunk differs
		if (memcmp(f1+s, f2+s, offset) != 0) _exit(EXIT_DIFF);
	}

	// Both files are equal!
	return EXIT_EQUAL;
}
