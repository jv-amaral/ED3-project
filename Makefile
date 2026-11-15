all:
	gcc -Wall -Wextra -g main.c funcionalidades2.c funcoes_base2.c -o programaTrab

run:
	./programaTrab

clean:
	rm -f programaTrab
