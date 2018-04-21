#include "duel_cull.h"
#include "Entity.h"

static bool IsPlayer(sharedEntity_t *ent) {
	return ent->s.eType == ET_PLAYER;
}

static bool IsNPC(sharedEntity_t *ent) {
	return ent->s.eType == ET_NPC;
}

static bool IsMover(sharedEntity_t *ent) {
	return ent->s.eType == ET_MOVER;
}

#if 1
static sharedEntity_t *valid_ent(int number) {
	if (number < 0 || number >= ENTITYNUM_WORLD) return nullptr;
	auto ent = SV_GentityNum(number);
	if (!jampog::Entity(ent).inuse()) return nullptr;
	if (!ent->r.linked) return nullptr;
	if (IsPlayer(ent) || IsNPC(ent)) return ent;
	return nullptr;
}
#endif

// Attempts to flatten entities to their owners
// Event entities can use clientNum, otherEntityNum, or otherEnttiyNum2
#if 1
sharedEntity_t *flatten(sharedEntity_t *ent) {
	auto parent = jampog::Entity(ent).parent_ptr();
	if (IsMover(ent) || IsPlayer(ent) || IsNPC(ent)) return ent;
	//else if (auto e = valid_ent(ent->s.number); e && e != ent) return e;
	else if (auto e = valid_ent(ent->s.clientNum); e && e != ent) return e;
	else if (auto e = valid_ent(ent->s.otherEntityNum); e && e != ent) return e;
	else if (auto e = valid_ent(ent->s.otherEntityNum2); e && e!= ent) return e;
	else if (auto e = valid_ent(ent->r.ownerNum); e && e != ent) return e;
	else if (auto e = valid_ent(ent->s.owner); e && e != ent) return e;
	else if (auto e = valid_ent(ent->s.trickedentindex); e && e != ent) return e;
	else if (auto e = valid_ent(ent->s.trickedentindex2); e && e != ent) return e;
	else if (auto e = valid_ent(ent->r.singleClient); e && e != ent) return e;
	else if (auto e = parent ? valid_ent(SV_NumForGentity(parent)) : nullptr; e && e != ent) return e;
	else return ent;
}
#endif

#if 0
static bool valid_ent(int n) {
	return n > 0 && n <= ENTITYNUM_WORLD && jampog::Entity(n).inuse();
}
#endif

#if 0
sharedEntity_t *flatten(sharedEntity_t *ent) {
	//Com_Printf("flatten\n");
	if (ent->s.eType == ET_MISSILE) {
		//Com_Printf("ET_MISSILE\n");
		return SV_GentityNum(ent->r.ownerNum);
	}
	if (ent->s.eType == ET_EVENTS + EV_GENERAL_SOUND) {
		//Com_Printf("EV_GENERAL_SOUND\n");
		return SV_GentityNum(ent->s.otherEntityNum);
	}
	if (ent->s.eType == ET_EVENTS + EV_SABER_HIT) {
		//Com_Printf("EV_SABER_HIT\n");
		return SV_GentityNum(ent->s.otherEntityNum2 == ENTITYNUM_NONE ? ent->s.otherEntityNum : ent->s.otherEntityNum2);
	}
	if (ent->s.eType == ET_EVENTS + EV_SHIELD_HIT) {
		//Com_Printf("EV_SHIELD_HIT\n");
		return SV_GentityNum(ent->s.otherEntityNum);
	}
	// some of EV_SABER_BLOCK's are not owned
	if (ent->s.eType == ET_EVENTS + EV_SABER_BLOCK
	    && valid_ent(ent->s.otherEntityNum)) {
		//Com_Printf("EV_SABER_BLOCK\n");
		return SV_GentityNum(ent->s.otherEntityNum);
	}
	if (ent->s.eFlags & EF_PLAYER_EVENT) {
		//return SV_GentityNum(ent->s.otherEntityNum);
		#if 0
		Com_Printf("EV_SABER_ATTACK: ent: %i, singleClient: %i\n",
			SV_NumForGentity(ent),
			ent->r.singleClient
		);
		#endif
		return SV_GentityNum(ent->r.singleClient);
	}
	if (ent->s.eType == ET_EVENTS + EV_PLAYER_TELEPORT_IN
	    || ent->s.eType == ET_EVENTS + EV_PLAYER_TELEPORT_OUT) {
		//Com_Printf("EV_PLAYER_TELEPORT_X\n");
		return SV_GentityNum(ent->s.clientNum);
	}
	if ((ent->s.event & ~EV_EVENT_BITS) == EV_GRENADE_BOUNCE) {
		return SV_GentityNum(ent->r.ownerNum);
	}
	//Com_Printf("END FLATTEN\n");
	return ent;
}
#endif

static playerState_t *GetPS(sharedEntity_t *ent) {
	return SV_GameClientNum(SV_NumForGentity(ent));
}

static bool IsDueling(sharedEntity_t *ent) {
	return IsPlayer(flatten(ent)) && GetPS(flatten(ent))->duelInProgress;
}

static bool IsActor(sharedEntity_t *ent) {
	return IsPlayer(flatten(ent)) || IsNPC(flatten(ent));
}

static bool IsDueling(sharedEntity_t *A, sharedEntity_t *B) {
	auto a = flatten(A);
	auto b = flatten(B);
	return IsDueling(a) && IsDueling(b) && (a == b || a->playerState->duelIndex == SV_NumForGentity(b));
}

bool DuelCull(sharedEntity_t *ent, sharedEntity_t *touch) {
	constexpr auto CULL = true;
	constexpr auto NO_CULL = !CULL;

	if (!Cvar_VariableIntegerValue("sv_enableDuelCull")) return NO_CULL;
	if (Cvar_VariableIntegerValue("g_gametype") != 0) return NO_CULL;

	if (IsActor(ent) && IsActor(touch)) {
		if (IsDueling(ent, touch)
		    || (!IsDueling(ent) && !IsDueling(touch))) {
			return NO_CULL;
		}
		return CULL;
	}

	return NO_CULL;
}
