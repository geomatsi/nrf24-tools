#
#
#

all: run_tests

#

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++

#

PRJ_DIR ?= ..

VPATH = $(PRJ_DIR)/common:$(PRJ_DIR)/test

CPPFLAGS += -DWITH_JSON_CONFIG
CPPFLAGS += -I$(PRJ_DIR)/include -I$(PRJ_DIR)/test

LIBS = -ljson-c -lCppUTest -lCppUTestExt

#

TEST_OBJS := main_test.o conf_tests.o config.o

test: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) $(LIBS) -o $@

#

tests: test

#

run_tests: tests
	./test -c

#

clean:
	rm -rf *.o
	rm test

