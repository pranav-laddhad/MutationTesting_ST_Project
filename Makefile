# Makefile for CUnit Testing

CC = gcc
# The CUNIT_PREFIX is where Homebrew installed CUnit
CUNIT_PREFIX = /opt/homebrew/opt/cunit

# Flags to specify CUnit include and library paths using the prefix
CUNIT_INCLUDE = -I$(CUNIT_PREFIX)/include
CUNIT_LIB_PATH = -L$(CUNIT_PREFIX)/lib

# Compiler flags: Wall/Wextra for warnings, C99 standard, and CUnit includes
CFLAGS = -Wall -Wextra -std=c99 $(CUNIT_INCLUDE)
LDFLAGS = $(CUNIT_LIB_PATH) -lcunit

# Files needed for the test executable
SERVER_OBJS = server.o
TEST_SRC = test_server.c
TEST_EXE = test_runner

# Target to build and run all tests
all: $(TEST_EXE)
	@echo "--- Running CUnit Tests ---"
	./$(TEST_EXE)

# Rule to compile the test runner and link with CUnit
$(TEST_EXE): $(TEST_SRC) $(SERVER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile server.c logic (excluding main function)
server.o: server.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TEST_EXE) *.o books.txt books_temp.txt members.txt members_temp2.txt