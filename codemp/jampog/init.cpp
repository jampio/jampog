#include <cstdio>
#include <link.h>
#include "init.h"
#include "forcepowers.h"
#include "followcycle.h"

static uintptr_t dladdress(void * const handle) {
	void *map;
	const int res = dlinfo(handle, RTLD_DI_LINKMAP, &map);
	if (res) {
		fprintf(stderr, "dladdress failed: %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	return ((struct link_map*)map)->l_addr;
}

void jampog::init(const vm_t * const vm) {
	Com_Printf("initializing jampog\n");
	const uintptr_t base = dladdress(vm->dllHandle);
	Com_Printf("patching BG_LegalizedForcePowers\n");
	patch_forcepowers(base);
	Com_Printf("patching SetTeam\n");
	patch_setteam(base);
}
