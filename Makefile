create:
	clang control.c

run: create
	./a.out

clean: a.out
	rm -rf testdb
	rm -f a.out


