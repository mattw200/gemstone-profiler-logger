IDIR=../include
CC=gcc
CFLAGS=-I$(IDIR)
DEPS=pmc-helper-64.h
BIN=../bin
#OBJ=pmc-test.o pmc-setup.o pmc-get-header.o pmc-get-row.o

#%.o: %.c $(DEPS)
#	$(cc) -c -o $@ $< $(CFLAGS)

all: $(BIN)/pmc-test $(BIN)/pmc-setup $(BIN)/pmc-get-header $(BIN)/pmc-get-row
odroid_c2: CFLAGS += -DPLATFORM_ODROID_C2
odroid_c2: $(BIN)/pmc-test $(BIN)/pmc-setup $(BIN)/pmc-get-header $(BIN)/pmc-get-row

$(BIN)/pmc-test: pmc-test.o pmc-helper-64.o
	$(CC) -o $@ $^ $(CLFAGS)

$(BIN)/pmc-setup: pmc-setup.o pmc-helper-64.o
	$(CC) -o $@ $^ $(CLFAGS)

$(BIN)/pmc-get-header: pmc-get-header.o pmc-helper-64.o
	$(CC) -o $@ $^ $(CLFAGS)

$(BIN)/pmc-get-row: pmc-get-row.o pmc-helper-64.o
	$(CC) -o $@ $^ $(CLFAGS)

.PHONY: clean

clean:
	rm -rf *.o $(BIN)/*

