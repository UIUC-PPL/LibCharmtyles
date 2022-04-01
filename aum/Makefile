#  Copyright (C) 2022 Nikunj Gupta
#
#  This program is free software: you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the Free
#   Software Foundation, version 3.
#
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program. If not, see <https://www.gnu.org/licenses/>.

include ../Makefile.common

all: libs

libs: libaum.a

.PHONY: distclean clean

libaum.a: aum.o
	$(CHARMC) $(LD_OPTS) -o $@ $< -I$(BASE_DIR); rm aum.o

aum.o: frontend/aum.cpp aum.hpp
	cd backend; $(CHARMC) exitter.ci; $(CHARMC) custom_callbacks.ci; $(CHARMC) scalar.ci; $(CHARMC) vector.ci; $(CHARMC) matrix.ci; $(CHARMC) container.ci; $(CHARMC) aum.ci;
	$(CHARMC) -c $< -I$(BASE_DIR) $(OPTS)

distclean: clean
	rm libaum.a

clean:
	cd backend; rm *.decl.h *.def.h
	rm libaum.a
