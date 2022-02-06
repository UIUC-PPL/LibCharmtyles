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
# with this program. If not, see <https://www.gnu.org/licenses/>.

include ../Makefile.common

TEST_DIR=$(BASE_DIR)/../tests
LIBS_DIR=$(BASE_DIR)

all: matrix performance vector scalar

.PHONY: complex matrix performance vector scalar clean

matrix:
	cd matrix; make

performance:
	cd performance; make

vector:
	cd vector; make

scalar:
	cd scalar; make

clean:
	cd matrix; make clean
	cd vector; make clean
	cd performance; make clean
	cd scalar; make clean
