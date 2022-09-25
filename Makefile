# Implementace překladače imperativního jazyka IFJ22
# Authors: Jiří Gallo (xgallo04)

CC := gcc
CFLAGS := -Wall -g

all: ifj22
test: all run_test

ifj22: ./*
	$(CC) main.c emitter.c -o ifj22

tester: ifj22 ./* tests/*
	g++ -std=c++17 tests/test.cpp -o tester


run_test: tester
	./tester

clean:
	rm -f ./ifj22
	rm -f ./tester
	rm -f ./*.zip

pack: clean
	zip -r xlogin99.zip . -x "ifj2022.pdf" -x "tests/*" -x ".git/*"
