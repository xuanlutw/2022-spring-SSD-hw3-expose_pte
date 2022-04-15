all: test hw3-test hw3-exploit hw3-sheep
test: test.c
	aarch64-linux-gnu-gcc -Wall -static test.c -o test
hw3-test: hw3-test.c
	aarch64-linux-gnu-gcc -Wall -static hw3-test.c -o hw3-test
hw3-exploit: hw3-exploit.c
	aarch64-linux-gnu-gcc -Wall -static hw3-exploit.c -o hw3-exploit
hw3-sheep: hw3-sheep.c
	aarch64-linux-gnu-gcc -Wall -static hw3-sheep.c -o hw3-sheep
clean:
	rm test hw3-test hw3-sheep hw3-exploit
