#include "detour.h"
#include "game_structs.h"
#include <cstdint>

constexpr auto FOLLOWCYCLE_OFFSET = 0x0012C264;
constexpr auto LEVEL_OFFSET = 0x0068A3A0;
constexpr auto G_ENTITIES_OFFSET = 0x006CE620;
constexpr auto GET_STRING_ED_OFFSET = 0x0008B184;
constexpr auto TRAP_SENDSERVERCOMMAND_OFFSET = 0x0015E454;
constexpr auto SET_TEAM_OFFSET = 0x0012ADA4;

#if 0
static const char *(*G_GetStringEdString)(char *refSection, char *refName) = nullptr;
static void (*trap_SendServerCommand)(int, const char *) = nullptr;
static gentity_t *g_entities = nullptr;
static level_locals_t *g_level = nullptr;
static void (*SetTeam)(gentity_t *ent, const char *team) = nullptr;
#endif

#if 0
//TODO: pers.connected doesn't seem to be as expected
// may have copied structs or offsets wrong
// using stock now, seems to be okay with setteam patches
static void followcycle(gentity_t *ent, int dir) {
	const int original = ent->client->sess.spectatorClient;
	for (int clientnum = original + dir,
		     i = 0, j = 0;
	     i < g_level->maxclients /*&& j < g_level->numNonSpectatorClients*/;
	     clientnum += dir, i++, j++) {
		if (clientnum >= g_level->maxclients)
			clientnum = 0;
		else if (clientnum < 0)
			clientnum = g_level->maxclients - 1;
		Com_Printf("Looping...\n");
		Com_Printf("clientnum: %d\n", clientnum);
		Com_Printf("original: %d\n", original);
		Com_Printf("is_connected: %d\n", g_level->clients[clientnum].pers.connected == CON_CONNECTED);
		Com_Printf("not_spectator: %d\n", g_level->clients[clientnum].sess.sessionTeam != TEAM_SPECTATOR);
		Com_Printf("tempSpectate: %d\n", g_level->clients[clientnum].tempSpectate < g_level->time);
		Com_Printf("is_not_original: %d\n", clientnum != original);
		if (g_level->clients[clientnum].pers.connected == CON_CONNECTED
		    && g_level->clients[clientnum].sess.sessionTeam != TEAM_SPECTATOR
		    && g_level->clients[clientnum].tempSpectate < g_level->time
			&& ((clientnum != original && ent->client->sess.spectatorState == SPECTATOR_FOLLOW) || ent->client->sess.spectatorState == SPECTATOR_FREE)) {
			ent->client->sess.spectatorClient = clientnum;
			ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
			return;
		}
	}
	ent->client->sess.spectatorClient = 0;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
}
#endif

namespace jampog {
	void patch_followcycle(uintptr_t base) {
		//detour((void*)(base + FOLLOWCYCLE_OFFSET),
		//       (void*)followcycle3);
		//G_GetStringEdString = (decltype(G_GetStringEdString))(base + GET_STRING_ED_OFFSET);
		//trap_SendServerCommand = (decltype(trap_SendServerCommand))(base + TRAP_SENDSERVERCOMMAND_OFFSET);
		//g_entities = (gentity_t *)(base + G_ENTITIES_OFFSET);
		//g_level = (level_locals_t *)(base + LEVEL_OFFSET);
		//SetTeam = (decltype(SetTeam))(base + SET_TEAM_OFFSET);
	}
	void patch_setteam(uintptr_t base) {
		// patch "team s" or "team scoreboard"
		patch_byte((unsigned char*)(base + 0x0012AE11), SPECTATOR_FREE);
		// patch "team follow1"
		patch_byte((unsigned char*)(base + 0x0012AE3D), SPECTATOR_FREE);
		patch_word((unsigned int*)(base + 0x0012AE42), 0);
		// patch "team follow2"
		patch_byte((unsigned char*)(base + 0x0012AE71), SPECTATOR_FREE);
		patch_word((unsigned int*)(base + 0x0012AE76), 0);
	}
}
