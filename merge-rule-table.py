#!/usr/bin/python
import gzip
f = gzip.open('rule-table.merged.gz','w')
f1,f2 = open('syntax-rule-table.sorted'),gzip.open('rule-table.gz')
s1,s2 = f1.readline(),f2.readline()
while s1 != '':
	while s1 != '' and s1<=s2:
		print >>f,s1.strip()
		s1 = f1.readline()
	while s2 != '' and s1>s2:
		print >>f,s2.strip()
		s2 = f2.readline()
while s2 != '':
	print >>f,s2.strip()
	s2 = f2.readline()
