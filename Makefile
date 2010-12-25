VERSION = 0.1
USE_PTHREADS = 1

TARGET = silm
CC = gcc
CFLAGS_DEBUG = -Wall
CFLAGS_PARAM = -DVERSION=$(VERSION)
CFLAGS_LIB =
CLFAGS_INCLUDE = -include silm.h
CFLAGS = $(CFLAGS_PARAM) $(CLFAGS_INCLUDE) $(CFLAGS_LIB) $(CFLAGS_DEBUG)

all: $(TARGET)
.PHONY: clean test

# Util
clean:
	rm -rf *.o tmp $(TARGET)
test: $(TARGET) ./test/*.sl
	@./$(TARGET) < ./test/core.sl && echo test ok
%.print: %.c
	$(CC) -E $< $(CFLAGS)
TAGS: *.c
	etags *.c

# Main
$(TARGET): $(TARGET).c api.o
	$(CC) -o $@ $(CFLAGS) $(TARGET).c api.o
api.o: api.c local.c local.h tmp/prototype.h tmp/vm.c tmp/vmdata.h
	$(CC) -o $@ $(CFLAGS) -c api.c -include local.h -include tmp/prototype.h

tmp:
	mkdir tmp
tmp/prototype.h: local.c tmp
	@perl -ne '/^(static\W+\w+\W*\*?\w+\W*\(.*\)){/ && print "$$1;\n"' < $< > $@ && echo make $@
tmp/vm.c: vm.c local.c tmp
	./script/gen -vm < $< > $@
tmp/vmdata.h: vm.c local.c tmp
	./script/gen -vmdata < $< > $@
