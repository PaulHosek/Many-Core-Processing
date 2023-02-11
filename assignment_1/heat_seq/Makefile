CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
SRC=$(filter-out $(wildcard ref*.c), $(wildcard *.c))
OBJ=$(patsubst %.c,%.o,$(SRC))

CC=gcc
INCLUDES=-I../../include
OPT=
ifndef DEBUG
CFLAGS=-O2 -std=gnu99
LIB=
else
CFLAGS=-Wall -std=gnu99 -O0
LIB=
endif
ifdef compiler
CC= $(compiler)
CFLAGS:=$(CFLAGS) $(OPT)
endif
ifdef optlevel
OPT= -$(optlevel)
CFLAGS:=$(CFLAGS) $(OPT)
endif
ifdef arch
ARCH=-march=$(arch)
CFLAGS:=$(CFLAGS) $(ARCH)
endif

all: $(PRJ)

$(PRJ): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIB)	

clean:
	-rm -f $(OBJ) $(PRJ)
