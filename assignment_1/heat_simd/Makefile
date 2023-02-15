CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
SRC=$(filter-out $(wildcard ref*.c), $(wildcard *.c))
OBJ=$(patsubst %.c,%.o,$(SRC))

CC=gcc
CFLAGS=-O0 -g3 -std=gnu99 -march=haswell -lm
INCLUDES=-I../../include
LIB=
ifdef DEBUG
CFLAGS=-O2 -std=gnu99 -march=haswell -Wall -lm
endif
ifdef autovec
CFLAGS:= -O3 -Wall -march=haswell -ffast-math -fopt-info-vec 
endif

all: $(PRJ)

$(PRJ): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIB)	

clean:
	-rm -f $(OBJ) $(PRJ)
