include Makefile.common

all: libs tests

.PHONY: libs tests clean

libs:
	cd charmtyles; make

tests: libs
	cd tests; make

clean:
	cd charmtyles; make clean
	cd tests; make clean
