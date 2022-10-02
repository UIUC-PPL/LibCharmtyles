include ../Makefile.common

all: libs

libs: libcharmtyles.a

.PHONY: distclean clean

libcharmtyles.a: charmtyles.o
	$(CHARMC) $(LD_OPTS) -o $@ $< -I$(BASE_DIR); rm charmtyles.o

charmtyles.o: frontend/charmtyles.cpp charmtyles.hpp
	# cd backend; $(CHARMC) charmtyles.ci;
	$(CHARMC) -c $< -I$(BASE_DIR) $(OPTS)

distclean: clean
	rm libcharmtyles.a

clean:
	cd backend; rm *.decl.h *.def.h
	rm libcharmtyles.a
