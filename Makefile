cfr: cfr.c
	gcc -O2 -o $@ $^
	strip $@
