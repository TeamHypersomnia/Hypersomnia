import sys
import os

cpp_file = sys.argv[1]
object_path = "CMakeFiles/Hypersomnia.dir/{}.o".format(cpp_file)

i=0

with open(cpp_file) as f:
    lines = f.readlines()

with open(cpp_file, 'w') as f:
    for line in lines:
        f.write(line)
