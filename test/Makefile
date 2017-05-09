#
#
#

all: tests

#

PRJ_DIR ?= ..

VPATH = $(PRJ_DIR)/common:$(PRJ_DIR)/test

CFLAGS += -DWITH_JSON_CONFIG
CFLAGS += -I$(PRJ_DIR)/include -I$(PRJ_DIR)/test

LIBS = -ljson-c

#

TEST1_SRCS := test_conf.c config.c
TEST1_OBJS := $(TEST1_SRCS:.c=.o)

test1: $(TEST1_OBJS)
	$(CC) $(TEST1_OBJS) $(LIBS) -o $@

#

tests: test1

#

clean:
	rm -rf *.o
	rm test1

