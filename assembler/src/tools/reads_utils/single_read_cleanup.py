#!/usr/bin/python

import sys
import os

def read_read(infile):
	read = infile.readline()

	if not read:
		return None, None

	id1 = (read.split('/', 1)[0])[1:]

	delim = (read.split('/', 1)[0])[0]

	line = infile.readline()
	while line and line.find(delim) != 0:
		read += line
		line = infile.readline()

	if not line:
		return None, None

	infile.seek(infile.tell() - len(line))
	
	return id1, read
	


if len(sys.argv) < 4:
	print("Usage: " + sys.argv[0] + " <left reads> <right reads> <1st bowtie log> [2nd bowtie log]")	
	sys.exit()

rFileName1 = sys.argv[1]
rFileName2 = sys.argv[2]

rFile1 = open(rFileName1, "r")
rFile2 = open(rFileName2, "r")
blFile1 = open(sys.argv[3], "r")

blFile2 = None
if len(sys.argv) == 5:
	blFile2 = open(sys.argv[4], "r")

ids = []

for line in blFile1:
	ids.append(line.split('/', 1)[0])

if blFile2 is not None:
	for line in blFile2:
		ids.append(line.split('/', 1)[0])


uids = set(ids)

fName1, ext1 = os.path.splitext(rFileName1)
outFile1 = open(fName1 + "_filtered" + ext1, "w") 
fName2, ext2 = os.path.splitext(rFileName2)
outFile2 = open(fName2 + "_filtered" + ext2, "w")

id1, read1 = read_read(rFile1)
id2, read2 = read_read(rFile2)

while id1 is not None and id2 is not None:
	if (id1 != id2):
		print("Unequal ids!: " + id1 + " " + id2)
		sys.exit(0)

	if id1 not in uids:
		outFile1.write(read1)
		outFile2.write(read2)

	id1, read1 = read_read(rFile1)
	id2, read2 = read_read(rFile2)


rFile1.close()
rFile2.close()

outFile1.close()
outFile2.close()

blFile1.close()
if blFile2 is not None:
	blFile2.close()
