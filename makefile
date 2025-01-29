

all: run

run: vm-fast build.s1
	./vm-fast build.s1

vm-fast: vm-fast.c
	gcc -g vm-fast.c -o vm-fast

