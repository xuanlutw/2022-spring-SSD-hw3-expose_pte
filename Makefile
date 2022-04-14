all: test
test: test.c
	aarch64-linux-gnu-gcc -Wall -static test.c -o test
