# Implementace překladače imperativního jazyka IFJ22
# Authors: Jiří Gallo (xgallo04)

CC := gcc
CFLAGS := -Wall -g

all: ifj22
test: all run_test

ifj22: Makefile *.c *.h
	$(CC) $(CFLAGS) main.c emitter.c lexer.c lexer_processor.c parser.c symtable.c ast.c string_builder.c optimizer.c pointer_hashtable.c code_generator.c -o ifj22

tester: ifj22 ./* tests/*
	g++ -std=c++17 tests/test.cpp -o tester


run_test: tester
	./tester

clean:
	rm -f ./ifj22
	rm -f ./tester
	rm -f ./*.zip
	rm -frd ./doc/Doxygen

pack: clean
	zip -r xzajic22.zip . -x "ifj2022.pdf" -x "tests/*" -x ".git*" -x "doc/*" -x "expression.*" -x "stack.*" -x "README.*" -x "*.sh" -x "temp/*"

docs:
	@doxygen ./Doxyfile
