.PHONY: clean install

CORES := $(shell nproc)
DESTDIR ?= /usr/local/bin

build/jampog:
	@mkdir -p build && cd build && cmake .. && cmake --build . -- -j$(CORES)

clean:
	@rm -rf build

install:
	@mv build/jampog $(DESTDIR)/jampog
