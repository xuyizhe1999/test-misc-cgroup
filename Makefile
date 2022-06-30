test : create.c test.c test.h
	gcc test.c create.c -o test

clean :
	rm test log
