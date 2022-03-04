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

include ../../Makefile.common

TEST_DIR=$(BASE_DIR)/../tests
LIBS_DIR=$(BASE_DIR)

all: level_1

.PHONY: clean

level_1: level_1.cpp level_1.ci
	$(CHARMC) -E $(word 2,$^)
	$(CHARMC) $< -L$(LIBS_DIR)/aum -laum -I$(BASE_DIR) -I$(BASE_DIR)/aum/backend -o $@.out $(OPTS)

clean:
	rm *.decl.h *.def.h *.out
