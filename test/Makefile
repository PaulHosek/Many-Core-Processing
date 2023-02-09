CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
OBJ=test_compute.o ../assignment_1/heat_seq/compute.o

CC=gcc
INCLUDES=-I../include
ifndef DEBUG
CFLAGS=-O2 -std=gnu99
LIB=
else
CFLAGS=-O0 -g3 -std=gnu99
LIB=
endif

all: $(PRJ)

$(PRJ): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIB)	

clean:
	-rm -f $(OBJ) $(PRJ)
