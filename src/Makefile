IDIR=../include
CC=gcc
CFLAGS=-I$(IDIR)
DEPS=pmc-helper-64.h pmc-get-row.h
BIN=../bin

all: $(BIN)/pmc-run $(BIN)/pmc-setup $(BIN)/pmc-get-header $(BIN)/pmc-get-pmcs $(BIN)/pmc-runtime

no_freq: CFLAGS += -DNO_FREQ
no_freq: $(BIN)/pmc-run $(BIN)/pmc-setup $(BIN)/pmc-get-header $(BIN)/pmc-get-pmcs $(BIN)/pmc-runtime

odroid_c2: CFLAGS += -DPLATFORM_ODROID_C2
odroid_c2: $(BIN)/pmc-run $(BIN)/pmc-setup $(BIN)/pmc-get-header $(BIN)/pmc-get-pmcs $(BIN)/pmc-runtime

odroid_xu3: CFLAGS += -DPLATFORM_ODROID_XU3
odroid_xu3: $(BIN)/pmc-run $(BIN)/pmc-setup $(BIN)/pmc-get-header $(BIN)/pmc-get-pmcs $(BIN)/pmc-runtime

$(BIN)/pmc-runtime: pmc-runtime.o pmc-get-row.o pmc-helper-64.o 
	$(CC) -o $@ $^ $(CLFAGS)

$(BIN)/pmc-run: pmc-run.o pmc-get-row.o pmc-helper-64.o 
	$(CC) -o $@ $^ $(CLFAGS)

$(BIN)/pmc-setup: pmc-setup.o pmc-helper-64.o
	$(CC) -o $@ $^ $(CLFAGS)

$(BIN)/pmc-get-header: pmc-get-header.o pmc-helper-64.o
	$(CC) -o $@ $^ $(CLFAGS)

$(BIN)/pmc-get-pmcs: pmc-get-pmcs.o pmc-get-row.o pmc-helper-64.o
	$(CC) -o $@ $^ $(CLFAGS)

.PHONY: clean

clean:
	rm -rf *.o $(BIN)/*

