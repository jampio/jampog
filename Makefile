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

/usr/lib32/libcxa.so.1:
	@wget https://github.com/jampio/jampog/releases/download/v1.01/libcxa.so.1 -O $@

~/.local/share/jampog/base/jampgamei386.so:
	@mkdir -p ~/.local/share/jampog/base/
	@wget https://github.com/jampio/jampog/releases/download/v1.01/jampgamei386.so -O $@

game-deps: /usr/lib32/libcxa.so.1 ~/.local/share/jampog/base/jampgamei386.so

build-deps:
	@apt install make cmake gcc-multilib g++-multilib

.PHONY: assets
assets:
	scripts/steam.sh
