CORES ?= $(shell nproc)
DESTDIR ?= /usr/local/bin

CXXSTD ?= c++17
CFLAGS ?= -m32 -march=i386
CXXFLAGS ?= -m32 -march=i386 -std=$(CXXSTD)

CMAKEFLAGS = -DCMAKE_INSTALL_PREFIX=$(DESTDIR) \
             -DCMAKE_C_FLAGS=$(CFLAGS) \
             -DCMAKE_CXX_FLAGS=$(CXXFLAGS)

.PHONY: all
all: | build/Makefile
	@cmake --build build -- -j$(CORES) --no-print-directory

.PHONY: clean
clean:
	@rm -rf build

.PHONY: install
install:
	@cmake --build build --target install -- --no-print-directory

build/:
	@mkdir -p $@

build/Makefile: | build/
	@cd build && cmake $(CMAKEFLAGS) ..
