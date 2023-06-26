all:
	gcc -Wall main.c util.c lexer.c parser.c codegen.c -o main
clean:
	rm main a.s out.o
