import re, sys

pattern = re.compile("default\s+=\s+\$(\d+)")

for i, line in enumerate(open(sys.argv[1])):
    for match in re.finditer(pattern, line):
        print ("0x" + match.groups()[0])