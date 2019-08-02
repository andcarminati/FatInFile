all: test

test: test.o fs.o list.o
	gcc -g test.o fs.o list.o -o test

test.o: test.c fat.h
	gcc -g -c test.c

fs.o: fs.c fs.h fat.h list.h
	gcc -g -c fs.c

list.o: list.c list.h
	gcc -g -c list.c

clean:
	/bin/rm -f test fs.o fs.o list.o fs.dat