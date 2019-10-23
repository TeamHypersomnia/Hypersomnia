import sys
import os

cpp_file = sys.argv[1]

with open(cpp_file) as f:
    lines = f.readlines()

processed_lines = []

for l in lines:
    out_line = l.split()
    try:
        c0 = int(out_line[0])
        c1 = int(out_line[1])
    except ValueError:
        pass
    else:
        del out_line[0]
        out_line[0] = "{}".format(c1 - c0)
        print(" ".join(out_line))

