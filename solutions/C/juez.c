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

	/**************************************************************
	*                  Do files differ in size?                  *
	**************************************************************/
	unsigned long size;
	{
		struct stat f1s;
		struct stat f2s;

		if (stat(argv[1], &f1s) < 0) { eexit("Cannot stat first file"); }
		if (stat(argv[2], &f2s) < 0) { eexit("Cannot stat second file"); }

		if (f1s.st_size != f2s.st_size) return EXIT_DIFF;
		size = f1s.st_size;
	}
	if (size == 0) return EXIT_EQUAL;  // Corner case

	/**************************************************************
	*                Do files differ in content?                 *
	**************************************************************/
	int fh1 = open(argv[1], O_RDWR, 0);
	if (fh1 < 0) { eexit("Cannot open first file"); }
	void *f1 = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fh1, 0);
	if (f1 == MAP_FAILED) { eexit("Cannot map first file"); }

	int fh2 = open(argv[2], O_RDWR, 0);
	if (fh2 < 0) { eexit("Cannot open second file"); }
	void *f2 = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fh2, 0);
	if (f2 == MAP_FAILED) { eexit("Cannot map second file"); }

	// One thread per processor core and one file chunk per thread
	omp_set_num_threads(omp_get_num_procs());
	size_t chunk = size/omp_get_num_procs();
	if (chunk == 0) chunk = size;

	#pragma omp parallel for
	for (size_t s=0; s<size; s+=chunk) {
		int offset = s+chunk > size ? chunk-((s+chunk)-size) : chunk;

		// Force exit if the current chunk differs
		if (memcmp(f1+s, f2+s, offset) != 0) _exit(EXIT_DIFF);
	}

	// Both files are equal!
	return EXIT_EQUAL;
}
