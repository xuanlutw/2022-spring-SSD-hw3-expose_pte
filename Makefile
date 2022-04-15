CC = aarch64-linux-gnu-gcc
CFLAGS = -Wall -static

all: test hw3-test hw3-exploit hw3-sheep

test: test.c expose_pte.c
	$(CC) $(CFLAGS) test.c expose_pte.c -o test
hw3-test: hw3-test.c
	$(CC) $(CFLAGS) hw3-test.c -o hw3-test
hw3-exploit: hw3-exploit.c expose_pte.c
	$(CC) $(CFLAGS) hw3-exploit.c expose_pte.c -o hw3-exploit
hw3-sheep: hw3-sheep.c
	$(CC) $(CFLAGS) hw3-sheep.c -o hw3-sheep

clean:
	rm test hw3-test hw3-sheep hw3-exploit
