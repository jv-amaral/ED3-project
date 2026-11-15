all:
	gcc -Wall -Wextra -g *.c auxiliares/*.c -o programaTrab

run:
	./programaTrab

clean:
	rm -f programaTrab
