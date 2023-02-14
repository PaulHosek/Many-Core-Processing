CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
SRC=$(filter-out $(wildcard ref*.c), $(wildcard *.c))
OBJ=$(patsubst %.c,%.o,$(SRC))

CC=gcc
CFLAGS= -Wall -std=gnu99
LIB=
INCLUDES=-I../../include
OPT=
ARCH=
# Set compiler and optimizations if specified in command line
ifdef compiler
CC=$(compiler)
endif
ifdef optlevel
OPT=-$(optlevel)
CFLAGS:= $(CFLAGS) $(OPT) 
endif
ifdef arch
ARCH=-march=$(arch)
CFLAGS:= $(CFLAGS) $(ARCH)
endif
ifdef autovec
CFLAGS := -O3 -march=haswell -ffast-math 
endif

all: $(PRJ)

$(PRJ): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIB)	

clean:
	-rm -f $(OBJ) $(PRJ)
