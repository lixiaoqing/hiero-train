#!/usr/bin/python
import gzip
f = gzip.open('rule-table.merged.gz','w')
f1,f2 = open('rules.c2e.sorted'),gzip.open('extract.sorted.gz')
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
f.close()
f1.close()
f2.close()

f = gzip.open('rule-table.inv.merged.gz','w')
f1,f2 = open('rules.e2c.sorted'),gzip.open('extract.inv.sorted.gz')
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
f.close()
f1.close()
f2.close()
