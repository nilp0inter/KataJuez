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
		struct stat fs1;
		struct stat fs2;

		if (stat(argv[1], &fs1) < 0) { eexit("Cannot stat first file"); }
		if (stat(argv[2], &fs2) < 0) { eexit("Cannot stat second file"); }

		if (fs1.st_size != fs2.st_size) return EXIT_DIFF;
		size = fs1.st_size;
	}
	if (size == 0) return EXIT_EQUAL;  // Corner case

	/**************************************************************
	*                Do files differ in content?                 *
	**************************************************************/
	int fd1 = open(argv[1], O_RDWR, 0);
	if (fd1 < 0) { eexit("Cannot open first file"); }
	void *fm1 = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd1, 0);
	if (fm1 == MAP_FAILED) { eexit("Cannot map first file"); }

	int fd2 = open(argv[2], O_RDWR, 0);
	if (fd2 < 0) { eexit("Cannot open second file"); }
	void *fm2 = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd2, 0);
	if (fm2 == MAP_FAILED) { eexit("Cannot map second file"); }

	// One thread per processor core and one file chunk per thread
	omp_set_num_threads(omp_get_num_procs());
	size_t page = getpagesize();
	size_t chunk = size < page ? page : size/omp_get_num_procs();

	#pragma omp parallel for
	for (size_t s=0; s<size; s+=chunk) {
		size_t offset = s+chunk > size ? chunk-((s+chunk)-size) : chunk;

		// Force exit if the current chunk differs
		if (memcmp(fm1+s, fm2+s, offset) != 0) _exit(EXIT_DIFF);
	}

	// Both files are equal!
	return EXIT_EQUAL;
}
