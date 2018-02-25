#include "base.h"
#include "forcepowers.h"
#include "followcycle.h"
#include "clean_name.h"
#include "duel.h"
#include "detour.h"
#include "Entity.h"
#include "offsets.h"

qboolean cheats_okay(void *ptr);

namespace jampog {
	void init(const vm_t * const vm) {
		Com_Printf("initializing jampog\n");
		const uintptr_t base = dladdress(vm->dllHandle);
		set_base((void*)base);
		Com_Printf("patching BG_LegalizedForcePowers\n");
		patch_forcepowers(base);
		Com_Printf("patching SetTeam\n");
		patch_setteam(base);
		Com_Printf("patching ClientCleanName\n");
		patch_clean_name(base);
		Com_Printf("patching Cmd_EngageDuel_f\n");
		patch_engage_duel(base);
		Com_Printf("patching CheatsOk\n");
		detour((void*)(base + 0x00129B94), (void*)cheats_okay);
		#if 0
		Com_Printf("unprotect g_entities[]\n");
		unprotect(Entity::start(), Entity::array_size());
		Com_Printf("unprotect g_clients[]\n");
		unprotect(Client::start(), Client::array_size());
		#endif
	}
}