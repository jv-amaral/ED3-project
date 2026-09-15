all:
	gcc -Wall -Wextra -g *.c -o programaTrab

run:
	./programaTrab

clean:
	rm -f programaTrab