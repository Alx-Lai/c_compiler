all:
	gcc -Wall main.c lexer.c util.c -o main
clean:
	rm main a.s out.o
