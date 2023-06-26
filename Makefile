all:
	gcc -Wall main.c util.c lexer.c parser.c -o main
clean:
	rm main a.s out.o
