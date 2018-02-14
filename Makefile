.PHONY: build clean install

CORES := $(shell nproc)
DESTDIR ?= /usr/local/bin
INSTALL_PROGRAM ?= /usr/bin/install

build:
	@mkdir -p build && cd build && cmake .. && cmake --build . -- -j$(CORES)

clean:
	@rm -rf build

install:
	@$(INSTALL_PROGRAM) build/jampog $(DESTDIR)
