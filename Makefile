all:
	gcc -Wall main.c util.c -o main
clean:
	rm main a.s out.o
