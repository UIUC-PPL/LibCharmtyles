include ../../Makefile.common

TEST_DIR=$(BASE_DIR)/../tests
LIBS_DIR=$(BASE_DIR)

all: comprehensive_tests

.PHONY: clean base

base: base.ci
	$(CHARMC) -E base.ci

comprehensive_tests: comprehensive.cpp base
	$(CHARMC) $< -L$(LIBS_DIR)/charmtyles -lcharmtyles -I$(BASE_DIR) -I$(BASE_DIR)/charmtyles/backend -o $@.out $(OPTS)

clean:
	rm *.decl.h *.def.h *.out charmrun 2>/dev/null || true
