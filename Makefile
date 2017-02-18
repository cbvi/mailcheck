all:
	cc -Wall -O2 -I/usr/local/include/json-c -L/usr/local/lib -ljson-c mailcheck.c -o mailcheck

debug:
	cc -Wall -O0 -ggdb -I/usr/local/include/json-c -L/usr/local/lib -ljson-c mailcheck.c -o mailcheck
