include ../../Makefile.common

TEST_DIR=$(BASE_DIR)/../tests
LIBS_DIR=$(BASE_DIR)

all: cg eigen_dot jacobi logistic

.PHONY: clean base

base: base.ci
	$(CHARMC) -E base.ci

cg: cg.cpp
	$(CHARMC) -E cg.ci
	$(CHARMC) $< -L$(LIBS_DIR)/charmtyles -lcharmtyles -I$(BASE_DIR) -I$(BASE_DIR)/charmtyles/backend -o $@.out $(OPTS)

eigen_dot: eigen_dot.cpp base
	$(CHARMC) $< -L$(LIBS_DIR)/charmtyles -lcharmtyles -I$(BASE_DIR) -I$(BASE_DIR)/charmtyles/backend -o $@.out $(OPTS)

jacobi: jacobi.cpp
	$(CHARMC) -E jacobi.ci
	$(CHARMC) $< -L$(LIBS_DIR)/charmtyles -lcharmtyles -I$(BASE_DIR) -I$(BASE_DIR)/charmtyles/backend -o $@.out $(OPTS)

logistic: logistic.cpp
	$(CHARMC) -E logistic.ci
	$(CHARMC) -c $< -I$(BASE_DIR) -I$(BASE_DIR)/charmtyles/backend $(OPTS) -o logistic.o
	$(CHARMC) logistic.o -L$(LIBS_DIR)/charmtyles -lcharmtyles -o $@.out $(LD_OPTS)

clean:
	rm *.decl.h *.def.h *.out charmrun
