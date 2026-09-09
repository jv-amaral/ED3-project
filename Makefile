all:
	gcc -Wall -Wextra -g main.c funcoes.c -o programaTrab

run:
	./programaTrab

clean:
	rm -f programaTrab