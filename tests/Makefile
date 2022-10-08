include ../Makefile.common

TEST_DIR=$(BASE_DIR)/../tests
LIBS_DIR=$(BASE_DIR)

all: matrix performance vector

.PHONY: matrix performance vector clean

matrix:
	cd matrix; make

performance:
	cd performance; make

vector:
	cd vector; make

clean:
	cd matrix; make clean
	cd performance; make clean
	cd vector; make clean
