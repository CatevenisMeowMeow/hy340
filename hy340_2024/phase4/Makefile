all: 
	flex --outfile=al.c al.l
	bison -v --yacc --defines --output=parser.c parser.y
	gcc -g -o calc al.c parser.c sym_table.h -lm

clean:
	rm -rf calc parser.c parser.h parser.output al.c
