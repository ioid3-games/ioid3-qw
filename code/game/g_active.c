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

/*
=======================================================================================================================================
G_DamageFeedback

Called just before a snapshot is sent to the given player. Totals up all damage and generates both the player_state_t damage values to
that client for pain blends and kicks, and global pain sound events for all clients.
=======================================================================================================================================
*/
void G_DamageFeedback(gentity_t *player) {
	gclient_t *client;
	float count;
	vec3_t angles;

	client = player->client;

	if (client->ps.pm_type == PM_DEAD) {
		return;
	}
	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;

	if (count == 0) {
		return; // didn't take any damage
	}

	if (count > 255) {
		count = 255;
	}
	// send the information to the client

	// world damage (falling, slime, etc.) uses a special code to make the blend blob centered instead of positional
	if (client->damage_fromWorld) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;
		client->damage_fromWorld = qfalse;
	} else {
		vectoangles(client->damage_from, angles);
		client->ps.damagePitch = angles[PITCH] / 360.0 * 256;
		client->ps.damageYaw = angles[YAW] / 360.0 * 256;
	}
	// play an appropriate pain sound
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE)) {
		player->pain_debounce_time = level.time + 700;
		G_AddEvent(player, EV_PAIN, player->health);
		client->ps.damageEvent++;
	}

	client->ps.damageCount = count;
	// clear totals
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}

/*
=======================================================================================================================================
G_WorldEffects

Check for lava/slime contents and drowning.
=======================================================================================================================================
*/
void G_WorldEffects(gentity_t *ent) {
	int waterlevel;

	if (ent->client->noclip) {
		ent->client->airOutTime = level.time + 30000; // don't need air
		return;
	}

	waterlevel = ent->waterlevel;
	// check for drowning
	if (waterlevel == 3) {
		// regeneration powerup gives air
		if (ent->client->ps.powerups[PW_REGEN] > level.time) {
			ent->client->airOutTime = level.time + 10000;
		}
		// if out of air, start drowning
		if (ent->client->airOutTime < level.time) {
			// drown!
			ent->client->airOutTime += 1000;

			if (ent->health > 0) {
				// take more damage the longer underwater
				ent->damage += 2;

				if (ent->damage > 15) {
					ent->damage = 15;
				}
				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage(ent, NULL, NULL, NULL, NULL, ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	} else {
		ent->client->airOutTime = level.time + 30000;
		ent->damage = 2;
	}
	// check for sizzle damage (move to pmove?)
	if (waterlevel && (ent->watertype &(CONTENTS_LAVA|CONTENTS_SLIME))) {
		if (ent->health > 0 && ent->pain_debounce_time <= level.time) {
			if (ent->watertype & CONTENTS_LAVA) {
				G_Damage(ent, NULL, NULL, NULL, NULL, 30 * waterlevel, 0, MOD_LAVA);
			}

			if (ent->watertype & CONTENTS_SLIME) {
				G_Damage(ent, NULL, NULL, NULL, NULL, 10 * waterlevel, 0, MOD_SLIME);
			}
		}
	}
}

/*
=======================================================================================================================================
G_SetClientSound
=======================================================================================================================================
*/
void G_SetClientSound(gentity_t *ent) {

	if (ent->s.eFlags & EF_TICKING) {
		ent->client->ps.loopSound = G_SoundIndex("sound/weapons/proxmine/wstbtick.wav");
	} else if (ent->waterlevel && (ent->watertype &(CONTENTS_LAVA|CONTENTS_SLIME))) {
		ent->client->ps.loopSound = level.snd_fry;
	} else {
		ent->client->ps.loopSound = 0;
	}
}

/*
=======================================================================================================================================
ClientImpacts
=======================================================================================================================================
*/
void ClientImpacts(gentity_t *ent, pmove_t *pm) {
	int i, j;
	trace_t trace;
	gentity_t *other;

	memset(&trace, 0, sizeof(trace));

	for (i = 0; i < pm->numtouch; i++) {
		for (j = 0; j < i; j++) {
			if (pm->touchents[j] == pm->touchents[i]) {
				break;
			}
		}

		if (j != i) {
			continue; // duplicated
		}

		other = &g_entities[pm->touchents[i]];

		if ((ent->r.svFlags & SVF_BOT) && (ent->touch)) {
			ent->touch(ent, other, &trace);
		}

		if (!other->touch) {
			continue;
		}

		other->touch(other, ent, &trace);
	}
}

/*
=======================================================================================================================================
G_TouchTriggers

Find all trigger entities that ent's current position touches. Spectators will only interact with teleporters.
=======================================================================================================================================
*/
void G_TouchTriggers(gentity_t *ent) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t *hit;
	trace_t trace;
	vec3_t mins, maxs;
	static vec3_t range = {40, 40, 52};

	if (!ent->client) {
		return;
	}
	// dead clients don't activate triggers!
	if (ent->client->ps.stats[STAT_HEALTH] <= 0) {
		return;
	}

	VectorSubtract(ent->client->ps.origin, range, mins);
	VectorAdd(ent->client->ps.origin, range, maxs);

	num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);
	// can't use ent->absmin, because that has a one unit pad
	VectorAdd(ent->client->ps.origin, ent->r.mins, mins);
	VectorAdd(ent->client->ps.origin, ent->r.maxs, maxs);

	for (i = 0; i < num; i++) {
		hit = &g_entities[touch[i]];

		if (!hit->touch && !ent->touch) {
			continue;
		}

		if (!(hit->r.contents & CONTENTS_TRIGGER)) {
			continue;
		}
		// ignore most entities if a spectator
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
			if (hit->s.eType != ET_TELEPORT_TRIGGER && hit->touch != Touch_DoorTrigger) { // this is ugly but adding a new ET_? type will most likely cause network incompatibilities
				continue;
			}
		}
		// use separate code for determining if an item is picked up so you don't have to actually contact its bounding box
		if (hit->s.eType == ET_ITEM) {
			if (!BG_PlayerTouchesItem(&ent->client->ps, &hit->s, level.time)) {
				continue;
			}
		} else {
			if (!trap_EntityContact(mins, maxs, hit)) {
				continue;
			}
		}

		memset(&trace, 0, sizeof(trace));

		if (hit->touch) {
			hit->touch(hit, ent, &trace);
		}

		if ((ent->r.svFlags & SVF_BOT) && (ent->touch)) {
			ent->touch(ent, hit, &trace);
		}
	}
	// if we didn't touch a jump pad this pmove frame
	if (ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount) {
		ent->client->ps.jumppad_frame = 0;
		ent->client->ps.jumppad_ent = 0;
	}
}

/*
=======================================================================================================================================
SpectatorThink
=======================================================================================================================================
*/
void SpectatorThink(gentity_t *ent, usercmd_t *ucmd) {
	pmove_t pm;
	gclient_t *client;

	client = ent->client;

	if (client->sess.spectatorState != SPECTATOR_FOLLOW) {
		if (client->noclip) {
			client->ps.pm_type = PM_NOCLIP;
		} else {
			client->ps.pm_type = PM_SPECTATOR;
		}

		client->ps.speed = 400; // faster than normal
		// set up for pmove
		memset(&pm, 0, sizeof(pm));

		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY; // spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;
		// perform a pmove
		Pmove(&pm);
		// save results of pmove
		VectorCopy(client->ps.origin, ent->s.origin);
		G_TouchTriggers(ent);
		trap_UnlinkEntity(ent);
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	// attack button cycles through spectators
	if ((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK)) {
		Cmd_FollowCycle_f(ent, 1);
	}
}

/*
=======================================================================================================================================
ClientInactivityTimer

Returns qfalse if the client is dropped.
=======================================================================================================================================
*/
qboolean ClientInactivityTimer(gclient_t *client) {

	if (!g_inactivity.integer) {
		// give everyone some time, so if the operator sets g_inactivity during gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if (client->pers.cmd.forwardmove || client->pers.cmd.rightmove || client->pers.cmd.upmove || (client->pers.cmd.buttons & BUTTON_ATTACK)) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	} else if (!client->pers.localClient) {
		if (level.time > client->inactivityTime) {
			trap_DropClient(client - level.clients, "Dropped due to inactivity");
			return qfalse;
		}

		if (level.time > client->inactivityTime - 10000 && !client->inactivityWarning) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand(client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"");
		}
	}

	return qtrue;
}

/*
=======================================================================================================================================
ClientTimerActions

Actions that happen once a second.
=======================================================================================================================================
*/
void ClientTimerActions(gentity_t *ent, int msec) {
	gclient_t *client;
	int maxHealth, addHealth;

	client = ent->client;
	client->timeResidual += msec;

	maxHealth = 0;
	addHealth = 0;

	while (client->timeResidual >= 1000) {
		client->timeResidual -= 1000;
		// regenerate
		if (client->ps.powerups[PW_REGEN]) {
			maxHealth = 200;
			addHealth += 10;
		}
		// guard
		if (bg_itemlist[client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD) {
			maxHealth = 200;
			addHealth += 5;
		}

		if (maxHealth) {
			if (ent->health < maxHealth * 0.5) {
				ent->health += addHealth * 2;

				if (ent->health > maxHealth * 1.1) {
					ent->health = maxHealth * 1.1;
				}

				G_AddEvent(ent, EV_POWERUP_REGEN, 0);
			} else if (ent->health < maxHealth) {
				ent->health += addHealth;

				if (ent->health > maxHealth) {
					ent->health = maxHealth;
				}

				G_AddEvent(ent, EV_POWERUP_REGEN, 0);
			}
		} else {
			// count down health when over max
			if (ent->health > 100) {
				ent->health--;
			}
			// count down armor when over max
			if (client->ps.stats[STAT_ARMOR] > 100) {
				client->ps.stats[STAT_ARMOR]--;
			}
		}
	}

	if (bg_itemlist[client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN) {
		int w, max, inc, t, i;
		int weapList[] = {WP_MACHINEGUN, WP_CHAINGUN, WP_SHOTGUN, WP_NAILGUN, WP_PROXLAUNCHER, WP_GRENADELAUNCHER, WP_NAPALMLAUNCHER, WP_ROCKETLAUNCHER, WP_BEAMGUN, WP_RAILGUN, WP_PLASMAGUN, WP_BFG};
		int weapCount = ARRAY_LEN(weapList);

		for (i = 0; i < weapCount; i++) {
			w = weapList[i];

			switch (w) {
				case WP_MACHINEGUN:
					max = 50;
					inc = 4;
					t = 1000;
					break;
				case WP_CHAINGUN:
					max = 100;
					inc = 5;
					t = 1000;
					break;
				case WP_SHOTGUN:
					max = 10;
					inc = 1;
					t = 1500;
					break;
				case WP_NAILGUN:
					max = 10;
					inc = 1;
					t = 1250;
					break;
				case WP_PROXLAUNCHER:
					max = 5;
					inc = 1;
					t = 2000;
					break;
				case WP_GRENADELAUNCHER:
					max = 10;
					inc = 1;
					t = 2000;
					break;
				case WP_NAPALMLAUNCHER:
					max = 10;
					inc = 1;
					t = 2000;
					break;
				case WP_ROCKETLAUNCHER:
					max = 10;
					inc = 1;
					t = 1750;
					break;
				case WP_BEAMGUN:
					max = 50;
					inc = 5;
					t = 1500;
					break;
				case WP_RAILGUN:
					max = 10;
					inc = 1;
					t = 1750;
					break;
				case WP_PLASMAGUN:
					max = 50;
					inc = 5;
					t = 1500;
					break;
				case WP_BFG:
					max = 10;
					inc = 1;
					t = 4000;
					break;
				default:
					max = 0;
					inc = 0;
					t = 1000;
					break;
			}

			client->ammoTimes[w] += msec;

			if (client->ps.ammo[w] >= max) {
				client->ammoTimes[w] = 0;
			}

			if (client->ammoTimes[w] >= t) {
				while (client->ammoTimes[w] >= t) {
					client->ammoTimes[w] -= t;
				}

				client->ps.ammo[w] += inc;

				if (client->ps.ammo[w] > max) {
					client->ps.ammo[w] = max;
				}
			}
		}
	}
}

/*
=======================================================================================================================================
ClientIntermissionThink
=======================================================================================================================================
*/
void ClientIntermissionThink(gclient_t *client) {

	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;
	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

	if (client->buttons & BUTTON_USE_HOLDABLE & (client->oldbuttons ^ client->buttons)) {
		// this used to be an ^1 but once a player says ready, it should stick
		client->readyToExit = 1;
	}
}

/*
=======================================================================================================================================
ClientEvents

Events will be passed on to the clients for presentation, but any server game effects are handled here.
=======================================================================================================================================
*/
void ClientEvents(gentity_t *ent, int oldEventSequence) {
	int i;
	int event;
	gclient_t *client;
	int damage;

	client = ent->client;

	if (oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS) {
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}

	for (i = oldEventSequence; i < client->ps.eventSequence; i++) {
		event = client->ps.events[i & (MAX_PS_EVENTS - 1)];

		switch (event) {
			case EV_FIRE_WEAPON:
				FireWeapon(ent);
				break;
			case EV_FALL_MEDIUM:
			case EV_FALL_FAR:
				if (ent->s.eType != ET_PLAYER) {
					break; // not in the player model
				}

				if (g_dmflags.integer & DF_NO_FALLING) {
					break;
				}

				if (event == EV_FALL_FAR) {
					damage = 10;
				} else {
					damage = 5;
				}

				ent->pain_debounce_time = level.time + 200; // no normal pain sound
				G_Damage(ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
				break;
			case EV_USE_ITEM1: // medkit
				ent->health = 100;
				break;
			case EV_USE_ITEM2: // kamikaze
				// start the kamikaze
				G_StartKamikaze(ent);
				break;
			default:
				break;
		}
	}
}

/*
=======================================================================================================================================
SendPendingPredictableEvents
=======================================================================================================================================
*/
void SendPendingPredictableEvents(playerState_t *ps) {
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if (ps->entityEventSequence < ps->eventSequence) {
		// create a temporary entity for this event which is sent to everyone except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
		event = ps->events[seq]|((ps->entityEventSequence & 3) << 8);
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity(ps->origin, event);
		number = t->s.number;

		BG_PlayerStateToEntityState(ps, &t->s, qtrue);

		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

/*
=======================================================================================================================================
ClientThink_Real

This will be called once for each client frame, which will usually be a couple times for each server frame on fast clients.
If "g_synchronousClients 1" is set, this will be called exactly once for each server frame, which makes for smooth demo recording.
=======================================================================================================================================
*/
void ClientThink_Real(gentity_t *ent) {
	gclient_t *client;
	pmove_t pm;
	int oldEventSequence;
	int msec;
	usercmd_t *ucmd;

	client = ent->client;
	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}
	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;
	// sanity check the command time to prevent speedup cheating
	if (ucmd->serverTime > level.time + 200) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n");
	}

	if (ucmd->serverTime < level.time - 1000) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n");
	}

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want to check for follow toggles
	if (msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW) {
		return;
	}

	if (msec > 200) {
		msec = 200;
	}

	if (pmove_msec.integer < 8) {
		trap_Cvar_SetValue("pmove_msec", 8);
		trap_Cvar_Update(&pmove_msec);
	} else if (pmove_msec.integer > 33) {
		trap_Cvar_SetValue("pmove_msec", 33);
		trap_Cvar_Update(&pmove_msec);
	}

	if (pmove_fixed.integer || client->pers.pmoveFixed) {
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer - 1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}
	// check for exiting intermission
	if (level.intermissiontime) {
		ClientIntermissionThink(client);
		return;
	}
	// spectators don't do much
	if (client->sess.sessionTeam == TEAM_SPECTATOR) {
		if (client->sess.spectatorState == SPECTATOR_SCOREBOARD) {
			return;
		}

		SpectatorThink(ent, ucmd);
		return;
	}
	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if (!ClientInactivityTimer(client)) {
		return;
	}

	if (client->noclip) {
		client->ps.pm_type = PM_NOCLIP;
	} else if (client->ps.stats[STAT_HEALTH] <= 0) {
		client->ps.pm_type = PM_DEAD;
	} else {
		client->ps.pm_type = PM_NORMAL;
	}

	client->ps.gravity = g_gravity.value;
	// set speed
	client->ps.speed = g_speed.value;

	if (bg_itemlist[client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT) {
		client->ps.speed *= SCOUT_SPEED_SCALE;
	}
	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset(&pm, 0, sizeof(pm));
	// check for the hit-scan gauntlet, don't let the action go through as an attack unless it actually hits something
	if (client->ps.weapon == WP_GAUNTLET && !(ucmd->buttons & BUTTON_TALK) && (ucmd->buttons & BUTTON_ATTACK) && client->ps.weaponTime <= 0) {
		pm.gauntletHit = CheckGauntletAttack(ent);
	}

	if (ent->flags & FL_FORCE_GESTURE) {
		ent->flags &= ~FL_FORCE_GESTURE;
		ent->client->pers.cmd.buttons |= BUTTON_GESTURE;
	}

	pm.ps = &client->ps;
	pm.cmd = *ucmd;

	if (pm.ps->pm_type == PM_DEAD) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	} else if (ent->r.svFlags & SVF_BOT) {
		pm.tracemask = MASK_PLAYERSOLID|CONTENTS_BOTCLIP;
	} else {
		pm.tracemask = MASK_PLAYERSOLID;
	}

	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = (g_dmflags.integer & DF_NO_FOOTSTEPS) > 0;
	pm.pmove_fixed = pmove_fixed.integer|client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	VectorCopy(client->ps.origin, client->oldOrigin);

	if (level.intermissionQueued != 0 && g_singlePlayer.integer) {
		if (level.time - level.intermissionQueued >= 1000) {
			pm.cmd.buttons = 0;
			pm.cmd.forwardmove = 0;
			pm.cmd.rightmove = 0;
			pm.cmd.upmove = 0;

			if (level.time - level.intermissionQueued >= 2000 && level.time - level.intermissionQueued <= 2500) {
				trap_Cmd_ExecuteText(EXEC_APPEND, "centerview\n");
			}

			ent->client->ps.pm_type = PM_SPINTERMISSION;
		}
	}

	Pmove(&pm);
// Tobias HACK: find a better, more natural-looking solution
	// prevent players from standing ontop of each other
	if (ent->client->ps.groundEntityNum >= 0 && ent->client->ps.groundEntityNum < MAX_CLIENTS && VectorLength(ent->client->ps.velocity) < 200) {
		// give them some random velocity
		ent->client->ps.velocity[0] += crandom() * 100;
		ent->client->ps.velocity[1] += crandom() * 100;
		ent->client->ps.velocity[2] += 100;
	}
// Tobias END
	// save results of pmove
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}

	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue);
	} else {
		BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);
	}

	SendPendingPredictableEvents(&ent->client->ps);

	if (!(ent->client->ps.eFlags & EF_FIRING)) {
		client->fireHeld = qfalse; // for grapple
	}
	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy(ent->s.pos.trBase, ent->r.currentOrigin);
	VectorCopy(pm.mins, ent->r.mins);
	VectorCopy(pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	// execute client events
	ClientEvents(ent, oldEventSequence);
	// link entity now
	trap_LinkEntity(ent);

	if (!ent->client->noclip) {
		G_TouchTriggers(ent);
	}
	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);
	// test for solid areas in the AAS file
	BotTestAAS(ent->r.currentOrigin);
	// touch other objects
	ClientImpacts(ent, &pm);
	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}
	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;
	// check for respawning
	if (client->ps.stats[STAT_HEALTH] <= 0) {
		// wait for the attack button to be pressed
		if (level.time > client->respawnTime) {
			// forcerespawn is to prevent users from waiting out powerups
			if (g_forcerespawn.integer > 0 && (level.time - client->respawnTime) > g_forcerespawn.integer * 1000) {
				ClientRespawn(ent);
				return;
			}
			// pressing attack or use is the normal respawn method
			if (ucmd->buttons & (BUTTON_ATTACK|BUTTON_USE_HOLDABLE)) {
				ClientRespawn(ent);
			}
		}

		return;
	}
	// perform once-a-second actions
	ClientTimerActions(ent, msec);
}

/*
=======================================================================================================================================
ClientThink

A new command has arrived from the client.
=======================================================================================================================================
*/
void ClientThink(int clientNum) {
	gentity_t *ent;

	ent = g_entities + clientNum;

	trap_GetUsercmd(clientNum, &ent->client->pers.cmd);
	// mark the time we got info, so we can display the phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if (!(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer) {
		ClientThink_Real(ent);
	}
}

/*
=======================================================================================================================================
G_RunClient
=======================================================================================================================================
*/
void G_RunClient(gentity_t *ent) {

	if (!(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer) {
		return;
	}

	ent->client->pers.cmd.serverTime = level.time;

	ClientThink_Real(ent);
}

/*
=======================================================================================================================================
SpectatorClientEndFrame
=======================================================================================================================================
*/
void SpectatorClientEndFrame(gentity_t *ent) {
	gclient_t *cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW) {
		int clientNum, flags;

		clientNum = ent->client->sess.spectatorClient;
		// team follow1 and team follow2 go to whatever clients are playing
		if (clientNum == -1) {
			clientNum = level.follow1;
		} else if (clientNum == -2) {
			clientNum = level.follow2;
		}

		if (clientNum >= 0) {
			cl = &level.clients[clientNum];

			if (cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR) {
				flags = (cl->ps.eFlags & ~(EF_VOTED|EF_TEAMVOTED))|(ent->client->ps.eFlags & (EF_VOTED|EF_TEAMVOTED));
				ent->client->ps = cl->ps;
				ent->client->ps.pm_flags |= PMF_FOLLOW;
				ent->client->ps.eFlags = flags;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if (ent->client->sess.spectatorClient >= 0) {
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin(ent->client - level.clients);
				}
			}
		}
	}

	if (ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

/*
=======================================================================================================================================
ClientEndFrame

Called at the end of each server frame for each connected client. A fast client will have multiple ClientThink for each ClientEndFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
=======================================================================================================================================
*/
void ClientEndFrame(gentity_t *ent) {
	int i;

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		SpectatorClientEndFrame(ent);
		return;
	}
	// turn off any expired powerups
	for (i = 0; i < MAX_POWERUPS; i++) {
		if (ent->client->ps.powerups[i] < level.time) {
			ent->client->ps.powerups[i] = 0;
		}
	}
	// set powerup for player animation
	if (bg_itemlist[ent->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN) {
		ent->client->ps.powerups[PW_AMMOREGEN] = level.time;
	}

	if (bg_itemlist[ent->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD) {
		ent->client->ps.powerups[PW_GUARD] = level.time;
	}

	if (bg_itemlist[ent->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_DOUBLER) {
		ent->client->ps.powerups[PW_DOUBLER] = level.time;
	}

	if (bg_itemlist[ent->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT) {
		ent->client->ps.powerups[PW_SCOUT] = level.time;
	}
	// save network bandwidth
#if 0
	if (!g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL) {
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear(ent->client->ps.viewangles);
	}
#endif
	// if the end of unit layout is displayed, don't give the player any normal movement attributes
	if (level.intermissiontime) {
		return;
	}
	// burn from lava, etc.
	G_WorldEffects(ent);
	// apply all the damage taken this frame
	G_DamageFeedback(ent);
	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if (level.time - ent->client->lastCmdTime > 1000) {
		ent->client->ps.eFlags |= EF_CONNECTION;
	} else {
		ent->client->ps.eFlags &= ~EF_CONNECTION;
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health; // FIXME: get rid of ent->health...

	G_SetClientSound(ent);
	// set the latest infor
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue);
	} else {
		BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);
	}

	SendPendingPredictableEvents(&ent->client->ps);
	// set the bit for the reachability area the client is currently in
	//i = trap_AAS_PointReachabilityAreaIndex(ent->client->ps.origin);
	//ent->client->areabits[i >> 3] |= 1 << (i & 7);
}
