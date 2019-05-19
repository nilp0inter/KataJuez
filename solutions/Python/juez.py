#!/usr/bin/env python
from itertools import zip_longest
from os.path import getsize
import sys


CHUNK_SIZE = 1024

EXIT_EQUAL = 0
EXIT_DIFF = 1
EXIT_FAIL = 255


if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <file1> <file2>")
    sys.exit(EXIT_FAIL)
elif getsize(sys.argv[1]) != getsize(sys.argv[2]):
    sys.exit(EXIT_DIFF)
else:
    with open(sys.argv[1], 'rb') as file1:
        with open(sys.argv[2], 'rb') as file2:
            reader1 = iter(lambda: file1.read(CHUNK_SIZE), b'')
            reader2 = iter(lambda: file2.read(CHUNK_SIZE), b'')
            chunks = zip_longest(reader1, reader2)

            files_equal = all(c1 == c2 for c1, c2 in chunks)
            sys.exit(EXIT_EQUAL if files_equal else EXIT_DIFF)
