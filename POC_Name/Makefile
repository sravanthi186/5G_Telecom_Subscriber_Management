CC = gcc
CFLAGS = -std=c11 -O0 -g -Wall -Wextra -Wpedantic -D_XOPEN_SOURCE=700
CPPFLAGS = -Iinclude
LDLIBS = -pthread -lcrypto
CORE = src/module1.c src/module2.c src/lifecycle.c src/utils.c
APP = $(CORE) src/auth.c src/benchmark.c src/concurrency.c src/main.c

all: build/telecom_poc

build/telecom_poc: $(APP) include/common.h include/module1.h include/module2.h
	@mkdir -p build data
	$(CC) $(CPPFLAGS) $(CFLAGS) $(APP) -o $@ $(LDLIBS)

test: build/test_module1 build/test_module2 build/test_auth build/test_lifecycle
	./build/test_module1
	./build/test_module2
	./build/test_auth
	./build/test_lifecycle

build/test_module1: $(CORE) test/test_module1.c
	@mkdir -p build data
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) test/test_module1.c -o $@ -pthread

build/test_module2: $(CORE) test/test_module2.c
	@mkdir -p build data
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) test/test_module2.c -o $@ -pthread

build/test_auth: src/auth.c test/test_auth.c
	@mkdir -p build data
	$(CC) $(CPPFLAGS) $(CFLAGS) src/auth.c test/test_auth.c -o $@ -lcrypto

coverage:
	@mkdir -p build data
	$(CC) $(CPPFLAGS) -std=c11 -O0 -g --coverage -D_XOPEN_SOURCE=700 $(CORE) test/test_module1.c -o build/test_coverage -pthread
	./build/test_coverage
	gcov -b -c -o build build/test_coverage-module1.gcno build/test_coverage-module2.gcno build/test_coverage-utils.gcno

clean:
	rm -f build/telecom_poc build/test_module1 build/test_module2 build/test_auth build/test_lifecycle build/test_coverage build/*.gcda build/*.gcno *.gcov
.PHONY: all test coverage clean

build/test_lifecycle: $(CORE) test/test_lifecycle.c
	@mkdir -p build data
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) test/test_lifecycle.c -o $@ -pthread -lm

gtest:
	@echo "Google Test requires libgtest-dev / installed GTest and a C++ compiler."
	@mkdir -p build data
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/module1.c -o build/gt_module1.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/module2.c -o build/gt_module2.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/lifecycle.c -o build/gt_lifecycle.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/auth.c -o build/gt_auth.o
	g++ -std=c++17 -g -Iinclude test/test_all_gtest.cpp build/gt_module1.o build/gt_module2.o build/gt_lifecycle.o build/gt_auth.o -lgtest -lgtest_main -pthread -lcrypto -o build/telecom_gtest
	./build/telecom_gtest
.PHONY: gtest
