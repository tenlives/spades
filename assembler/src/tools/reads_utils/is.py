#!/usr/bin/python -O

import sys

inFile = open(sys.argv[1])
outFile = open(sys.argv[2], 'w')

prevLine = ""
for line in inFile:
	if (prevLine == ""):
		prevLine = line
		continue

	if (line.split('/', 1)[0] == prevLine.split('/', 1)[0]):
		pos2 = int(line.split('\t', 4)[3])
		pos1 = int(prevLine.split('\t', 4)[3])
		outFile.write(str(pos2 - pos1))
		outFile.write('\n')
	else:
		print("Non-equal pairs\n")
		print(prevLine.split('/', 1)[0])
		print(line.split('/', 1)[0])

	prevLine = ""


inFile.close()
outFile.close()
