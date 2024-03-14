all: al.c
	flex --outfile=al.c al.l
	bison --yacc --defines --output=parser.c parser.y
	gcc  -o calc al.c parser.c symTable.h

clean:
	rm calc
