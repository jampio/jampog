.PHONY: build clean install

CORES := $(shell nproc)
DESTDIR ?= /usr/local/bin

build:
	@mkdir -p build && \
		cd build && \
		cmake -DCMAKE_INSTALL_PREFIX=$(DESTDIR) .. && \
		cmake --build . -- -j$(CORES)

clean:
	@rm -rf build

install:
	@cmake --build build --target install
