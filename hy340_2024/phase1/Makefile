all: scanner.c
	flex --outfile=scanner.c scanner.l
	gcc  scanner.c -o scanner

clean:
	rm scanner scanner.c