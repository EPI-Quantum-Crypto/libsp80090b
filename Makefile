CC ?= gcc
# Same floating-point behaviour as the NIST tool (g++ -O2 -ffloat-store
# -march=native): -std=c99 would otherwise turn FMA contraction off, and the
# results would differ from ea_* in the 11th digit. MARCH= gives a portable
# build (results may then differ from a -march=native NIST build).
MARCH ?= -march=native
CFLAGS ?= -std=c99 -O2 -fopenmp -ffloat-store -ffp-contract=fast $(MARCH) -I./include

AR ?= ar

# Run-time dependencies: libdivsufsort (LRS, t-tuple), libbz2 (5.1.11), libgomp, libm
LIB_FLAGS = -ldivsufsort -ldivsufsort64 -lbz2 -lm

all: lib test_c test_iid test_non_iid test_restart test_conditioning selftest_lib

clean:
	rm -f src/*.o libsp80090b.a libsp80090b.so test_c test_iid test_non_iid test_restart test_conditioning test_selftest
	rm -f lab/jent_record

# Library targets (Pure C compilation with gcc)
src/sp80090b_api.o: src/sp80090b_api.c
	$(CC) $(CFLAGS) -fPIC -c src/sp80090b_api.c -o src/sp80090b_api.o

libsp80090b.a: src/sp80090b_api.o
	$(AR) rcs libsp80090b.a src/sp80090b_api.o

libsp80090b.so: src/sp80090b_api.o
	$(CC) -shared $(CFLAGS) src/sp80090b_api.o -o libsp80090b.so $(LIB_FLAGS)

lib: libsp80090b.a libsp80090b.so

# C Test Executables
test_c: src/test_lib.c libsp80090b.a
	$(CC) $(CFLAGS) src/test_lib.c libsp80090b.a $(LIB_FLAGS) -o test_c

test_iid: src/test_iid.c libsp80090b.a
	$(CC) $(CFLAGS) src/test_iid.c libsp80090b.a $(LIB_FLAGS) -o test_iid

test_non_iid: src/test_non_iid.c libsp80090b.a
	$(CC) $(CFLAGS) src/test_non_iid.c libsp80090b.a $(LIB_FLAGS) -o test_non_iid

test_restart: src/test_restart.c libsp80090b.a
	$(CC) $(CFLAGS) src/test_restart.c libsp80090b.a $(LIB_FLAGS) -o test_restart

test_conditioning: src/test_conditioning.c libsp80090b.a
	$(CC) $(CFLAGS) src/test_conditioning.c libsp80090b.a $(LIB_FLAGS) -o test_conditioning

selftest_lib: src/test_selftest.c libsp80090b.a
	$(CC) $(CFLAGS) src/test_selftest.c libsp80090b.a $(LIB_FLAGS) -o test_selftest

# Documentation: docs/pub/<fr|en>/NT-xx_<fr|en>.md -> .docx and .pdf in the same directory (see docs/README.md)
docs:
	python3 docs/build/build_docs.py

# Verification: NIST selftest (test_non_iid on the 11 NIST datasets of bin/,
# compared with selftest/refdata) and the API self-test
check: all
	cd selftest && ./selftest
	./test_selftest

# Raw jitterentropy noise for the labs (replaces jitterentropy-hashtime + extractlsb).
# The noise source must be built with -O0, as in the jitterentropy build.
# Path to the jitterentropy sources (https://github.com/smuellerDD/jitterentropy-library)
JENT_SRC ?= ../jitterentropy-library
JENT_CFLAGS = -std=gnu99 -O0 -Wall -Wextra -DJENT_CONF_ENABLE_INTERNAL_TIMER \
	-I$(JENT_SRC) -I$(JENT_SRC)/src -I$(JENT_SRC)/arch -I$(JENT_SRC)/tests

lab/jent_record: lab/jent_record.c
	$(CC) $(JENT_CFLAGS) lab/jent_record.c -o $@ -lrt -lpthread

.PHONY: all clean lib docs check
