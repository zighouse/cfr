.PHONY: all

all: cfr testcf

cfr: cfr.c
	gcc -O2 -o $@ $^
	strip $@

testcf: testcf.c cf.c homo.c bihomo.c simp.c numbs.c gcf.c
	gcc -g -o $@ $^
