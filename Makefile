.PHONY: all

all: cfr testcf

cfr: cfr.c
	gcc -O2 -o $@ $^
	strip $@

testcf: testcf.c cf.c homo.c bihomo.c approx.c numbs.c gcf.c gendec.c gmp.c
	gcc -g -o $@ $^ -lgmp
