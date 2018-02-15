CORES ?= $(shell nproc)
DESTDIR ?= /usr/local/bin

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
	@cd build && cmake -DCMAKE_INSTALL_PREFIX=$(DESTDIR) ..
