#include "duel_cull.h"

static sharedEntity_t *flatten(sharedEntity_t *ent) {
	return ent->r.ownerNum == ENTITYNUM_NONE ? ent : SV_GentityNum(ent->r.ownerNum);
}

static playerState_t *GetPS(sharedEntity_t *ent) {
	return SV_GameClientNum(ent->s.number);
}

static bool IsPlayer(sharedEntity_t *ent) {
	return ent->s.eType == ET_PLAYER;
}

static bool IsNPC(sharedEntity_t *ent) {
	return ent->s.eType == ET_NPC;
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
	return GetPS(a)
	       && GetPS(a)->duelInProgress
	       && GetPS(a)->duelIndex == b->s.number;
}

bool DuelCull(sharedEntity_t *ent, sharedEntity_t *touch) {
	constexpr auto CULL = true;
	constexpr auto NO_CULL = !CULL;
	if (IsActor(ent) && IsActor(touch)) {
		if (IsDueling(ent, touch)
		    || (!IsDueling(ent) && !IsDueling(touch))) {
			return NO_CULL;
		}
		return CULL;
	}
	return NO_CULL;
}
