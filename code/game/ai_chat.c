/*
=======================================================================================================================================
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Spearmint Source Code.
If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms. You should have received a copy of these additional
terms immediately following the terms and conditions of the GNU General Public License. If not, please request a copy in writing from
id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
=======================================================================================================================================
*/

#include "g_local.h"
#include "../botlib/botlib.h"
#include "../botlib/be_aas.h"
#include "../botlib/be_ea.h"
#include "../botlib/be_ai_char.h"
#include "../botlib/be_ai_chat.h"
#include "../botlib/be_ai_gen.h"
#include "../botlib/be_ai_goal.h"
#include "../botlib/be_ai_move.h"
#include "../botlib/be_ai_weap.h"
#include "ai_main.h"
#include "ai_dmq3.h"
#include "ai_chat.h"
#include "ai_cmd.h"
#include "ai_dmnet.h"
#include "ai_team.h"
#include "ai_vcmd.h"
#include "chars.h" // characteristics
#include "inv.h" // indexes into the inventory
#include "syn.h" // synonyms
#include "match.h" // string matching types and vars
#include "../../ui/menudef.h" // for the voice chats

#define TIME_BETWEENCHATTING 25

/*
=======================================================================================================================================
BotNumActivePlayers
=======================================================================================================================================
*/
int BotNumActivePlayers(void) {
	int i, num;
	char buf[MAX_INFO_STRING];

	num = 0;

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!buf[0] || !*Info_ValueForKey(buf, "n")) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}

		num++;
	}

	return num;
}

/*
=======================================================================================================================================
BotIsFirstInRankings
=======================================================================================================================================
*/
int BotIsFirstInRankings(bot_state_t *bs) {
	int i, score;
	char buf[MAX_INFO_STRING];
	playerState_t ps;

	score = bs->cur_ps.persistant[PERS_SCORE];

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!buf[0] || !*Info_ValueForKey(buf, "n")) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}

		if (BotAI_GetClientState(i, &ps) && score < ps.persistant[PERS_SCORE]) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotIsLastInRankings
=======================================================================================================================================
*/
int BotIsLastInRankings(bot_state_t *bs) {
	int i, score;
	char buf[MAX_INFO_STRING];
	playerState_t ps;

	score = bs->cur_ps.persistant[PERS_SCORE];

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!buf[0] || !*Info_ValueForKey(buf, "n")) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}

		if (BotAI_GetClientState(i, &ps) && score > ps.persistant[PERS_SCORE]) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotFirstClientInRankings
=======================================================================================================================================
*/
char *BotFirstClientInRankings(void) {
	int i, bestscore, bestclient;
	char buf[MAX_INFO_STRING];
	static char name[32];
	playerState_t ps;

	bestscore = -999999;
	bestclient = 0;

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!buf[0] || !*Info_ValueForKey(buf, "n")) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}

		if (BotAI_GetClientState(i, &ps) && ps.persistant[PERS_SCORE] > bestscore) {
			bestscore = ps.persistant[PERS_SCORE];
			bestclient = i;
		}
	}

	EasyClientName(bestclient, name, 32);
	return name;
}

/*
=======================================================================================================================================
BotLastClientInRankings
=======================================================================================================================================
*/
char *BotLastClientInRankings(void) {
	int i, worstscore, bestclient;
	char buf[MAX_INFO_STRING];
	static char name[32];
	playerState_t ps;

	worstscore = 999999;
	bestclient = 0;

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!buf[0] || !*Info_ValueForKey(buf, "n")) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}

		if (BotAI_GetClientState(i, &ps) && ps.persistant[PERS_SCORE] < worstscore) {
			worstscore = ps.persistant[PERS_SCORE];
			bestclient = i;
		}
	}

	EasyClientName(bestclient, name, 32);
	return name;
}

/*
=======================================================================================================================================
BotRandomOpponentName
=======================================================================================================================================
*/
char *BotRandomOpponentName(bot_state_t *bs) {
	int i, count;
	char buf[MAX_INFO_STRING];
	int opponents[MAX_CLIENTS], numopponents;
	static char name[32];

	numopponents = 0;
	opponents[0] = 0;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}

		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!buf[0] || !*Info_ValueForKey(buf, "n")) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}
		// skip teammates
		if (BotSameTeam(bs, i)) {
			continue;
		}

		opponents[numopponents] = i;
		numopponents++;
	}

	count = random() * numopponents;

	for (i = 0; i < numopponents; i++) {
		count--;

		if (count <= 0) {
			EasyClientName(opponents[i], name, sizeof(name));
			return name;
		}
	}

	EasyClientName(opponents[0], name, sizeof(name));
	return name;
}

/*
=======================================================================================================================================
BotMapTitle
=======================================================================================================================================
*/
char *BotMapTitle(void) {
	char info[1024];
	static char mapname[128];

	trap_GetServerinfo(info, sizeof(info));
	strncpy(mapname, Info_ValueForKey(info, "mapname"), sizeof(mapname) - 1);

	mapname[sizeof(mapname) - 1] = '\0';
	return mapname;
}

/*
=======================================================================================================================================
BotWeaponNameForMeansOfDeath
=======================================================================================================================================
*/
char *BotWeaponNameForMeansOfDeath(int mod) {

	switch (mod) {
		case MOD_GAUNTLET:
			return "Gauntlet";
		case MOD_MACHINEGUN:
			return "Machine Gun";
		case MOD_CHAINGUN:
			return "Chain Gun";
		case MOD_SHOTGUN:
			return "Shotgun";
		case MOD_NAIL:
			return "Nail Gun";
		case MOD_PROXIMITY_MINE:
			return "Proximity Launcher";
		case MOD_GRENADE_SPLASH:
			return "Grenade Launcher";
		case MOD_NAPALM:
		case MOD_NAPALM_SPLASH:
			return "Napalm Launcher";
		case MOD_ROCKET:
		case MOD_ROCKET_SPLASH:
			return "Rocket Launcher";
		case MOD_BEAMGUN:
			return "Beam Gun";
		case MOD_RAILGUN:
			return "Railgun";
		case MOD_PLASMA:
		case MOD_PLASMA_SPLASH:
			return "Plasma Gun";
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			return "BFG 10K";
		case MOD_KAMIKAZE:
			return "Kamikaze";
		default:
			return "[unknown weapon]";
	}
}

/*
=======================================================================================================================================
BotRandomWeaponName
=======================================================================================================================================
*/
char *BotRandomWeaponName(void) {
	int rnd;

	rnd = random() * 12.9;

	switch (rnd) {
		case 0:
			return "Gauntlet";
		case 1:
			return "Machine Gun";
		case 2:
			return "Chain Gun";
		case 3:
			return "Shotgun";
		case 4:
			return "Nail Gun";
		case 5:
			return "Proximity Launcher";
		case 6:
			return "Grenade Launcher";
		case 7:
			return "Napalm Launcher";
		case 8:
			return "Rocket Launcher";
		case 9:
			return "Beam Gun";
		case 10:
			return "Railgun";
		case 11:
			return "Plasma Gun";
		default:
			return "BFG 10K";
	}
}

/*
=======================================================================================================================================
BotVisibleEnemies
=======================================================================================================================================
*/
int BotVisibleEnemies(bot_state_t *bs) {
	int i;
	aas_entityinfo_t entinfo;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if on the same team
		if (BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if the entity isn't dead
		if (EntityIsDead(&entinfo)) {
			continue;
		}
		// if the enemy is invisible
		if (EntityIsInvisible(&entinfo)) {
			continue;
		}
		// check if the enemy is visible
		if (BotEntityVisible(&bs->cur_ps, 360, i)) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotValidChatPosition
=======================================================================================================================================
*/
int BotValidChatPosition(bot_state_t *bs) {
	vec3_t feet;

	// if the bot is dead all positions are valid
	if (BotIsDead(bs)) {
		return qtrue;
	}

	if (level.clients[bs->client].lasthurt_time > level.time - 1000) {
		return qfalse;
	}
	// must be on the ground
	if (bs->cur_ps.groundEntityNum != ENTITYNUM_NONE) {
		return qfalse;
	}
	// do not chat if in water, lava or slime
	VectorCopy(bs->origin, feet);

	feet[2] -= 23;

	if (trap_AAS_PointContents(feet) & (CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)) {
		return qfalse;
	}
	// the bot is in a position where it can chat
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_EnterGame
=======================================================================================================================================
*/
int BotChat_EnterGame(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}
	// don't chat in teamplay
	if (TeamPlayIsOn()) {
		return qfalse;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_ENTEREXITGAME, 0, 1);

	if (!bot_fastchat.integer) {
		if (random() > rnd) {
			return qfalse;
		}
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	if (!BotValidChatPosition(bs)) {
		return qfalse;
	}

	BotAI_BotInitialChat(bs, "game_enter", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), "[invalid var]", "[invalid var]", BotMapTitle(), NULL);

	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_ExitGame
=======================================================================================================================================
*/
int BotChat_ExitGame(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}
	// don't chat in teamplay
	if (TeamPlayIsOn()) {
		return qfalse;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_ENTEREXITGAME, 0, 1);

	if (!bot_fastchat.integer) {
		if (random() > rnd) {
			return qfalse;
		}
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	BotAI_BotInitialChat(bs, "game_exit", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), "[invalid var]", "[invalid var]", BotMapTitle(), NULL);

	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_StartLevel
=======================================================================================================================================
*/
int BotChat_StartLevel(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (BotIsObserver(bs)) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}
	// don't chat in teamplay
	if (TeamPlayIsOn()) {
		trap_EA_Command(bs->client, "vtaunt");
		return qfalse;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_STARTENDLEVEL, 0, 1);

	if (!bot_fastchat.integer) {
		if (random() > rnd) {
			return qfalse;
		}
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	BotAI_BotInitialChat(bs, "level_start", EasyClientName(bs->client, name, 32), NULL);

	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_EndLevel
=======================================================================================================================================
*/
int BotChat_EndLevel(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (BotIsObserver(bs)) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}
	// don't chat in teamplay
	if (TeamPlayIsOn()) {
		if (BotIsFirstInRankings(bs)) {
			trap_EA_Command(bs->client, "vtaunt");
		}

		return qtrue;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_STARTENDLEVEL, 0, 1);

	if (!bot_fastchat.integer) {
		if (random() > rnd) {
			return qfalse;
		}
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	if (BotIsFirstInRankings(bs)) {
		BotAI_BotInitialChat(bs, "level_end_victory", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), "[invalid var]", BotLastClientInRankings(), BotMapTitle(), NULL);
	} else if (BotIsLastInRankings(bs)) {
		BotAI_BotInitialChat(bs, "level_end_lose", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), BotFirstClientInRankings(), "[invalid var]", BotMapTitle(), NULL);
	} else {
		BotAI_BotInitialChat(bs, "level_end", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), BotFirstClientInRankings(), BotLastClientInRankings(), BotMapTitle(), NULL);
	}

	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_Death
=======================================================================================================================================
*/
int BotChat_Death(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_DEATH, 0, 1);
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}
	// if fast chatting is off
	if (!bot_fastchat.integer) {
		if (random() > rnd) {
			return qfalse;
		}
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	if (bs->lastkilledby >= 0 && bs->lastkilledby < MAX_CLIENTS) {
		EasyClientName(bs->lastkilledby, name, 32);
	} else {
		strcpy(name, "[world]");
	}

	if (TeamPlayIsOn() && BotSameTeam(bs, bs->lastkilledby)) {
		if (bs->lastkilledby == bs->client) {
			return qfalse;
		}

		BotAI_BotInitialChat(bs, "death_teammate", name, NULL);

		bs->chatto = CHAT_TEAM;
	} else {
		// don't chat in teamplay
		if (TeamPlayIsOn()) {
			trap_EA_Command(bs->client, "vtaunt");
			return qtrue;
		}

		if (bs->botdeathtype == MOD_KAMIKAZE && trap_BotNumInitialChats(bs->cs, "death_kamikaze")) {
			BotAI_BotInitialChat(bs, "death_kamikaze", name, NULL);
		} else if (bs->botdeathtype == MOD_TELEFRAG) {
			BotAI_BotInitialChat(bs, "death_telefrag", name, NULL);
		} else if (bs->botdeathtype == MOD_WATER) {
			BotAI_BotInitialChat(bs, "death_drown", BotRandomOpponentName(bs), NULL);
		} else if (bs->botdeathtype == MOD_SLIME) {
			BotAI_BotInitialChat(bs, "death_slime", BotRandomOpponentName(bs), NULL);
		} else if (bs->botdeathtype == MOD_LAVA) {
			BotAI_BotInitialChat(bs, "death_lava", BotRandomOpponentName(bs), NULL);
		} else if (bs->botdeathtype == MOD_FALLING) {
			BotAI_BotInitialChat(bs, "death_cratered", BotRandomOpponentName(bs), NULL);
		// all other suicides by own weapon
		} else if (bs->botsuicide || bs->botdeathtype == MOD_TRIGGER_HURT || bs->botdeathtype == MOD_CRUSH || bs->botdeathtype == MOD_TARGET_LASER || bs->botdeathtype == MOD_SUICIDE || bs->botdeathtype == MOD_UNKNOWN) {
			BotAI_BotInitialChat(bs, "death_suicide", BotRandomOpponentName(bs), NULL);
		} else {
			if ((bs->botdeathtype == MOD_GAUNTLET || bs->botdeathtype == MOD_RAILGUN || bs->botdeathtype == MOD_BFG || bs->botdeathtype == MOD_BFG_SPLASH) && random() < 0.5) {
				if (bs->botdeathtype == MOD_GAUNTLET) {
					BotAI_BotInitialChat(bs, "death_gauntlet", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
				} else if (bs->botdeathtype == MOD_RAILGUN) {
					BotAI_BotInitialChat(bs, "death_rail", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
				} else {
					BotAI_BotInitialChat(bs, "death_bfg", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
				}
			// choose between insult and praise
			} else if (random() < trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_INSULT, 0, 1)) {
				BotAI_BotInitialChat(bs, "death_insult", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
			} else {
				BotAI_BotInitialChat(bs, "death_praise", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
			}
		}

		bs->chatto = CHAT_ALL;
	}

	bs->lastchat_time = FloatTime();
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_Kill
=======================================================================================================================================
*/
int BotChat_Kill(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_KILL, 0, 1);
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}
	// if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd) {
			return qfalse;
		}
	}

	if (bs->lastkilledplayer == bs->client) {
		return qfalse;
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	if (!BotValidChatPosition(bs)) {
		return qfalse;
	}

	if (BotVisibleEnemies(bs)) {
		return qfalse;
	}

	if (bs->enemy >= MAX_CLIENTS) {
		return qfalse;
	}

	EasyClientName(bs->lastkilledplayer, name, 32);

	bs->chatto = CHAT_ALL;

	if (TeamPlayIsOn() && BotSameTeam(bs, bs->lastkilledplayer)) {
		BotAI_BotInitialChat(bs, "kill_teammate", name, NULL);

		bs->chatto = CHAT_TEAM;
	} else {
		// don't chat in teamplay
		if (TeamPlayIsOn()) {
			trap_EA_Command(bs->client, "vtaunt");
			return qfalse; // don't wait
		}

		if (bs->enemydeathtype == MOD_GAUNTLET) {
			BotAI_BotInitialChat(bs, "kill_gauntlet", name, NULL);
		} else if (bs->enemydeathtype == MOD_RAILGUN) {
			BotAI_BotInitialChat(bs, "kill_rail", name, NULL);
		} else if (bs->botdeathtype == MOD_KAMIKAZE && trap_BotNumInitialChats(bs->cs, "kill_kamikaze")) {
			BotAI_BotInitialChat(bs, "kill_kamikaze", name, NULL);
		} else if (bs->enemydeathtype == MOD_TELEFRAG) {
			BotAI_BotInitialChat(bs, "kill_telefrag", name, NULL);
		// choose between insult and praise
		} else if (random() < trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_INSULT, 0, 1)) {
			BotAI_BotInitialChat(bs, "kill_insult", name, NULL);
		} else {
			BotAI_BotInitialChat(bs, "kill_praise", name, NULL);
		}
	}

	bs->lastchat_time = FloatTime();
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_EnemySuicide
=======================================================================================================================================
*/
int BotChat_EnemySuicide(bot_state_t *bs) {
	char name[32];
	float rnd;

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_ENEMYSUICIDE, 0, 1);
	// don't chat in teamplay
	if (TeamPlayIsOn()) {
		return qfalse;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}
	// if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd) {
			return qfalse;
		}
	}

	if (!BotValidChatPosition(bs)) {
		return qfalse;
	}

	if (BotVisibleEnemies(bs)) {
		return qfalse;
	}

	if (bs->enemy >= MAX_CLIENTS) {
		return qfalse;
	}

	if (bs->enemy >= 0) {
		EasyClientName(bs->enemy, name, 32);
	} else {
		strcpy(name, "");
	}

	BotAI_BotInitialChat(bs, "enemy_suicide", name, NULL);

	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_HitNoDeath
=======================================================================================================================================
*/
int BotChat_HitNoDeath(bot_state_t *bs) {
	char name[32], *weap;
	float rnd;
	int lasthurt_client;
	aas_entityinfo_t entinfo;

	lasthurt_client = g_entities[bs->client].client->lasthurt_client;

	if (!lasthurt_client) {
		return qfalse;
	}

	if (lasthurt_client == bs->client) {
		return qfalse;
	}

	if (lasthurt_client < 0 || lasthurt_client >= MAX_CLIENTS) {
		return qfalse;
	}

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_HITNODEATH, 0, 1);
	// don't chat in teamplay
	if (TeamPlayIsOn()) {
		return qfalse;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}
	// if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd * 0.5) {
			return qfalse;
		}
	}

	if (!BotValidChatPosition(bs)) {
		return qfalse;
	}

	if (BotVisibleEnemies(bs)) {
		return qfalse;
	}

	if (bs->enemy >= MAX_CLIENTS) {
		return qfalse;
	}

	if (bs->enemy >= 0) {
		// get the entity information
		BotEntityInfo(bs->enemy, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			return qfalse;
		}

		if (EntityIsShooting(&entinfo)) {
			return qfalse;
		}
	}

	ClientName(lasthurt_client, name, sizeof(name));

	weap = BotWeaponNameForMeansOfDeath(g_entities[bs->client].client->lasthurt_mod);

	BotAI_BotInitialChat(bs, "hit_nodeath", name, weap, NULL);

	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_HitNoKill
=======================================================================================================================================
*/
int BotChat_HitNoKill(bot_state_t *bs) {
	char name[32], *weap;
	float rnd;
	aas_entityinfo_t entinfo;

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_HITNOKILL, 0, 1);
	// don't chat in teamplay
	if (TeamPlayIsOn()) {
		return qfalse;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}
	// if fast chat is off
	if (!bot_fastchat.integer) {
		if (random() > rnd * 0.5) {
			return qfalse;
		}
	}

	if (!BotValidChatPosition(bs)) {
		return qfalse;
	}

	if (BotVisibleEnemies(bs)) {
		return qfalse;
	}

	if (bs->enemy >= MAX_CLIENTS) {
		return qfalse;
	}

	if (bs->enemy >= 0) {
		// get the entity information
		BotEntityInfo(bs->enemy, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			return qfalse;
		}

		if (EntityIsShooting(&entinfo)) {
			return qfalse;
		}
	}

	ClientName(bs->enemy, name, sizeof(name));

	weap = BotWeaponNameForMeansOfDeath(g_entities[bs->enemy].client->lasthurt_mod);

	BotAI_BotInitialChat(bs, "hit_nokill", name, weap, NULL);

	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
=======================================================================================================================================
BotChat_Random
=======================================================================================================================================
*/
int BotChat_Random(bot_state_t *bs) {
	float rnd;
	char name[32];

	if (bot_nochat.integer) {
		return qfalse;
	}

	if (BotIsObserver(bs)) {
		return qfalse;
	}

	if (bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING) {
		return qfalse;
	}
	// don't chat in tournament mode
	if (gametype == GT_TOURNAMENT) {
		return qfalse;
	}
	// don't chat when doing something important :)
	if (bs->ltgtype == LTG_RUSHBASE || bs->ltgtype == LTG_TEAMHELP || bs->ltgtype == LTG_TEAMACCOMPANY) {
		return qfalse;
	}

	rnd = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_RANDOM, 0, 1);

	if (random() > bs->thinktime * 0.1) {
		return qfalse;
	}

	if (!bot_fastchat.integer) {
		if (random() > rnd) {
			return qfalse;
		}

		if (random() > 0.25) {
			return qfalse;
		}
	}

	if (BotNumActivePlayers() <= 1) {
		return qfalse;
	}

	if (!BotValidChatPosition(bs)) {
		return qfalse;
	}

	if (BotVisibleEnemies(bs)) {
		return qfalse;
	}

	if (bs->enemy >= MAX_CLIENTS) {
		return qfalse;
	}

	if (bs->lastkilledplayer == bs->client) {
		strcpy(name, BotRandomOpponentName(bs));
	} else {
		EasyClientName(bs->lastkilledplayer, name, sizeof(name));
	}
	// don't chat in teamplay
	if (TeamPlayIsOn()) {
		trap_EA_Command(bs->client, "vtaunt");
		return qfalse; // don't wait
	}

	if (random() < trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_MISC, 0, 1)) {
		BotAI_BotInitialChat(bs, "random_misc", BotRandomOpponentName(bs), name, "[invalid var]", "[invalid var]", BotMapTitle(), BotRandomWeaponName(), NULL);
	} else {
		BotAI_BotInitialChat(bs, "random_insult", BotRandomOpponentName(bs), name, "[invalid var]", "[invalid var]", BotMapTitle(), BotRandomWeaponName(), NULL);
	}

	bs->lastchat_time = FloatTime();
	bs->chatto = CHAT_ALL;
	return qtrue;
}

/*
=======================================================================================================================================
BotChatTest
=======================================================================================================================================
*/
void BotChatTest(bot_state_t *bs) {
	char name[32];
	char *weap;
	int num, i;

	num = trap_BotNumInitialChats(bs->cs, "game_enter");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "game_enter", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), "[invalid var]", "[invalid var]", BotMapTitle(), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "game_exit");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "game_exit", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), "[invalid var]", "[invalid var]", BotMapTitle(), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "level_start");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "level_start", EasyClientName(bs->client, name, 32), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "level_end_victory");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "level_end_victory", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), BotFirstClientInRankings(), BotLastClientInRankings(), BotMapTitle(), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "level_end_lose");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "level_end_lose", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), BotFirstClientInRankings(), BotLastClientInRankings(), BotMapTitle(), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "level_end");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "level_end", EasyClientName(bs->client, name, 32), BotRandomOpponentName(bs), BotFirstClientInRankings(), BotLastClientInRankings(), BotMapTitle(), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	EasyClientName(bs->lastkilledby, name, sizeof(name));

	num = trap_BotNumInitialChats(bs->cs, "death_drown");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_drown", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_slime");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_slime", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_lava");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_lava", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_cratered");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_cratered", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_suicide");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_suicide", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_telefrag");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_telefrag", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_gauntlet");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_gauntlet", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_rail");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_rail", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_bfg");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_bfg", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_insult");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_insult", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "death_praise");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "death_praise", name, BotWeaponNameForMeansOfDeath(bs->botdeathtype), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	EasyClientName(bs->lastkilledplayer, name, 32);

	num = trap_BotNumInitialChats(bs->cs, "kill_gauntlet");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "kill_gauntlet", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "kill_rail");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "kill_rail", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "kill_telefrag");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "kill_telefrag", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "kill_insult");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "kill_insult", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "kill_praise");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "kill_praise", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "enemy_suicide");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "enemy_suicide", name, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	ClientName(g_entities[bs->client].client->lasthurt_client, name, sizeof(name));

	weap = BotWeaponNameForMeansOfDeath(g_entities[bs->client].client->lasthurt_mod);
	num = trap_BotNumInitialChats(bs->cs, "hit_nodeath");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "hit_nodeath", name, weap, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "hit_nokill");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "hit_nokill", name, weap, NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	if (bs->lastkilledplayer == bs->client) {
		strcpy(name, BotRandomOpponentName(bs));
	} else {
		EasyClientName(bs->lastkilledplayer, name, sizeof(name));
	}

	num = trap_BotNumInitialChats(bs->cs, "random_misc");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "random_misc", BotRandomOpponentName(bs), name, "[invalid var]", "[invalid var]", BotMapTitle(), BotRandomWeaponName(), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}

	num = trap_BotNumInitialChats(bs->cs, "random_insult");

	for (i = 0; i < num; i++) {
		BotAI_BotInitialChat(bs, "random_insult", BotRandomOpponentName(bs), name, "[invalid var]", "[invalid var]", BotMapTitle(), BotRandomWeaponName(), NULL);
		trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
	}
}
