#pragma once

#include "qcommon/qcommon.h"

constexpr auto MAX_ITEM_MODELS = 4;
constexpr auto SABER_NAME_LENGTH = 64;
constexpr auto MAX_SABERS = 2;
constexpr auto MAX_BLADES = 8;
constexpr auto MAX_FAILED_NODES = 8;
constexpr auto MAX_PARMS = 16;
constexpr auto MAX_PARM_STRING_LENGTH = MAX_QPATH;
constexpr auto VEH_MAX_PASSENGERS = 10;
constexpr auto MAX_VEHICLE_MUZZLES = 12;
constexpr auto MAX_VEHICLE_EXHAUSTS = 12;
constexpr auto MAX_VEHICLE_WEAPONS = 2;
constexpr auto MAX_VEHICLE_TURRETS = 2;
constexpr auto MAX_VEHICLE_TURRET_MUZZLES = 2;
constexpr auto MAX_GROUP_MEMBERS = 32;
constexpr auto MAX_SPAWN_VARS = 64;
constexpr auto MAX_SPAWN_VARS_CHARS	= 4096;
constexpr auto BODY_QUEUE_SIZE = 8;
constexpr auto MAX_ALERT_EVENTS = 32;
constexpr auto MAX_FRAME_GROUPS = 32;
constexpr auto MAX_INTEREST_POINTS = 64;
constexpr auto MAX_COMBAT_POINTS = 512;

#define	MAX_ENEMY_POS_LAG	2400
#define	ENEMY_POS_LAG_INTERVAL	100
#define	ENEMY_POS_LAG_STEPS	(MAX_ENEMY_POS_LAG/ENEMY_POS_LAG_INTERVAL)

struct gentity_t;
struct Vehicle_t;
struct gclient_t;

struct interestPoint_t {
	vec3_t		origin;
	char		*target;
};

struct combatPoint_t {
	vec3_t		origin;
	int			flags;
//	char		*NPC_targetname;
//	team_t		team;
	qboolean	occupied;
	int			waypoint;
	int			dangerTime;
};

typedef enum {
	AET_SIGHT,
	AET_SOUND,
} alertEventType_e;

typedef enum {
	AEL_MINOR,			//Enemy responds to the sound, but only by looking
	AEL_SUSPICIOUS,		//Enemy looks at the sound, and will also investigate it
	AEL_DISCOVERED,		//Enemy knows the player is around, and will actively hunt
	AEL_DANGER,			//Enemy should try to find cover
	AEL_DANGER_GREAT,	//Enemy should run like hell!
} alertEventLevel_e;

struct alertEvent_t {
	vec3_t				position;	//Where the event is located
	float				radius;		//Consideration radius
	alertEventLevel_e	level;		//Priority level of the event
	alertEventType_e	type;		//Event type (sound,sight)
	gentity_t			*owner;		//Who made the sound
	float				light;		//ambient light level at point
	float				addLight;	//additional light- makes it more noticable, even in darkness
	int					ID;			//unique... if get a ridiculous number, this will repeat, but should not be a problem as it's just comparing it to your lastAlertID
	int					timestamp;	//when it was created
};

typedef enum {
	TEAM_FREE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_SPECTATOR,

	TEAM_NUM_TEAMS
} team_t;

enum
{
	SQUAD_IDLE,					//No target found, waiting
	SQUAD_STAND_AND_SHOOT,		//Standing in position and shoot (no cover)
	SQUAD_RETREAT,				//Running away from combat
	SQUAD_COVER,				//Under protective cover
	SQUAD_TRANSITION,			//Moving between points, not firing
	SQUAD_POINT,				//On point, laying down suppressive fire
	SQUAD_SCOUT,				//Poking out to draw enemy
	NUM_SQUAD_STATES,
};


struct AIGroupMember_t {
	int	number;
	int waypoint;
	int pathCostToEnemy;
	int	closestBuddy;
};

struct AIGroupInfo_t {
	int			numGroup;
	qboolean	processed;
	team_t		team;
	gentity_t	*enemy;
	int			enemyWP;
	int			speechDebounceTime;
	int			lastClearShotTime;
	int			lastSeenEnemyTime;
	int			morale;
	int			moraleAdjust;
	int			moraleDebounce;
	int			memberValidateTime;
	int			activeMemberNum;
	gentity_t	*commander;
	vec3_t		enemyLastSeenPos;
	int			numState[NUM_SQUAD_STATES];
	AIGroupMember_t member[MAX_GROUP_MEMBERS];
};


struct level_locals_t {
	gclient_t	*clients;		// [maxclients]

	gentity_t	*gentities;
	int			gentitySize;
	int			num_entities;		// current number, <= MAX_GENTITIES

	int			warmupTime;			// restart match at this time

	fileHandle_t	logFile;

	// store latched cvars here that we want to get at often
	int			maxclients;

	int			framenum;
	int			time;					// in msec
	int			previousTime;			// so movers can back up when blocked

	int			startTime;				// level.time the map was started

	int			teamScores[TEAM_NUM_TEAMS];
	int			lastTeamLocationTime;		// last time of client team location update

	qboolean	newSession;				// don't use any old session data, because
										// we changed gametype

	qboolean	restarted;				// waiting for a map_restart to fire

	int			numConnectedClients;
	int			numNonSpectatorClients;	// includes connecting clients
	int			numPlayingClients;		// connected, non-spectators
	int			sortedClients[MAX_CLIENTS];		// sorted by score
	int			follow1, follow2;		// clientNums for auto-follow spectators

	int			snd_fry;				// sound index for standing in lava

	int			snd_hack;				//hacking loop sound
    int			snd_medHealed;			//being healed by supply class
	int			snd_medSupplied;		//being supplied by supply class

	int			warmupModificationCount;	// for detecting if g_warmup is changed

	// voting state
	char		voteString[MAX_STRING_CHARS];
	char		voteDisplayString[MAX_STRING_CHARS];
	int			voteTime;				// level.time vote was called
	int			voteExecuteTime;		// time the vote is executed
	int			voteYes;
	int			voteNo;
	int			numVotingClients;		// set by CalculateRanks

	qboolean	votingGametype;
	int			votingGametypeTo;

	// team voting state
	char		teamVoteString[2][MAX_STRING_CHARS];
	int			teamVoteTime[2];		// level.time vote was called
	int			teamVoteYes[2];
	int			teamVoteNo[2];
	int			numteamVotingClients[2];// set by CalculateRanks

	// spawn variables
	qboolean	spawning;				// the G_Spawn*() functions are valid
	int			numSpawnVars;
	char		*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int			numSpawnVarChars;
	char		spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// intermission state
	int			intermissionQueued;		// intermission was qualified, but
										// wait INTERMISSION_DELAY_TIME before
										// actually going there so the last
										// frag can be watched.  Disable future
										// kills during this delay
	int			intermissiontime;		// time the intermission was started
	char		*changemap;
	qboolean	readyToExit;			// at least one client wants to exit
	int			exitTime;
	vec3_t		intermission_origin;	// also used for spectator spawns
	vec3_t		intermission_angle;

	qboolean	locationLinked;			// target_locations get linked
	gentity_t	*locationHead;			// head of the location list
	int			bodyQueIndex;			// dead bodies
	gentity_t	*bodyQue[BODY_QUEUE_SIZE];
	int			portalSequence;

	alertEvent_t	alertEvents[ MAX_ALERT_EVENTS ];
	int				numAlertEvents;
	int				curAlertID;

	AIGroupInfo_t	groups[MAX_FRAME_GROUPS];

	//Interest points- squadmates automatically look at these if standing around and close to them
	interestPoint_t	interestPoints[MAX_INTEREST_POINTS];
	int			numInterestPoints;

	//Combat points- NPCs in bState BS_COMBAT_POINT will find their closest empty combat_point
	combatPoint_t	combatPoints[MAX_COMBAT_POINTS];
	int			numCombatPoints;

	//rwwRMG - added:
	int			mNumBSPInstances;
	int			mBSPInstanceDepth;
	vec3_t		mOriginAdjust;
	float		mRotationAdjust;
	char		*mTargetAdjust;

	char		mTeamFilter[MAX_QPATH];

};


typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

typedef enum //# jumpState_e
{
	JS_WAITING = 0,
	JS_FACING,
	JS_CROUCHING,
	JS_JUMPING,
	JS_LANDING
} jumpState_t;

typedef enum //# bState_e
{//These take over only if script allows them to be autonomous
	BS_DEFAULT = 0,//# default behavior for that NPC
	BS_ADVANCE_FIGHT,//# Advance to captureGoal and shoot enemies if you can
	BS_SLEEP,//# Play awake script when startled by sound
	BS_FOLLOW_LEADER,//# Follow your leader and shoot any enemies you come across
	BS_JUMP,//# Face navgoal and jump to it.
	BS_SEARCH,//# Using current waypoint as a base, search the immediate branches of waypoints for enemies
	BS_WANDER,//# Wander down random waypoint paths
	BS_NOCLIP,//# Moves through walls, etc.
	BS_REMOVE,//# Waits for player to leave PVS then removes itself
	BS_CINEMATIC,//# Does nothing but face it's angles and move to a goal if it has one
	//# #eol
	//internal bStates only
	BS_WAIT,//# Does nothing but face it's angles
	BS_STAND_GUARD,
	BS_PATROL,
	BS_INVESTIGATE,//# head towards temp goal and look for enemies and listen for sounds
	BS_STAND_AND_SHOOT,
	BS_HUNT_AND_KILL,
	BS_FLEE,//# Run away!
	NUM_BSTATES
} bState_t;

typedef enum {
	RANK_CIVILIAN,
	RANK_CREWMAN,
	RANK_ENSIGN,
	RANK_LT_JG,
	RANK_LT,
	RANK_LT_COMM,
	RANK_COMMANDER,
	RANK_CAPTAIN
} rank_t;

typedef enum {VIS_UNKNOWN, VIS_NOT, VIS_PVS, VIS_360, VIS_FOV, VIS_SHOOT} visibility_t;

struct gNPCstats_t {//Stats, loaded in, and can be set by scripts
	//AI
	int		aggression;			//			"
	int		aim;				//			"
	float	earshot;			//			"
	int		evasion;			//			"
	int		hfov;				// horizontal field of view
	int		intelligence;		//			"
	int		move;				//			"
	int		reactions;			// 1-5, higher is better
	float	shootDistance;		//Maximum range- overrides range set for weapon if nonzero
	int		vfov;				// vertical field of view
	float	vigilance;			//			"
	float	visrange;			//			"
	//Movement
	int		runSpeed;
	int		walkSpeed;
	float	yawSpeed;			// 1 - whatever, default is 50
	int		health;
	int		acceleration;
};

struct gNPC_t {
	//FIXME: Put in playerInfo or something
	int			timeOfDeath;			//FIXME do we really need both of these
	gentity_t	*touchedByPlayer;

	visibility_t	enemyLastVisibility;

	int			aimTime;
	float		desiredYaw;
	float		desiredPitch;
	float		lockedDesiredYaw;
	float		lockedDesiredPitch;
	gentity_t	*aimingBeam;		// debugging aid

	vec3_t		enemyLastSeenLocation;
	int			enemyLastSeenTime;
	vec3_t		enemyLastHeardLocation;
	int			enemyLastHeardTime;
	int			lastAlertID;		//unique ID

	int			eFlags;
	int			aiFlags;

	int			currentAmmo;		// this sucks, need to find a better way
	int			shotTime;
	int			burstCount;
	int			burstMin;
	int			burstMean;
	int			burstMax;
	int			burstSpacing;
	int			attackHold;
	int			attackHoldTime;
	vec3_t		shootAngles;		//Angles to where bot is shooting - fixme: make he torso turn to reflect these

	//extra character info
	rank_t		rank;		//for pips

	//Behavior state info
	bState_t	behaviorState;	//determines what actions he should be doing
	bState_t	defaultBehavior;//State bot will default to if none other set
	bState_t	tempBehavior;//While valid, overrides other behavior

	qboolean	ignorePain;		//only play pain scripts when take pain

	int			duckDebounceTime;//Keeps them ducked for a certain time
	int			walkDebounceTime;
	int			enemyCheckDebounceTime;
	int			investigateDebounceTime;
	int			investigateCount;
	vec3_t		investigateGoal;
	int			investigateSoundDebounceTime;
	int			greetingDebounceTime;//when we can greet someone next
	gentity_t	*eventOwner;

	//bState-specific fields
	gentity_t	*coverTarg;
	jumpState_t	jumpState;
	float		followDist;

	// goal, navigation & pathfinding
	gentity_t	*tempGoal;			// used for locational goals (player's last seen/heard position)
	gentity_t	*goalEntity;
	gentity_t	*lastGoalEntity;
	gentity_t	*eventualGoal;
	gentity_t	*captureGoal;		//Where we should try to capture
	gentity_t	*defendEnt;			//Who we're trying to protect
	gentity_t	*greetEnt;			//Who we're greeting
	int			goalTime;	//FIXME: This is never actually used
	qboolean	straightToGoal;	//move straight at navgoals
	float		distToGoal;
	int			navTime;
	int			blockingEntNum;
	int			blockedSpeechDebounceTime;
	int			lastSideStepSide;
	int			sideStepHoldTime;
	int			homeWp;
	AIGroupInfo_t	*group;

	vec3_t		lastPathAngles;		//So we know which way to face generally when we stop

	//stats
	gNPCstats_t	stats;
	int			aimErrorDebounceTime;
	float		lastAimErrorYaw;
	float		lastAimErrorPitch;
	vec3_t		aimOfs;
	int			currentAim;
	int			currentAggression;

	//scriptflags
	int			scriptFlags;//in b_local.h

	//moveInfo
	int			desiredSpeed;
	int			currentSpeed;
	char		last_forwardmove;
	char		last_rightmove;
	vec3_t		lastClearOrigin;
	int			consecutiveBlockedMoves;
	int			blockedDebounceTime;
	int			shoveCount;
	vec3_t		blockedDest;

	//
	int			combatPoint;//NPCs in bState BS_COMBAT_POINT will find their closest empty combat_point
	int			lastFailedCombatPoint;//NPCs in bState BS_COMBAT_POINT will find their closest empty combat_point
	int			movementSpeech;	//what to say when you first successfully move
	float		movementSpeechChance;//how likely you are to say it

	//Testing physics at 20fps
	int			nextBStateThink;
	usercmd_t	last_ucmd;

	//
	//JWEIER ADDITIONS START

	qboolean	combatMove;
	int			goalRadius;

	//FIXME: These may be redundant

	/*
	int			weaponTime;		//Time until refire is valid
	int			jumpTime;
	*/
	int			pauseTime;		//Time to stand still
	int			standTime;

	int			localState;		//Tracking information local to entity
	int			squadState;		//Tracking information for team level interaction

	//JWEIER ADDITIONS END
	//

	int			confusionTime;	//Doesn't respond to alerts or pick up enemies (unless shot) until this time is up
	int			charmedTime;	//charmed to enemy team
	int			controlledTime;	//controlled by player
	int			surrenderTime;	//Hands up

	//Lagging enemy position - FIXME: seems awful wasteful...
	vec3_t		enemyLaggedPos[ENEMY_POS_LAG_STEPS];

	gentity_t	*watchTarget;	//for BS_CINEMATIC, keeps facing this ent

	int			ffireCount;		//sigh... you'd think I'd be able to find a way to do this without having to use 3 int fields, but...
	int			ffireDebounce;
	int			ffireFadeDebounce;
};

struct entityShared_t {
	qboolean	linked;				// qfalse if not in any good cluster
	int			linkcount;

	int			svFlags;			// SVF_NOCLIENT, SVF_BROADCAST, etc
	int			singleClient;		// only send to this client when SVF_SINGLECLIENT is set

	qboolean	bmodel;				// if false, assume an explicit mins / maxs bounding box
									// only set by trap_SetBrushModel
	vec3_t		mins, maxs;
	int			contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		// derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;
	vec3_t		currentAngles;
	qboolean	mIsRoffing;			// set to qtrue when the entity is being roffed

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int			ownerNum;

	// mask of clients that this entity should be broadcast too.  The first 32 clients
	// are represented by the first array index and the latter 32 clients are represented
	// by the second array index.
	int			broadcastClients[2];

};

struct turretStats_t {
	int			iWeapon;	//what vehWeaponInfo index to use
	int			iDelay;		//delay between turret muzzle shots
	int			iAmmoMax;	//how much ammo it has
	int			iAmmoRechargeMS;	//how many MS between every point of recharged ammo
	char		*yawBone;	//bone on ship that this turret uses to yaw
	char		*pitchBone;	//bone on ship that this turret uses to pitch
	int			yawAxis;	//axis on yawBone to which we should to apply the yaw angles
	int			pitchAxis;	//axis on pitchBone to which we should to apply the pitch angles
	float		yawClampLeft;	//how far the turret is allowed to turn left
	float		yawClampRight;	//how far the turret is allowed to turn right
	float		pitchClampUp;	//how far the turret is allowed to title up
	float		pitchClampDown; //how far the turret is allowed to tilt down
	int			iMuzzle[MAX_VEHICLE_TURRET_MUZZLES];//iMuzzle-1 = index of ship's muzzle to fire this turret's 1st and 2nd shots from
	char		*gunnerViewTag;//Where to put the view origin of the gunner (name)
	float		fTurnSpeed;	//how quickly the turret can turn
	qboolean	bAI;	//whether or not the turret auto-targets enemies when it's not manned
	qboolean	bAILead;//whether
	float		fAIRange;	//how far away the AI will look for enemies
	int			passengerNum;//which passenger, if any, has control of this turret (overrides AI)
};

struct vehWeaponStats_t {
//*** IMPORTANT!!! *** See note at top of next structure!!! ***
	// Weapon stuff.
	int			ID;//index into the weapon data
	// The delay between shots for each weapon.
	int			delay;
	// Whether or not all the muzzles for each weapon can be linked together (linked delay = weapon delay * number of muzzles linked!)
	int			linkable;
	// Whether or not to auto-aim the projectiles/tracelines at the thing under the crosshair when we fire
	qboolean	aimCorrect;
	//maximum ammo
	int			ammoMax;
	//ammo recharge rate - milliseconds per unit (minimum of 100, which is 10 ammo per second)
	int			ammoRechargeMS;
	//sound to play when out of ammo (plays default "no ammo" sound if none specified)
	int			soundNoAmmo;
};

typedef enum {
	VH_NONE = 0,	//0 just in case anyone confuses VH_NONE and VEHICLE_NONE below
	VH_WALKER,		//something you ride inside of, it walks like you, like an AT-ST
	VH_FIGHTER,		//something you fly inside of, like an X-Wing or TIE fighter
	VH_SPEEDER,		//something you ride on that hovers, like a speeder or swoop
	VH_ANIMAL,		//animal you ride on top of that walks, like a tauntaun
	VH_FLIER,		//animal you ride on top of that flies, like a giant mynoc?
	VH_NUM_VEHICLES
} vehicleType_t;

struct bgEntity_t {
	entityState_t	s;
	playerState_t	*playerState;
	Vehicle_t		*m_pVehicle; //vehicle data
	void			*ghoul2; //g2 instance
	int				localAnimIndex; //index locally (game/cgame) to anim data for this skel
	vec3_t			modelScale; //needed for g2 collision

	//Data type(s) must directly correspond to the head of the gentity and centity structures
};

struct vehWeaponStatus_t {
	//linked firing mode
	qboolean	linked;//weapon 1's muzzles are in linked firing mode
	//current weapon ammo
	int			ammo;
	//debouncer for ammo recharge
	int			lastAmmoInc;
	//which muzzle will fire next
	int			nextMuzzle;
};

struct vehTurretStatus_t {
	//current weapon ammo
	int			ammo;
	//debouncer for ammo recharge
	int			lastAmmoInc;
	//which muzzle will fire next
	int			nextMuzzle;
	//which entity they're after
	int			enemyEntNum;
	//how long to hold on to our current enemy
	int			enemyHoldTime;
};

struct vehicleInfo_t {
//*** IMPORTANT!!! *** vehFields table correponds to this structure!
	char		*name;	//unique name of the vehicle

	//general data
	vehicleType_t	type;	//what kind of vehicle
	int			numHands;	//if 2 hands, no weapons, if 1 hand, can use 1-handed weapons, if 0 hands, can use 2-handed weapons
	float		lookPitch;	//How far you can look up and down off the forward of the vehicle
	float		lookYaw;	//How far you can look left and right off the forward of the vehicle
	float		length;		//how long it is - used for body length traces when turning/moving?
	float		width;		//how wide it is - used for body length traces when turning/moving?
	float		height;		//how tall it is - used for body length traces when turning/moving?
	vec3_t		centerOfGravity;//offset from origin: {forward, right, up} as a modifier on that dimension (-1.0f is all the way back, 1.0f is all the way forward)

	//speed stats
	float		speedMax;		//top speed
	float		turboSpeed;		//turbo speed
	float		speedMin;		//if < 0, can go in reverse
	float		speedIdle;		//what speed it drifts to when no accel/decel input is given
	float		accelIdle;		//if speedIdle > 0, how quickly it goes up to that speed
	float		acceleration;	//when pressing on accelerator
	float		decelIdle;		//when giving no input, how quickly it drops to speedIdle
	float		throttleSticks;	//if true, speed stays at whatever you accel/decel to, unless you turbo or brake
	float		strafePerc;		//multiplier on current speed for strafing.  If 1.0f, you can strafe at the same speed as you're going forward, 0.5 is half, 0 is no strafing

	//handling stats
	float		bankingSpeed;	//how quickly it pitches and rolls (not under player control)
	float		rollLimit;		//how far it can roll to either side
	float		pitchLimit;		//how far it can roll forward or backward
	float		braking;		//when pressing on decelerator
	float		mouseYaw;		// The mouse yaw override.
	float		mousePitch;		// The mouse pitch override.
	float		turningSpeed;	//how quickly you can turn
	qboolean	turnWhenStopped;//whether or not you can turn when not moving
	float		traction;		//how much your command input affects velocity
	float		friction;		//how much velocity is cut on its own
	float		maxSlope;		//the max slope that it can go up with control
	qboolean	speedDependantTurning;//vehicle turns faster the faster it's going

	//durability stats
	int			mass;			//for momentum and impact force (player mass is 10)
	int			armor;			//total points of damage it can take
	int			shields;		//energy shield damage points
	int			shieldRechargeMS;//energy shield milliseconds per point recharged
	float		toughness;		//modifies incoming damage, 1.0 is normal, 0.5 is half, etc.  Simulates being made of tougher materials/construction
	int			malfunctionArmorLevel;//when armor drops to or below this point, start malfunctioning
	int			surfDestruction; //can parts of this thing be torn off on impact? -rww

	//individual "area" health -rww
	int			health_front;
	int			health_back;
	int			health_right;
	int			health_left;

	//visuals & sounds
	char		*model;			//what model to use - if make it an NPC's primary model, don't need this?
	char		*skin;			//what skin to use - if make it an NPC's primary model, don't need this?
	int			g2radius;		//render radius for the ghoul2 model
	int			riderAnim;		//what animation the rider uses
	int			radarIconHandle;//what icon to show on radar in MP
	int			dmgIndicFrameHandle;//what image to use for the frame of the damage indicator
	int			dmgIndicShieldHandle;//what image to use for the shield of the damage indicator
	int			dmgIndicBackgroundHandle;//what image to use for the background of the damage indicator
	int			iconFrontHandle;//what image to use for the front of the ship on the damage indicator
	int			iconBackHandle;	//what image to use for the back of the ship on the damage indicator
	int			iconRightHandle;//what image to use for the right of the ship on the damage indicator
	int			iconLeftHandle;	//what image to use for the left of the ship on the damage indicator
	int			crosshairShaderHandle;//what image to use for the left of the ship on the damage indicator
	int			shieldShaderHandle;//What shader to use when drawing the shield shell
	char		*droidNPC;		//NPC to attach to *droidunit tag (if it exists in the model)

	int			soundOn;		//sound to play when get on it
	int			soundTakeOff;	//sound to play when ship takes off
	int			soundEngineStart;//sound to play when ship's thrusters first activate
	int			soundLoop;		//sound to loop while riding it
	int			soundSpin;		//sound to loop while spiraling out of control
	int			soundTurbo;		//sound to play when turbo/afterburner kicks in
	int			soundHyper;		//sound to play when ship lands
	int			soundLand;		//sound to play when ship lands
	int			soundOff;		//sound to play when get off
	int			soundFlyBy;		//sound to play when they buzz you
	int			soundFlyBy2;	//alternate sound to play when they buzz you
	int			soundShift1;	//sound to play when accelerating
	int			soundShift2;	//sound to play when accelerating
	int			soundShift3;	//sound to play when decelerating
	int			soundShift4;	//sound to play when decelerating

	int			iExhaustFX;		//exhaust effect, played from "*exhaust" bolt(s)
	int			iTurboFX;		//turbo exhaust effect, played from "*exhaust" bolt(s) when ship is in "turbo" mode
	int			iTurboStartFX;	//turbo begin effect, played from "*exhaust" bolts when "turbo" mode begins
	int			iTrailFX;		//trail effect, played from "*trail" bolt(s)
	int			iImpactFX;		//impact effect, for when it bumps into something
	int			iExplodeFX;		//explosion effect, for when it blows up (should have the sound built into explosion effect)
	int			iWakeFX;			//effect it makes when going across water
	int			iDmgFX;			//effect to play on damage from a weapon or something
	int			iInjureFX;
	int			iNoseFX;		//effect for nose piece flying away when blown off
	int			iLWingFX;		//effect for left wing piece flying away when blown off
	int			iRWingFX;		//effect for right wing piece flying away when blown off

	//Weapon stats
	vehWeaponStats_t	weapon[MAX_VEHICLE_WEAPONS];

	// Which weapon a muzzle fires (has to match one of the weapons this vehicle has). So 1 would be weapon 1,
	// 2 would be weapon 2 and so on.
	int			weapMuzzle[MAX_VEHICLE_MUZZLES];

	//turrets (if any) on the vehicle
	turretStats_t	turret[MAX_VEHICLE_TURRETS];

	// The max height before this ship (?) starts (auto)landing.
	float		landingHeight;

	//other misc stats
	int			gravity;		//normal is 800
	float		hoverHeight;	//if 0, it's a ground vehicle
	float		hoverStrength;	//how hard it pushes off ground when less than hover height... causes "bounce", like shocks
	qboolean	waterProof;		//can drive underwater if it has to
	float		bouyancy;		//when in water, how high it floats (1 is neutral bouyancy)
	int			fuelMax;		//how much fuel it can hold (capacity)
	int			fuelRate;		//how quickly is uses up fuel
	int			turboDuration;	//how long turbo lasts
	int			turboRecharge;	//how long turbo takes to recharge
	int			visibility;		//for sight alerts
	int			loudness;		//for sound alerts
	float		explosionRadius;//range of explosion
	int			explosionDamage;//damage of explosion

	int			maxPassengers;	// The max number of passengers this vehicle may have (Default = 0).
	qboolean	hideRider;		// rider (and passengers?) should not be drawn
	qboolean	killRiderOnDeath;//if rider is on vehicle when it dies, they should die
	qboolean	flammable;		//whether or not the vehicle should catch on fire before it explodes
	int			explosionDelay;	//how long the vehicle should be on fire/dying before it explodes
	//camera stuff
	qboolean	cameraOverride;	//whether or not to use all of the following 3rd person camera override values
	float		cameraRange;	//how far back the camera should be - normal is 80
	float		cameraVertOffset;//how high over the vehicle origin the camera should be - normal is 16
	float		cameraHorzOffset;//how far to left/right (negative/positive) of of the vehicle origin the camera should be - normal is 0
	float		cameraPitchOffset;//a modifier on the camera's pitch (up/down angle) to the vehicle - normal is 0
	float		cameraFOV;		//third person camera FOV, default is 80
	float		cameraAlpha;	//fade out the vehicle to this alpha (0.1-1.0f) if it's in the way of the crosshair
	qboolean	cameraPitchDependantVertOffset;//use the hacky AT-ST pitch dependant vertical offset

	//NOTE: some info on what vehicle weapon to use?  Like ATST or TIE bomber or TIE fighter or X-Wing...?

//===VEH_PARM_MAX========================================================================
//*** IMPORTANT!!! *** vehFields table correponds to this structure!

//THE FOLLOWING FIELDS are not in the vehFields table because they are internal variables, not read in from the .veh file
	int			modelIndex;		//set internally, not until this vehicle is spawned into the level

	// NOTE: Please note that most of this stuff has been converted from C++ classes to generic C.
	// This part of the structure is used to simulate inheritance for vehicles. The basic idea is that all vehicle use
	// this vehicle interface since they declare their own functions and assign the function pointer to the
	// corresponding function. Meanwhile, the base logic can still call the appropriate functions. In C++ talk all
	// of these functions (pointers) are pure virtuals and this is an abstract base class (although it cannot be
	// inherited from, only contained and reimplemented (through an object and a setup function respectively)). -AReis

	// Makes sure that the vehicle is properly animated.
	void (*AnimateVehicle)( Vehicle_t *pVeh );

	// Makes sure that the rider's in this vehicle are properly animated.
	void (*AnimateRiders)( Vehicle_t *pVeh );

	// Determine whether this entity is able to board this vehicle or not.
	qboolean (*ValidateBoard)( Vehicle_t *pVeh, bgEntity_t *pEnt );

	// Set the parent entity of this Vehicle NPC.
	void (*SetParent)( Vehicle_t *pVeh, bgEntity_t *pParentEntity );

	// Add a pilot to the vehicle.
	void (*SetPilot)( Vehicle_t *pVeh, bgEntity_t *pPilot );

	// Add a passenger to the vehicle (false if we're full).
	qboolean (*AddPassenger)( Vehicle_t *pVeh );

	// Animate the vehicle and it's riders.
	void (*Animate)( Vehicle_t *pVeh );

	// Board this Vehicle (get on). The first entity to board an empty vehicle becomes the Pilot.
	qboolean (*Board)( Vehicle_t *pVeh, bgEntity_t *pEnt );

	// Eject an entity from the vehicle.
	qboolean (*Eject)( Vehicle_t *pVeh, bgEntity_t *pEnt, qboolean forceEject );

	// Eject all the inhabitants of this vehicle.
	qboolean (*EjectAll)( Vehicle_t *pVeh );

	// Start a delay until the vehicle dies.
	void (*StartDeathDelay)( Vehicle_t *pVeh, int iDelayTime );

	// Update death sequence.
	void (*DeathUpdate)( Vehicle_t *pVeh );

	// Register all the assets used by this vehicle.
	void (*RegisterAssets)( Vehicle_t *pVeh );

	// Initialize the vehicle (should be called by Spawn?).
	qboolean (*Initialize)( Vehicle_t *pVeh );

	// Like a think or move command, this updates various vehicle properties.
	qboolean (*Update)( Vehicle_t *pVeh, const usercmd_t *pUcmd );

	// Update the properties of a Rider (that may reflect what happens to the vehicle).
	//
	//	[return]		bool			True if still in vehicle, false if otherwise.
	qboolean (*UpdateRider)( Vehicle_t *pVeh, bgEntity_t *pRider, usercmd_t *pUcmd );

	// ProcessMoveCommands the Vehicle.
	void (*ProcessMoveCommands)( Vehicle_t *pVeh );

	// ProcessOrientCommands the Vehicle.
	void (*ProcessOrientCommands)( Vehicle_t *pVeh );

	// Attachs all the riders of this vehicle to their appropriate position/tag (*driver, *pass1, *pass2, whatever...).
	void (*AttachRiders)( Vehicle_t *pVeh );

	// Make someone invisible and un-collidable.
	void (*Ghost)( Vehicle_t *pVeh, bgEntity_t *pEnt );

	// Make someone visible and collidable.
	void (*UnGhost)( Vehicle_t *pVeh, bgEntity_t *pEnt );

	// Get the pilot of this vehicle.
	const bgEntity_t *(*GetPilot)( Vehicle_t *pVeh );

	// Whether this vehicle is currently inhabited (by anyone) or not.
	qboolean (*Inhabited)( Vehicle_t *pVeh );
};

struct Vehicle_t {
	// The entity who pilots/drives this vehicle.
	// NOTE: This is redundant (since m_pParentEntity->owner _should_ be the pilot). This makes things clearer though.
	bgEntity_t *m_pPilot;

	int m_iPilotTime; //if spawnflag to die without pilot and this < level.time then die.
	int m_iPilotLastIndex; //index to last pilot
	qboolean m_bHasHadPilot; //qtrue once the vehicle gets its first pilot

	// The passengers of this vehicle.
	//bgEntity_t **m_ppPassengers;
	bgEntity_t *m_ppPassengers[VEH_MAX_PASSENGERS];

	//the droid unit NPC for this vehicle, if any
	bgEntity_t *m_pDroidUnit;

	// The number of passengers currently in this vehicle.
	int m_iNumPassengers;

	// The entity from which this NPC comes from.
	bgEntity_t *m_pParentEntity;

	// If not zero, how long to wait before we can do anything with the vehicle (we're getting on still).
	// -1 = board from left, -2 = board from right, -3 = jump/quick board.  -4 & -5 = throw off existing pilot
	int		m_iBoarding;

	// Used to check if we've just started the boarding process
	qboolean	m_bWasBoarding;

	// The speed the vehicle maintains while boarding occurs (often zero)
	vec3_t	m_vBoardingVelocity;

	// Time modifier (must only be used in ProcessMoveCommands() and ProcessOrientCommands() and is updated in Update()).
	float m_fTimeModifier;

	// Ghoul2 Animation info.
	//int m_iDriverTag;
	int m_iLeftExhaustTag;
	int m_iRightExhaustTag;
	int m_iGun1Tag;
	int m_iGun1Bone;
	int m_iLeftWingBone;
	int m_iRightWingBone;

	int m_iExhaustTag[MAX_VEHICLE_EXHAUSTS];
	int m_iMuzzleTag[MAX_VEHICLE_MUZZLES];
	int m_iDroidUnitTag;
	int	m_iGunnerViewTag[MAX_VEHICLE_TURRETS];//Where to put the view origin of the gunner (index)

	//this stuff is a little bit different from SP, because I am lazy -rww
	int m_iMuzzleTime[MAX_VEHICLE_MUZZLES];
	// These are updated every frame and represent the current position and direction for the specific muzzle.
	vec3_t m_vMuzzlePos[MAX_VEHICLE_MUZZLES], m_vMuzzleDir[MAX_VEHICLE_MUZZLES];

	// This is how long to wait before being able to fire a specific muzzle again. This is based on the firing rate
	// so that a firing rate of 10 rounds/sec would make this value initially 100 miliseconds.
	int m_iMuzzleWait[MAX_VEHICLE_MUZZLES];

	// The user commands structure.
	usercmd_t m_ucmd;

	// The direction an entity will eject from the vehicle towards.
	int m_EjectDir;

	// Flags that describe the vehicles behavior.
	unsigned long m_ulFlags;

	// NOTE: Vehicle Type ID, Orientation, and Armor MUST be transmitted over the net.

	// The ID of the type of vehicle this is.
	int m_iVehicleTypeID;

	// Current angles of this vehicle.
	//vec3_t		m_vOrientation;
	float		*m_vOrientation;
	//Yeah, since we use the SP code for vehicles, I want to use this value, but I'm going
	//to make it a pointer to a vec3_t in the playerstate for prediction's sake. -rww

	// How long you have strafed left or right (increments every frame that you strafe to right, decrements every frame you strafe left)
	int			m_fStrafeTime;

	// Previous angles of this vehicle.
	vec3_t		m_vPrevOrientation;

	// Previous viewangles of the rider
	vec3_t		m_vPrevRiderViewAngles;

	// When control is lost on a speeder, current angular velocity is stored here and applied until landing
	float		m_vAngularVelocity;

	vec3_t		m_vFullAngleVelocity;

	// Current armor and shields of your vehicle (explodes if armor to 0).
	int			m_iArmor;	//hull strength - STAT_HEALTH on NPC
	int			m_iShields;	//energy shielding - STAT_ARMOR on NPC

	//mp-specific
	int			m_iHitDebounce;

	// Timer for all cgame-FX...? ex: exhaust?
	int			m_iLastFXTime;

	// When to die.
	int			m_iDieTime;

	// This pointer is to a valid VehicleInfo (which could be an animal, speeder, fighter, whatever). This
	// contains the functions actually used to do things to this specific kind of vehicle as well as shared
	// information (max speed, type, etc...).
	vehicleInfo_t *m_pVehicleInfo;

	// This trace tells us if we're within landing height.
	trace_t m_LandTrace;

	// TEMP: The wing angles (used to animate it).
	vec3_t m_vWingAngles;

	//amount of damage done last impact
	int			m_iLastImpactDmg;

	//bitflag of surfaces that have broken off
	int			m_iRemovedSurfaces;

	int			m_iDmgEffectTime;

	// the last time this vehicle fired a turbo burst
	int			m_iTurboTime;

	//how long it should drop like a rock for after freed from SUSPEND
	int			m_iDropTime;

	int			m_iSoundDebounceTimer;

	//last time we incremented the shields
	int			lastShieldInc;

	//so we don't hold it down and toggle it back and forth
	qboolean	linkWeaponToggleHeld;

	//info about our weapons (linked, ammo, etc.)
	vehWeaponStatus_t	weaponStatus[MAX_VEHICLE_WEAPONS];
	vehTurretStatus_t	turretStatus[MAX_VEHICLE_TURRETS];

	//the guy who was previously the pilot
	bgEntity_t *	m_pOldPilot;

};

typedef enum //# lookMode_e
{
	LM_ENT = 0,
	LM_INTEREST
} lookMode_t;

typedef struct renderInfo_s
{
	//In whole degrees, How far to let the different model parts yaw and pitch
	int		headYawRangeLeft;
	int		headYawRangeRight;
	int		headPitchRangeUp;
	int		headPitchRangeDown;

	int		torsoYawRangeLeft;
	int		torsoYawRangeRight;
	int		torsoPitchRangeUp;
	int		torsoPitchRangeDown;

	int		legsFrame;
	int		torsoFrame;

	float	legsFpsMod;
	float	torsoFpsMod;

	//Fields to apply to entire model set, individual model's equivalents will modify this value
	vec3_t	customRGB;//Red Green Blue, 0 = don't apply
	int		customAlpha;//Alpha to apply, 0 = none?

	//RF?
	int			renderFlags;

	//
	vec3_t		muzzlePoint;
	vec3_t		muzzleDir;
	vec3_t		muzzlePointOld;
	vec3_t		muzzleDirOld;
	//vec3_t		muzzlePointNext;	// Muzzle point one server frame in the future!
	//vec3_t		muzzleDirNext;
	int			mPCalcTime;//Last time muzzle point was calced

	//
	float		lockYaw;//

	//
	vec3_t		headPoint;//Where your tag_head is
	vec3_t		headAngles;//where the tag_head in the torso is pointing
	vec3_t		handRPoint;//where your right hand is
	vec3_t		handLPoint;//where your left hand is
	vec3_t		crotchPoint;//Where your crotch is
	vec3_t		footRPoint;//where your right hand is
	vec3_t		footLPoint;//where your left hand is
	vec3_t		torsoPoint;//Where your chest is
	vec3_t		torsoAngles;//Where the chest is pointing
	vec3_t		eyePoint;//Where your eyes are
	vec3_t		eyeAngles;//Where your eyes face
	int			lookTarget;//Which ent to look at with lookAngles
	lookMode_t	lookMode;
	int			lookTargetClearTime;//Time to clear the lookTarget
	int			lastVoiceVolume;//Last frame's voice volume
	vec3_t		lastHeadAngles;//Last headAngles, NOT actual facing of head model
	vec3_t		headBobAngles;//headAngle offsets
	vec3_t		targetHeadBobAngles;//head bob angles will try to get to targetHeadBobAngles
	int			lookingDebounceTime;//When we can stop using head looking angle behavior
	float		legsYaw;//yaw angle your legs are actually rendering at

	//for tracking legitimate bolt indecies
	void		*lastG2; //if it doesn't match ent->ghoul2, the bolts are considered invalid.
	int			headBolt;
	int			handRBolt;
	int			handLBolt;
	int			torsoBolt;
	int			crotchBolt;
	int			footRBolt;
	int			footLBolt;
	int			motionBolt;

	int			boltValidityTime;
} renderInfo_t;

typedef struct parms_s {
	char	parm[MAX_PARMS][MAX_PARM_STRING_LENGTH];
} parms_t;

typedef enum
{
	HL_NONE = 0,
	HL_FOOT_RT,
	HL_FOOT_LT,
	HL_LEG_RT,
	HL_LEG_LT,
	HL_WAIST,
	HL_BACK_RT,
	HL_BACK_LT,
	HL_BACK,
	HL_CHEST_RT,
	HL_CHEST_LT,
	HL_CHEST,
	HL_ARM_RT,
	HL_ARM_LT,
	HL_HAND_RT,
	HL_HAND_LT,
	HL_HEAD,
	HL_GENERIC1,
	HL_GENERIC2,
	HL_GENERIC3,
	HL_GENERIC4,
	HL_GENERIC5,
	HL_GENERIC6,
	HL_MAX
} hitLocation_t;

typedef enum {
	TID_CHAN_VOICE = 0,	// Waiting for a voice sound to complete
	TID_ANIM_UPPER,		// Waiting to finish a lower anim holdtime
	TID_ANIM_LOWER,		// Waiting to finish a lower anim holdtime
	TID_ANIM_BOTH,		// Waiting to finish lower and upper anim holdtimes or normal md3 animating
	TID_MOVE_NAV,		// Trying to get to a navgoal or For ET_MOVERS
	TID_ANGLE_FACE,		// Turning to an angle or facing
	TID_BSTATE,			// Waiting for a certain bState to finish
	TID_LOCATION,		// Waiting for ent to enter a specific trigger_location
//	TID_MISSIONSTATUS,	// Waiting for player to finish reading MISSION STATUS SCREEN
	TID_RESIZE,			// Waiting for clear bbox to inflate size
	TID_SHOOT,			// Waiting for fire event
	NUM_TIDS,			// for def of taskID array
} taskID_t;

typedef enum //# bSet_e
{//This should check to matching a behavior state name first, then look for a script
	BSET_INVALID = -1,
	BSET_FIRST = 0,
	BSET_SPAWN = 0,//# script to use when first spawned
	BSET_USE,//# script to use when used
	BSET_AWAKE,//# script to use when awoken/startled
	BSET_ANGER,//# script to use when aquire an enemy
	BSET_ATTACK,//# script to run when you attack
	BSET_VICTORY,//# script to run when you kill someone
	BSET_LOSTENEMY,//# script to run when you can't find your enemy
	BSET_PAIN,//# script to use when take pain
	BSET_FLEE,//# script to use when take pain below 50% of health
	BSET_DEATH,//# script to use when killed
	BSET_DELAYED,//# script to run when self->delayScriptTime is reached
	BSET_BLOCKED,//# script to run when blocked by a friendly NPC or player
	BSET_BUMPED,//# script to run when bumped into a friendly NPC or player (can set bumpRadius)
	BSET_STUCK,//# script to run when blocked by a wall
	BSET_FFIRE,//# script to run when player shoots their own teammates
	BSET_FFDEATH,//# script to run when player kills a teammate
	BSET_MINDTRICK,//# script to run when player does a mind trick on this NPC

	NUM_BSETS
} bSet_t;

typedef enum saber_styles_e {
	SS_NONE=0,
	SS_FAST,
	SS_MEDIUM,
	SS_STRONG,
	SS_DESANN,
	SS_TAVION,
	SS_DUAL,
	SS_STAFF,
	SS_NUM_SABER_STYLES
} saber_styles_t;

typedef struct saberTrail_s {
	// Actual trail stuff
	int			inAction;	// controls whether should we even consider starting one
	int			duration;	// how long each trail seg stays in existence
	int			lastTime;	// time a saber segement was last stored
	vec3_t		base, dualbase;
	vec3_t		tip, dualtip;

	// Marks stuff
	qboolean	haveOldPos[2];
	vec3_t		oldPos[2];
	vec3_t		oldNormal[2];	// store this in case we don't have a connect-the-dots situation
								//	..then we'll need the normal to project a mark blob onto the impact point
} saberTrail_t;

typedef struct bladeInfo_s {
	qboolean		active;
	saber_colors_t	color;
	float			radius;
	float			length, lengthMax, lengthOld;
	float			desiredLength;
	vec3_t			muzzlePoint, muzzlePointOld;
	vec3_t			muzzleDir, muzzleDirOld;
	saberTrail_t	trail;
	int				hitWallDebounceTime;
	int				storageTime;
	int				extendDebounce;
} bladeInfo_t;

typedef enum {
	IT_BAD,
	IT_WEAPON,				// EFX: rotate + upscale + minlight
	IT_AMMO,				// EFX: rotate
	IT_ARMOR,				// EFX: rotate + minlight
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_POWERUP,				// instant on, timer based
							// EFX: rotate + external ring that rotates
	IT_HOLDABLE,			// single use, holdable item
							// EFX: rotate + bob
	IT_PERSISTANT_POWERUP,
	IT_TEAM
} itemType_t;

typedef struct gitem_s {
	char		*classname;	// spawning name
	char		*pickup_sound;
	char		*world_model[MAX_ITEM_MODELS];
	char		*view_model;
	char		*icon;
//	char		*pickup_name;	// for printing on pickup

	int			quantity;		// for ammo how much, or duration of powerup
	itemType_t  giType;			// IT_* flags

	int			giTag;

	char		*precaches;		// string of all models and images this item will use
	char		*sounds;		// string of all sounds this item will use
	char		*description;
} gitem_t;

typedef enum {
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
} gametype_t;

typedef enum {
	SABER_NONE = 0,
	SABER_SINGLE,
	SABER_STAFF,
	SABER_DAGGER,
	SABER_BROAD,
	SABER_PRONG,
	SABER_ARC,
	SABER_SAI,
	SABER_CLAW,
	SABER_LANCE,
	SABER_STAR,
	SABER_TRIDENT,
	SABER_SITH_SWORD,
	NUM_SABERS
} saberType_t;

typedef struct saberInfo_s {
	char			name[SABER_NAME_LENGTH];				// entry in sabers.cfg, if any
	char			fullName[SABER_NAME_LENGTH];			// the "Proper Name" of the saber, shown in UI
	saberType_t		type;									// none, single or staff
	char			model[MAX_QPATH];						// hilt model
	qhandle_t		skin;									// registered skin id
	int				soundOn;								// game soundindex for turning on sound
	int				soundLoop;								// game soundindex for hum/loop sound
	int				soundOff;								// game soundindex for turning off sound
	int				numBlades;
	bladeInfo_t		blade[MAX_BLADES];						// blade info - like length, trail, origin, dir, etc.
	int				stylesLearned;							// styles you get when you get this saber, if any
	int				stylesForbidden;						// styles you cannot use with this saber, if any
	int				maxChain;								// how many moves can be chained in a row with this weapon (-1 is infinite, 0 is use default behavior)
	int				forceRestrictions;						// force powers that cannot be used while this saber is on (bitfield) - FIXME: maybe make this a limit on the max level, per force power, that can be used with this type?
	int				lockBonus;								// in saberlocks, this type of saber pushes harder or weaker
	int				parryBonus;								// added to strength of parry with this saber
	int				breakParryBonus, breakParryBonus2;		// added to strength when hit a parry
	int				disarmBonus, disarmBonus2;				// added to disarm chance when win saberlock or have a good parry (knockaway)
	saber_styles_t	singleBladeStyle;						// makes it so that you use a different style if you only have the first blade active

	//these values are global to the saber, like all of the ones above
	int				saberFlags, saberFlags2;				// from SFL(2)_ list above

	//done in cgame (client-side code)
	qhandle_t		spinSound;								// none - if set, plays this sound as it spins when thrown
	qhandle_t		swingSound[3];							// none - if set, plays one of these 3 sounds when swung during an attack - NOTE: must provide all 3!!!

	//done in game (server-side code)
	float			moveSpeedScale;							// 1.0 - you move faster/slower when using this saber
	float			animSpeedScale;							// 1.0 - plays normal attack animations faster/slower

	//done in both cgame and game (BG code)
	int				kataMove;								// LS_INVALID - if set, player will execute this move when they press both attack buttons at the same time
	int				lungeAtkMove;							// LS_INVALID - if set, player will execute this move when they crouch+fwd+attack
	int				jumpAtkUpMove;							// LS_INVALID - if set, player will execute this move when they jump+attack
	int				jumpAtkFwdMove;							// LS_INVALID - if set, player will execute this move when they jump+fwd+attack
	int				jumpAtkBackMove;						// LS_INVALID - if set, player will execute this move when they jump+back+attack
	int				jumpAtkRightMove;						// LS_INVALID - if set, player will execute this move when they jump+rightattack
	int				jumpAtkLeftMove;						// LS_INVALID - if set, player will execute this move when they jump+left+attack
	int				readyAnim;								// -1 - anim to use when standing idle
	int				drawAnim;								// -1 - anim to use when drawing weapon
	int				putawayAnim;							// -1 - anim to use when putting weapon away
	int				tauntAnim;								// -1 - anim to use when hit "taunt"
	int				bowAnim;								// -1 - anim to use when hit "bow"
	int				meditateAnim;							// -1 - anim to use when hit "meditate"
	int				flourishAnim;							// -1 - anim to use when hit "flourish"
	int				gloatAnim;								// -1 - anim to use when hit "gloat"

	//***NOTE: you can only have a maximum of 2 "styles" of blades, so this next value, "bladeStyle2Start" is the number of the first blade to use these value on... all blades before this use the normal values above, all blades at and after this number use the secondary values below***
	int				bladeStyle2Start;						// 0 - if set, blades from this number and higher use the following values (otherwise, they use the normal values already set)

	//***The following can be different for the extra blades - not setting them individually defaults them to the value for the whole saber (and first blade)***

	//done in cgame (client-side code)
	int				trailStyle, trailStyle2;				// 0 - default (0) is normal, 1 is a motion blur and 2 is no trail at all (good for real-sword type mods)
	int				g2MarksShader, g2MarksShader2;			// none - if set, the game will use this shader for marks on enemies instead of the default "gfx/damage/saberglowmark"
	int				g2WeaponMarkShader, g2WeaponMarkShader2;// none - if set, the game will ry to project this shader onto the weapon when it damages a person (good for a blood splatter on the weapon)
	qhandle_t		hitSound[3], hit2Sound[3];				// none - if set, plays one of these 3 sounds when saber hits a person - NOTE: must provide all 3!!!
	qhandle_t		blockSound[3], block2Sound[3];			// none - if set, plays one of these 3 sounds when saber/sword hits another saber/sword - NOTE: must provide all 3!!!
	qhandle_t		bounceSound[3], bounce2Sound[3];		// none - if set, plays one of these 3 sounds when saber/sword hits a wall and bounces off (must set bounceOnWall to 1 to use these sounds) - NOTE: must provide all 3!!!
	int				blockEffect, blockEffect2;				// none - if set, plays this effect when the saber/sword hits another saber/sword (instead of "saber/saber_block.efx")
	int				hitPersonEffect, hitPersonEffect2;		// none - if set, plays this effect when the saber/sword hits a person (instead of "saber/blood_sparks_mp.efx")
	int				hitOtherEffect, hitOtherEffect2;		// none - if set, plays this effect when the saber/sword hits something else damagable (instead of "saber/saber_cut.efx")
	int				bladeEffect, bladeEffect2;				// none - if set, plays this effect at the blade tag

	//done in game (server-side code)
	float			knockbackScale, knockbackScale2;		// 0 - if non-zero, uses damage done to calculate an appropriate amount of knockback
	float			damageScale, damageScale2;				// 1 - scale up or down the damage done by the saber
	float			splashRadius, splashRadius2;			// 0 - radius of splashDamage
	int				splashDamage, splashDamage2;			// 0 - amount of splashDamage, 100% at a distance of 0, 0% at a distance = splashRadius
	float			splashKnockback, splashKnockback2;		// 0 - amount of splashKnockback, 100% at a distance of 0, 0% at a distance = splashRadius
} saberInfo_t;


typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

struct playerTeamState_t {
	playerTeamStateState_t	state;

	int			location;

	int			captures;
	int			basedefense;
	int			carrierdefense;
	int			flagrecovery;
	int			fragcarrier;
	int			assists;

	float		lasthurtcarrier;
	float		lastreturnedflag;
	float		flagsince;
	float		lastfraggedcarrier;
};


struct clientPersistant_t {
	clientConnected_t	connected;
	usercmd_t	cmd;				// we would lose angles if not persistant
	qboolean	localClient;		// true if "ip" info key is "localhost"
	qboolean	initialSpawn;		// the first spawn should be at a cool location
	qboolean	predictItemPickup;	// based on cg_predictItems userinfo
	qboolean	pmoveFixed;			//
	char		netname[MAX_NETNAME];
	char		netname_nocolor[MAX_NETNAME];
	int			netnameTime;				// Last time the name was changed
	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
	playerTeamState_t teamState;	// status in teamplay games
	qboolean	teamInfo;			// send team overlay updates?

	int			connectTime;

	char		saber1[MAX_QPATH], saber2[MAX_QPATH];

	int			vote, teamvote; // 0 = none, 1 = yes, 2 = no

	char		guid[33];
};

struct clientSession_t {
	team_t		sessionTeam;
	int			spectatorNum;		// for determining next-in-line to play
	spectatorState_t	spectatorState;
	int			spectatorClient;	// for chasecam and follow mode
	int			wins, losses;		// tournament stats
	int			selectedFP;			// check against this, if doesn't match value in playerstate then update userinfo
	int			saberLevel;			// similar to above method, but for current saber attack level
	int			setForce;			// set to true once player is given the chance to set force powers
	int			updateUITime;		// only update userinfo for FP/SL if < level.time
	qboolean	teamLeader;			// true when this client is a team leader
	char		siegeClass[64];
	int			duelTeam;
	int			siegeDesiredTeam;

	char		IP[NET_ADDRSTRMAXLEN];
};

struct gclient_t {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	saberInfo_t	saber[MAX_SABERS];
	void		*weaponGhoul2[MAX_SABERS];

	int			tossableItemDebounce;

	int			bodyGrabTime;
	int			bodyGrabIndex;

	int			pushEffectTime;

	int			invulnerableTimer;

	int			saberCycleQueue;

	int			legsAnimExecute;
	int			torsoAnimExecute;
	qboolean	legsLastFlip;
	qboolean	torsoLastFlip;

	qboolean	readyToExit;		// wishes to leave the intermission

	qboolean	noclip;

	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION
									// we can't just use pers.lastCommand.time, because
									// of the g_sycronousclients case
	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	vec3_t		oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector

	int			damageBoxHandle_Head; //entity number of head damage box
	int			damageBoxHandle_RLeg; //entity number of right leg damage box
	int			damageBoxHandle_LLeg; //entity number of left leg damage box

	int			accurateCount;		// for "impressive" reward sound

	int			accuracy_shots;		// total number of shots
	int			accuracy_hits;		// total number of hits

	//
	int			lastkilled_client;	// last client that this client killed
	int			lasthurt_client;	// last client that damaged this client
	int			lasthurt_mod;		// type of damage the client did

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	int			inactivityTime;		// kick players when time > this
	qboolean	inactivityWarning;	// qtrue if the five seoond warning has been given
	int			rewardTime;			// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int			airOutTime;

	int			lastKillTime;		// for multiple kill rewards

	qboolean	fireHeld;			// used for hook
	gentity_t	*hook;				// grapple hook if out

	int			switchTeamTime;		// time the player switched teams

	int			switchDuelTeamTime;		// time the player switched duel teams

	int			switchClassTime;	// class changed debounce timer

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;

	char		*areabits;

	int			g2LastSurfaceHit; //index of surface hit during the most recent ghoul2 collision performed on this client.
	int			g2LastSurfaceTime; //time when the surface index was set (to make sure it's up to date)

	int			corrTime;

	vec3_t		lastHeadAngles;
	int			lookTime;

	int			brokenLimbs;

	qboolean	noCorpse; //don't leave a corpse on respawn this time.

	int			jetPackTime;

	qboolean	jetPackOn;
	int			jetPackToggleTime;
	int			jetPackDebRecharge;
	int			jetPackDebReduce;

	int			cloakToggleTime;
	int			cloakDebRecharge;
	int			cloakDebReduce;

	int			saberStoredIndex; //stores saberEntityNum from playerstate for when it's set to 0 (indicating saber was knocked out of the air)

	int			saberKnockedTime; //if saber gets knocked away, can't pull it back until this value is < level.time

	vec3_t		olderSaberBase; //Set before lastSaberBase_Always, to whatever lastSaberBase_Always was previously
	qboolean	olderIsValid;	//is it valid?

	vec3_t		lastSaberDir_Always; //every getboltmatrix, set to saber dir
	vec3_t		lastSaberBase_Always; //every getboltmatrix, set to saber base
	int			lastSaberStorageTime; //server time that the above two values were updated (for making sure they aren't out of date)

	qboolean	hasCurrentPosition;	//are lastSaberTip and lastSaberBase valid?

	int			dangerTime;		// level.time when last attack occured

	int			idleTime;		//keep track of when to play an idle anim on the client.

	int			idleHealth;		//stop idling if health decreases
	vec3_t		idleViewAngles;	//stop idling if viewangles change

	int			forcePowerSoundDebounce; //if > level.time, don't do certain sound events again (drain sound, absorb sound, etc)

	char		modelname[MAX_QPATH];

	qboolean	fjDidJump;

	qboolean	ikStatus;

	int			throwingIndex;
	int			beingThrown;
	int			doingThrow;

	float		hiddenDist;//How close ents have to be to pick you up as an enemy
	vec3_t		hiddenDir;//Normalized direction in which NPCs can't see you (you are hidden)

	renderInfo_t	renderInfo;

	//mostly NPC stuff:
	npcteam_t	playerTeam;
	npcteam_t	enemyTeam;
	char		*squadname;
	gentity_t	*team_leader;
	gentity_t	*leader;
	gentity_t	*follower;
	int			numFollowers;
	gentity_t	*formationGoal;
	int			nextFormGoal;
	class_t		NPC_class;

	vec3_t		pushVec;
	int			pushVecTime;

	int			siegeClass;
	int			holdingObjectiveItem;

	//time values for when being healed/supplied by supplier class
	int			isMedHealed;
	int			isMedSupplied;

	//seperate debounce time for refilling someone's ammo as a supplier
	int			medSupplyDebounce;

	//used in conjunction with ps.hackingTime
	int			isHacking;
	vec3_t		hackingAngles;

	//debounce time for sending extended siege data to certain classes
	int			siegeEDataSend;

	int			ewebIndex; //index of e-web gun if spawned
	int			ewebTime; //e-web use debounce
	int			ewebHealth; //health of e-web (to keep track between deployments)

	int			inSpaceIndex; //ent index of space trigger if inside one
	int			inSpaceSuffocation; //suffocation timer

	int			tempSpectate; //time to force spectator mode

	//keep track of last person kicked and the time so we don't hit multiple times per kick
	int			jediKickIndex;
	int			jediKickTime;

	//special moves (designed for kyle boss npc, but useable by players in mp)
	int			grappleIndex;
	int			grappleState;

	int			solidHack;

	int			noLightningTime;

	unsigned	mGameFlags;

	//fallen duelist
	qboolean	iAmALoser;

	int			lastGenCmd;
	int			lastGenCmdTime;

	struct force {
		int		regenDebounce;
		int		drainDebounce;
		int		lightningDebounce;
	} force;
};

struct gentity_t {
	//rww - entstate must be first, to correspond with the bg shared entity structure
	entityState_t	s;				// communicated by server to clients
	playerState_t	*playerState;	//ptr to playerstate if applicable (for bg ents)
	Vehicle_t		*m_pVehicle; //vehicle data
	void			*ghoul2; //g2 instance
	int				localAnimIndex; //index locally (game/cgame) to anim data for this skel
	vec3_t			modelScale; //needed for g2 collision

	//From here up must be the same as centity_t/bgEntity_t

	entityShared_t	r;				// shared by both the server system and game

	//rww - these are shared icarus things. They must be in this order as well in relation to the entityshared structure.
	int				taskID[NUM_TIDS];
	parms_t			*parms;
	char			*behaviorSet[NUM_BSETS];
	char			*script_targetname;
	int				delayScriptTime;
	char			*fullName;

	//rww - targetname and classname are now shared as well. ICARUS needs access to them.
	char			*targetname;
	char			*classname;			// set in QuakeEd

	//rww - and yet more things to share. This is because the nav code is in the exe because it's all C++.
	int				waypoint;			//Set once per frame, if you've moved, and if someone asks
	int				lastWaypoint;		//To make sure you don't double-back
	int				lastValidWaypoint;	//ALWAYS valid -used for tracking someone you lost
	int				noWaypointTime;		//Debouncer - so don't keep checking every waypoint in existance every frame that you can't find one
	int				combatPoint;
	int				failedWaypoints[MAX_FAILED_NODES];
	int				failedWaypointCheckTime;

	int				next_roff_time; //rww - npc's need to know when they're getting roff'd

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	gclient_t	*client;			// NULL if not a client

	gNPC_t		*NPC;//Only allocated if the entity becomes an NPC
	int			cantHitEnemyCounter;//HACK - Makes them look for another enemy on the same team if the one they're after can't be hit

	qboolean	noLumbar; //see note in cg_local.h

	qboolean	inuse;

	int			lockCount; //used by NPCs

	int			spawnflags;			// set in QuakeEd

	int			teamnodmg;			// damage will be ignored if it comes from this team

	char		*roffname;			// set in QuakeEd
	char		*rofftarget;		// set in QuakeEd

	char		*healingclass; //set in quakeed
	char		*healingsound; //set in quakeed
	int			healingrate; //set in quakeed
	int			healingDebounce; //debounce for generic object healing shiz

	char		*ownername;

	int			objective;
	int			side;

	int			passThroughNum;		// set to index to pass through (+1) for missiles

	int			aimDebounceTime;
	int			painDebounceTime;
	int			attackDebounceTime;
	int			alliedTeam;			// only useable by this team, never target this team

	int			roffid;				// if roffname != NULL then set on spawn

	qboolean	neverFree;			// if true, FreeEntity will only unlink
									// bodyque uses this

	int			flags;				// FL_* variables

	char		*model;
	char		*model2;
	int			freetime;			// level.time when the object was freed

	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;

	qboolean	physicsObject;		// if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects,
	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

//Only used by NPC_spawners
	char		*NPC_type;
	char		*NPC_targetname;
	char		*NPC_target;

	// movers
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	gentity_t	*parent;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	vec3_t		pos1, pos2;

	//for npc's
	vec3_t		pos3;

	char		*message;

	int			timestamp;		// body queue sinking, etc

	float		angle;			// set in editor, -1 = up, -2 = down
	char		*target;
	char		*target2;
	char		*target3;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*target4;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*target5;		//mainly added for siege items
	char		*target6;		//mainly added for siege items

	char		*team;
	char		*targetShaderName;
	char		*targetShaderNewName;
	gentity_t	*target_ent;

	char		*closetarget;
	char		*opentarget;
	char		*paintarget;

	char		*goaltarget;
	char		*idealclass;

	float		radius;

	int			maxHealth; //used as a base for crosshair health display

	float		speed;
	vec3_t		movedir;
	float		mass;
	int			setTime;

//Think Functions
	int			nextthink;
	void		(*think)(gentity_t *self);
	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
	void		(*blocked)(gentity_t *self, gentity_t *other);
	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage);
	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

	int			pain_debounce_time;
	int			fly_sound_debounce_time;	// wind tunnel
	int			last_move_time;

//Health and damage fields
	int			health;
	qboolean	takedamage;
	material_t	material;

	int			damage;
	int			dflags;
	int			splashDamage;	// quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;

	int			locationDamage[HL_MAX];		// Damage accumulated on different body locations

	int			count;
	int			bounceCount;
	qboolean	alt_fire;

	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*lastEnemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team

	int			watertype;
	int			waterlevel;

	int			noise_index;

	// timing variables
	float		wait;
	float		random;
	int			delay;

	//generic values used by various entities for different purposes.
	int			genericValue1;
	int			genericValue2;
	int			genericValue3;
	int			genericValue4;
	int			genericValue5;
	int			genericValue6;
	int			genericValue7;
	int			genericValue8;
	int			genericValue9;
	int			genericValue10;
	int			genericValue11;
	int			genericValue12;
	int			genericValue13;
	int			genericValue14;
	int			genericValue15;

	char		*soundSet;

	qboolean	isSaberEntity;

	int			damageRedirect; //if entity takes damage, redirect to..
	int			damageRedirectTo; //this entity number

	vec3_t		epVelocity;
	float		epGravFactor;

	gitem_t		*item;			// for bonus items

	// OpenJK add
	int			useDebounceTime;	// for cultist_destroyer
};
