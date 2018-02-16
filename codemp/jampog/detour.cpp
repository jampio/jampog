#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

static inline uintptr_t pagealign(const uintptr_t p) {
	const uintptr_t pageSize = uintptr_t(sysconf(_SC_PAGESIZE));
	return (p & ~(pageSize - 1));
}

static void protect(const void * const addr, const size_t len,
                    const int flags = PROT_READ|PROT_WRITE|PROT_EXEC) {
	const uintptr_t page = pagealign((uintptr_t)addr);
	const size_t size = uintptr_t(addr) - page + len;
	const int res = mprotect((void*)page, size, flags);
	if (res) {
		perror("mprotect");
		exit(EXIT_FAILURE);
	}
}

namespace jampog {

void detour(void * const before, const void * const after) {
	protect(before, 5);
	*(unsigned char*)before = 0xE9;
	*(uint32_t*)((unsigned char*)before+1) =
		(uint32_t)((unsigned char*)after - ((unsigned char*)before + 5));
}

}
