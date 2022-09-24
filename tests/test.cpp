// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <filesystem>

TEST_CASE("Verify that required files exists") {
	using namespace std::filesystem;
	CHECK(exists("./Makefile"));
	CHECK(exists("./symtable.c"));
	CHECK(exists("./dokumentace.pdf"));
	CHECK(exists("./rozdeleni"));
}
