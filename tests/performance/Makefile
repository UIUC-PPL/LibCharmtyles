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

all: conjugate_gradient individual_metric multi_dot

.PHONY: clean performance

performance: performance.ci
	$(CHARMC) -E performance.ci

conjugate_gradient: conjugate_gradient.cpp performance
	$(CHARMC) $< -L$(LIBS_DIR)/aum -laum -I$(BASE_DIR) -I$(BASE_DIR)/aum/backend -o $@.out $(OPTS)

individual_metric: individual_metric.cpp performance
	$(CHARMC) $< -L$(LIBS_DIR)/aum -laum -I$(BASE_DIR) -I$(BASE_DIR)/aum/backend -o $@.out $(OPTS)

multi_dot: multi_dot.cpp performance
	$(CHARMC) $< -L$(LIBS_DIR)/aum -laum -I$(BASE_DIR) -I$(BASE_DIR)/aum/backend -o $@.out $(OPTS)

clean:
	rm *.decl.h *.def.h *.out charmrun