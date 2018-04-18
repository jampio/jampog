#include "duel_cull.h"

static bool valid_number(int number) {
	return number >= 0
	       && number < ENTITYNUM_NONE;
}

// Attempts to flatten entities to their owners
// Event entities can use clientNum, otherEntityNum, or otherEnttiyNum2
static sharedEntity_t *flatten(sharedEntity_t *ent) {
	if (valid_number(ent->s.clientNum)) {
		return SV_GentityNum(ent->s.clientNum);
	} else if (valid_number(ent->s.otherEntityNum)) {
		return SV_GentityNum(ent->s.otherEntityNum);
	} else if (valid_number(ent->s.otherEntityNum2)) {
		return SV_GentityNum(ent->s.otherEntityNum2);
	} else if (valid_number(ent->r.ownerNum)) {
		return SV_GentityNum(ent->r.ownerNum);
	} else {
		return ent;
	}
}

static playerState_t *GetPS(sharedEntity_t *ent) {
	return SV_GameClientNum(SV_NumForGentity(ent));
}

static bool IsPlayer(sharedEntity_t *ent) {
	return ent->s.eType == ET_PLAYER;
}

static bool IsNPC(sharedEntity_t *ent) {
	return ent->s.eType == ET_NPC;
}

static bool IsMover(sharedEntity_t *ent) {
	return ent->s.eType == ET_MOVER;
}

static bool IsDueling(sharedEntity_t *ent) {
	return IsPlayer(flatten(ent)) && GetPS(flatten(ent))->duelInProgress;
}

static bool IsActor(sharedEntity_t *ent) {
	// movers are not safe to flatten
	return !IsMover(ent) && (IsPlayer(flatten(ent)) || IsNPC(flatten(ent)));
}

static bool IsDueling(sharedEntity_t *A, sharedEntity_t *B) {
	auto a = flatten(A);
	auto b = flatten(B);
	// checking for owned event entities
	if (a == b || SV_NumForGentity(a) == SV_NumForGentity(b)) {
		return true;
	}
	return GetPS(a)
	       && GetPS(a)->duelInProgress
	       && GetPS(a)->duelIndex == SV_NumForGentity(b);
}

bool DuelCull(sharedEntity_t *ent, sharedEntity_t *touch) {
	constexpr auto CULL = true;
	constexpr auto NO_CULL = !CULL;
	if (Cvar_VariableIntegerValue("g_gametype") != 0) {
		return NO_CULL;
	}
	if (IsActor(ent) && IsActor(touch)) {
		if (IsDueling(ent, touch)
		    || (!IsDueling(ent) && !IsDueling(touch))) {
			return NO_CULL;
		}
		return CULL;
	}
	return NO_CULL;
}
