test: test.c
	gcc test.c -o test

run: test
	./test

.PHONY: clean
clean: 
	rm -f test
