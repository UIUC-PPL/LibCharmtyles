include ../Makefile.common

TEST_DIR=$(BASE_DIR)/../tests
LIBS_DIR=$(BASE_DIR)

all: matrix vector

.PHONY: matrix vector clean

matrix:
	cd matrix; make

vector:
	cd vector; make

clean:
	cd matrix; make clean
	cd vector; make clean
