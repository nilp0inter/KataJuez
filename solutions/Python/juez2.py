#!/usr/bin/env python
from concurrent.futures import ProcessPoolExecutor, wait, FIRST_EXCEPTION
from itertools import zip_longest
from os.path import getsize
import multiprocessing
import mmap
import sys


EXIT_EQUAL = 0
EXIT_DIFF = 1
EXIT_FAIL = 255


def compare_chunk(fd1, fd2, start, end, finish):
    map1 = mmap.mmap(fd1, 0, prot=mmap.PROT_READ)
    map2 = mmap.mmap(fd2, 0, prot=mmap.PROT_READ)
    for i in range(start, end, mmap.PAGESIZE):
        s = slice(i, min(i + mmap.PAGESIZE, finish))
        if map1[s] != map2[s]:
            raise ValueError('Chunk differs!')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <file1> <file2>")
        sys.exit(EXIT_FAIL)
    elif getsize(sys.argv[1]) != getsize(sys.argv[2]):
        sys.exit(EXIT_DIFF)
    else:
        size = getsize(sys.argv[1])
        with open(sys.argv[1], 'rb') as file1:
            with open(sys.argv[2], 'rb') as file2:
                fd1, fd2 = file1.fileno(), file2.fileno()
                cpu_count = multiprocessing.cpu_count()
                with ProcessPoolExecutor(max_workers=cpu_count) as executor:
                    chunk_size = size // cpu_count
                    comparisons = [
                        executor.submit(compare_chunk,
                                        fd1, fd2,
                                        start, end, size)
                        for start, end in
                        zip_longest(range(0, size, chunk_size),
                                    range(chunk_size, size, chunk_size),
                                    fillvalue=size)]
                    done, not_done = wait(comparisons,
                                          return_when=FIRST_EXCEPTION)
                    try:
                        for future in done:
                            future.result()
                    except ValueError:
                        sys.exit(EXIT_DIFF)
                    else:
                        sys.exit(EXIT_EQUAL)
