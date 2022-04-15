all: test hw3-test
test: test.c
	aarch64-linux-gnu-gcc -Wall -static test.c -o test
hw3-test: hw3-test.c
	aarch64-linux-gnu-gcc -Wall -static hw3-test.c -o hw3-test
