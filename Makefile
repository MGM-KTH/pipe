all:
	gcc -Wall -ansi digenv.c -o digenv

clean:
	rm -rf *.o
	rm digenv
