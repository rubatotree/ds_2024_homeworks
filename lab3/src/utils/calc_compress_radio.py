import os
import sys

if(len(sys.argv) < 2):
    print("Usage: python calc_compress_radio.py <original> <compressed>")
    sys.exit(1)

original = os.stat(sys.argv[1]).st_size
compressed = os.stat(sys.argv[2]).st_size

if(original == 0):
    print("Compress radio: inf (original file is empty)")
else:
    print("Compress radio: %.2f%%" % (compressed/original*100))