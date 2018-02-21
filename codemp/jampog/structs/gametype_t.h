#pragma once

enum {
	GT_FFA,				// free for all
	GT_HOLOCRON,		// holocron ffa
	GT_JEDIMASTER,		// jedi master
	GT_DUEL,		// one on one tournament
	GT_POWERDUEL,
	GT_SINGLE_PLAYER,	// single player ffa

	//-- team games go after this --

	GT_TEAM,			// team deathmatch
	GT_SIEGE,			// siege
	GT_CTF,				// capture the flag
	GT_CTY,
	GT_MAX_GAME_TYPE
};
