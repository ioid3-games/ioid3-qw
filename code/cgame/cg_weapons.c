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

/**************************************************************************************************************************************
 Events and effects dealing with weapons.
**************************************************************************************************************************************/

#include "cg_local.h"

/*
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	IMPACT

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

/*
=======================================================================================================================================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing.
=======================================================================================================================================
*/
void CG_MissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir) {
	qhandle_t mod;
	qhandle_t mark;
	qhandle_t shader;
	sfxHandle_t sfx;
	float markRadius;
	float light;
	vec3_t lightColor;
	localEntity_t *le;
	int r;
	qboolean alphaFade;
	qboolean isSprite;
	int lightDuration, markDuration, sfxRadius;
	vec3_t sprOrg;
	vec3_t sprVel;

	// set defaults
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 0.75f;
	lightColor[2] = 0;
	lightDuration = 600;
	isSprite = qfalse;
	markDuration = -1; // keep -1 markDuration for temporary marks
	sfxRadius = 128;

	switch (weapon) {
		case WP_HANDGUN:
		case WP_MACHINEGUN:
		case WP_HEAVY_MACHINEGUN:
			mod = cgs.media.bulletFlashModel;
			shader = cgs.media.bulletExplosionShader;
			mark = cgs.media.bulletMarkShader;
			markRadius = 2;
			markDuration = 30000;
			r = rand() & 3;

			if (r == 0) {
				sfx = cgs.media.sfx_ric1;
			} else if (r == 1) {
				sfx = cgs.media.sfx_ric2;
			} else {
				sfx = cgs.media.sfx_ric3;
			}

			break;
		case WP_CHAINGUN:
			mod = cgs.media.bulletFlashModel;
			mark = cgs.media.bulletMarkShader;
			markRadius = 4;
			markDuration = 30000;
			sfx = cgs.media.sfx_chghit;
			break;
		case WP_SHOTGUN:
			mod = cgs.media.bulletFlashModel;
			shader = cgs.media.bulletExplosionShader;
			mark = cgs.media.bulletMarkShader;
			markRadius = 2;
			markDuration = 30000;
			sfx = 0;
			break;
		case WP_NAILGUN:
			mark = cgs.media.holeMarkShader;
			markRadius = 4;
			markDuration = 30000;
			sfx = cgs.media.sfx_nghit;
			break;
		case WP_PHOSPHORGUN:
			mod = cgs.media.dishFlashModel;
			light = 30;
			shader = cgs.media.phosphorExplosionShader;
			isSprite = qtrue;
			mark = cgs.media.burnMarkShader;
			markRadius = 10;
			markDuration = 50000;
			sfx = cgs.media.sfx_rockexp;
			sfxRadius = 256;
			break;
		case WP_PROXLAUNCHER:
			mod = cgs.media.dishFlashModel;
			light = 300;
			shader = cgs.media.grenadeExplosionShader;
			isSprite = qtrue;
			mark = cgs.media.burnMarkShader;
			markRadius = 64;
			markDuration = 50000;
			sfx = cgs.media.sfx_proxexp;
			sfxRadius = 256;
			break;
		case WP_GRENADELAUNCHER:
			mod = cgs.media.dishFlashModel;
			light = 300;
			shader = cgs.media.grenadeExplosionShader;
			isSprite = qtrue;
			mark = cgs.media.burnMarkShader;
			markRadius = 64;
			markDuration = 50000;
			sfx = cgs.media.sfx_rockexp;
			sfxRadius = 256;
			break;
		case WP_NAPALMLAUNCHER:
			mod = cgs.media.dishFlashModel;
			light = 300;
			shader = cgs.media.grenadeExplosionShader;
			isSprite = qtrue;
			mark = cgs.media.burnMarkShader;
			markRadius = 64;
			sfx = cgs.media.sfx_rockexp;
			break;
		case WP_ROCKETLAUNCHER:
			mod = cgs.media.dishFlashModel;
			light = 300;
			lightDuration = 1000;
			shader = cgs.media.rocketExplosionShader;
			isSprite = qtrue;
			mark = cgs.media.burnMarkShader;
			markRadius = 64;
			markDuration = 50000;
			sfx = cgs.media.sfx_rockexp;
			sfxRadius = 256;

			if (cg_oldRocket.integer == 0) {
				// explosion sprite animation
				VectorMA(origin, 24, dir, sprOrg);
				VectorScale(dir, 64, sprVel);
				CG_ParticleExplosion("explode1", sprOrg, sprVel, 1400, 20, 30);
			}

			break;
		default:
		case WP_BEAMGUN:
			// no explosion at LG impact, it is added with the beam
			mark = cgs.media.holeMarkShader;
			markRadius = 12;
			markDuration = 30000;
			r = rand() & 3;

			if (r < 2) {
				sfx = cgs.media.sfx_lghit2;
			} else if (r == 2) {
				sfx = cgs.media.sfx_lghit1;
			} else {
				sfx = cgs.media.sfx_lghit3;
			}

			break;
		case WP_RAILGUN:
			mod = cgs.media.ringFlashModel;
			shader = cgs.media.railExplosionShader;
			mark = cgs.media.energyMarkShader;
			markRadius = 24;
			markDuration = 50000;
			sfx = cgs.media.sfx_plasmaexp;
			sfxRadius = 256;
			break;
		case WP_PLASMAGUN:
			mod = cgs.media.ringFlashModel;
			light = 200;
			lightColor[0] = 0.7f;
			lightColor[1] = 0.8f;
			lightColor[2] = 1.0f;
			shader = cgs.media.plasmaExplosionShader;
			mark = cgs.media.energyMarkShader;
			markRadius = 16;
			markDuration = 50000;
			sfx = cgs.media.sfx_plasmaexp;
			break;
		case WP_BFG:
			mod = cgs.media.dishFlashModel;
			light = 300;
			lightColor[0] = 0.65f;
			lightColor[1] = 1.0f;
			lightColor[2] = 0.7f;
			shader = cgs.media.bfgExplosionShader;
			isSprite = qtrue;
			mark = cgs.media.burnMarkShader;
			markRadius = 32;
			markDuration = 50000;
			sfx = cgs.media.sfx_rockexp;
			break;
		case WP_MISSILELAUNCHER:
			mod = cgs.media.dishFlashModel;
			light = 1000;
			lightDuration = 1000;
			shader = cgs.media.rocketExplosionShader;
			isSprite = qtrue;
			mark = cgs.media.burnMarkShader;
			markRadius = 512;
			markDuration = 60000;
			sfx = cgs.media.sfx_rockexp;
			sfxRadius = 256;
			// explosion sprite animation
			VectorMA(origin, 24, dir, sprOrg);
			VectorScale(dir, 64, sprVel);

			CG_ParticleExplosion("explode1", sprOrg, sprVel, 1400, 20, 30);
			break;
	}
	// create the explosion
	if (mod) {
		le = CG_MakeExplosion(origin, dir, mod, shader, lightDuration, isSprite);
		le->light = light;

		VectorCopy(lightColor, le->lightColor);
		// colorize with client color
		VectorCopy(cgs.clientinfo[clientNum].color1, le->color);

		le->refEntity.shaderRGBA[0] = le->color[0] * 0xff;
		le->refEntity.shaderRGBA[1] = le->color[1] * 0xff;
		le->refEntity.shaderRGBA[2] = le->color[2] * 0xff;
		le->refEntity.shaderRGBA[3] = 0xff;
	}
	// impact mark
	if (markDuration) {
		alphaFade = (mark == cgs.media.energyMarkShader); // plasma fades alpha, all others fade color

		if (weapon == WP_RAILGUN) {
			float *color;

			// colorize with client color
			color = cgs.clientinfo[clientNum].color1;

			CG_ImpactMark(mark, origin, dir, random() * 360, color[0], color[1], color[2], 1, alphaFade, markRadius, markDuration);
		} else {
			CG_ImpactMark(mark, origin, dir, random() * 360, 1, 1, 1, 1, alphaFade, markRadius, markDuration);
		}
	}
	// impact sound
	if (sfx) {
		trap_S_StartSound(origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx, sfxRadius, 255);
	}
}

/*
=======================================================================================================================================
CG_MissileHitPlayer
=======================================================================================================================================
*/
void CG_MissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum) {

	CG_Bleed(origin, entityNum);
	// some weapons will make an explosion with the blood, while others will just make the blood
	switch (weapon) {
		case WP_CHAINGUN:
		case WP_NAILGUN:
		case WP_PROXLAUNCHER:
		case WP_GRENADELAUNCHER:
		case WP_NAPALMLAUNCHER:
		case WP_ROCKETLAUNCHER:
		case WP_PLASMAGUN:
		case WP_BFG:
		case WP_MISSILELAUNCHER:
			CG_MissileHitWall(weapon, 0, origin, dir);
			break;
		default:
			break;
	}
}

/*
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	TRAILS

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

/*
=======================================================================================================================================
CG_PhosphorTrail

NOTE: Currently only a modified version of CG_Tracer
=======================================================================================================================================
*/
void CG_PhosphorTrail(vec3_t start, vec3_t end) {
	vec3_t forward, right, line, midpoint;
	polyVert_t verts[4];

	VectorSubtract(end, start, forward);

	line[0] = DotProduct(forward, cg.refdef.viewaxis[1]);
	line[1] = DotProduct(forward, cg.refdef.viewaxis[2]);

	VectorScale(cg.refdef.viewaxis[1], line[1], right);
	VectorMA(right, -line[0] * 0.15, cg.refdef.viewaxis[2], right);
	VectorNormalize(right);

	VectorMA(end, cg_tracerWidth.value, right, verts[0].xyz);
	verts[0].st[0] = 1;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA(end, -cg_tracerWidth.value, right, verts[1].xyz);
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA(start, -cg_tracerWidth.value, right, verts[2].xyz);
	verts[2].st[0] = 0;
	verts[2].st[1] = 0;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA(start, cg_tracerWidth.value, right, verts[3].xyz);
	verts[3].st[0] = 0;
	verts[3].st[1] = 1;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene(cgs.media.tracerShader, 4, verts);

	midpoint[0] = (start[0] + end[0]) * 0.5;
	midpoint[1] = (start[1] + end[1]) * 0.5;
	midpoint[2] = (start[2] + end[2]) * 0.5;
	// add the tracer sound
	trap_S_StartSound(midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound, 64, 255);
}

/*
=======================================================================================================================================
CG_NailTrail
=======================================================================================================================================
*/
static void CG_NailTrail(centity_t *ent, const weaponInfo_t *wi) {
	int step;
	vec3_t origin, lastPos;
	int t;
	int startTime, contents;
	int lastContents;
	entityState_t *es;
	vec3_t up;
	localEntity_t *smoke;

	if (cg_noProjectileTrail.integer) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;
	step = 50;
	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ((startTime + step) / step);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin, qfalse, es->effect2Time);

	contents = CG_PointContents(origin, -1);
	// if object (e.g. grenade) is stationary, don't toss up smoke
	if (es->pos.trType == TR_STATIONARY) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos, qfalse, es->effect2Time);

	lastContents = CG_PointContents(lastPos, -1);
	ent->trailTime = cg.time;

	if (contents & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA)) {
		if (contents & lastContents & CONTENTS_WATER) {
			CG_BubbleTrail(lastPos, origin, 8);
		}

		return;
	}

	for (; t <= ent->trailTime; t += step) {
		BG_EvaluateTrajectory(&es->pos, t, lastPos, qfalse, es->effect2Time);

		smoke = CG_SmokePuff(lastPos, up, wi->trailRadius, 1, 1, 1, 0.33f, wi->wiTrailTime, t, 0, 0, cgs.media.nailPuffShader);
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}
}

/*
=======================================================================================================================================
CG_RocketTrail
=======================================================================================================================================
*/
static void CG_RocketTrail(centity_t *ent, const weaponInfo_t *wi) {
	int step;
	vec3_t origin, lastPos;
	int t;
	int startTime, contents;
	int lastContents;
	entityState_t *es;
	vec3_t up;
	localEntity_t *smoke;

	if (cg_noProjectileTrail.integer) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;
	step = 50;
	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ((startTime + step) / step);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin, qfalse, es->effect2Time);

	contents = CG_PointContents(origin, -1);
	// if object (e.g. grenade) is stationary, don't toss up smoke
	if (es->pos.trType == TR_STATIONARY) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos, qfalse, es->effect2Time);

	lastContents = CG_PointContents(lastPos, -1);
	ent->trailTime = cg.time;

	if (contents & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA)) {
		if (contents & lastContents & CONTENTS_WATER) {
			CG_BubbleTrail(lastPos, origin, 8);
		}

		return;
	}

	for (; t <= ent->trailTime; t += step) {
		BG_EvaluateTrajectory(&es->pos, t, lastPos, qfalse, es->effect2Time);

		smoke = CG_SmokePuff(lastPos, up, wi->trailRadius, 1, 1, 1, 0.33f, wi->wiTrailTime, t, 0, 0, cgs.media.smokePuffShader);
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}
}

/*
=======================================================================================================================================
CG_GrenadeTrail
=======================================================================================================================================
*/
static void CG_GrenadeTrail(centity_t *ent, const weaponInfo_t *wi) {
	CG_RocketTrail(ent, wi);
}

/*
=======================================================================================================================================
CG_BeamgunBolt

Origin will be the exact tag point, which is slightly different than the muzzle point used for determining hits. The cent should be the
non-predicted cent if it is from the player, so the endpoint will reflect the simulated strike (lagging the predicted angle).
=======================================================================================================================================
*/
static void CG_BeamgunBolt(centity_t *cent, vec3_t origin) {
	trace_t trace;
	refEntity_t beam;
	vec3_t forward;
	vec3_t muzzlePoint, endPoint;
	int anim;

	if (cent->currentState.weapon != WP_BEAMGUN) {
		return;
	}

	memset(&beam, 0, sizeof(beam));
	// always shoot straight forward from our current position
	if (cent->currentState.number == cg.snap->ps.clientNum) {
		AngleVectorsForward(cg.predictedPlayerState.viewangles, forward);
		VectorCopy(cg.predictedPlayerState.origin, muzzlePoint);
	} else {
		AngleVectorsForward(cent->lerpAngles, forward);
		VectorCopy(cent->lerpOrigin, muzzlePoint);
	}

	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

	if (anim == LEGS_WALKCR || anim == LEGS_IDLECR) {
		muzzlePoint[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzlePoint[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA(muzzlePoint, 14, forward, muzzlePoint);
	// project forward by the beam gun range
	VectorMA(muzzlePoint, BEAMGUN_RANGE, forward, endPoint);
	// see if it hit a wall
	CG_Trace(&trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, cent->currentState.number, MASK_SHOT);
	// this is the endpoint
	VectorCopy(trace.endpos, beam.oldorigin);
	// use the provided origin, even though it may be slightly different than the muzzle origin
	VectorCopy(origin, beam.origin);

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;

	trap_R_AddRefEntityToScene(&beam);
	// add the impact flare if it hit something
	if (trace.fraction < 1.0) {
		vec3_t angles;
		vec3_t dir;

		VectorSubtract(beam.oldorigin, beam.origin, dir);
		VectorNormalize(dir);

		memset(&beam, 0, sizeof(beam));

		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA(trace.endpos, -16, dir, beam.origin);
		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;

		AnglesToAxis(angles, beam.axis);
		trap_R_AddRefEntityToScene(&beam);
	}
}
/*
static void CG_BeamgunBolt(centity_t *cent, vec3_t origin) {
	trace_t trace;
	refEntity_t beam;
	vec3_t forward;
	vec3_t muzzlePoint, endPoint;

	if (cent->currentState.weapon != WP_BEAMGUN) {
		return;
	}

	memset(&beam, 0, sizeof(beam));
	// find muzzle point for this frame
	VectorCopy(cent->lerpOrigin, muzzlePoint);
	AngleVectorsForward(cent->lerpAngles, forward);
	// FIXME: crouch
	muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

	VectorMA(muzzlePoint, 14, forward, muzzlePoint);
	// project forward by the beam gun range
	VectorMA(muzzlePoint, BEAMGUN_RANGE, forward, endPoint);
	// see if it hit a wall
	CG_Trace(&trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, cent->currentState.number, MASK_SHOT);
	// this is the endpoint
	VectorCopy(trace.endpos, beam.oldorigin);
	// use the provided origin, even though it may be slightly different than the muzzle origin
	VectorCopy(origin, beam.origin);

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	trap_R_AddRefEntityToScene(&beam);
	// add the impact flare if it hit something
	if (trace.fraction < 1.0) {
		vec3_t angles;
		vec3_t dir;

		VectorSubtract(beam.oldorigin, beam.origin, dir);
		VectorNormalize(dir);

		memset(&beam, 0, sizeof(beam));

		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA(trace.endpos, -16, dir, beam.origin);
		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis(angles, beam.axis);
		trap_R_AddRefEntityToScene(&beam);
	}
}
*/
/*
=======================================================================================================================================
CG_RailTrail
=======================================================================================================================================
*/
void CG_RailTrail(clientInfo_t *ci, vec3_t start, vec3_t end) {
	vec3_t axis[36], move, move2, vec, temp;
	float len;
	int i, j, skip;
	localEntity_t *le;
	refEntity_t *re;
#define RADIUS 4
#define ROTATION 1
#define SPACING 5
	start[2] -= 4;

	le = CG_AllocLocalEntity();

	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;

	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);

	re->shaderRGBA[0] = ci->color1[0] * 255;
	re->shaderRGBA[1] = ci->color1[1] * 255;
	re->shaderRGBA[2] = ci->color1[2] * 255;
	re->shaderRGBA[3] = 255;

	le->color[0] = ci->color1[0] * 0.75;
	le->color[1] = ci->color1[1] * 0.75;
	le->color[2] = ci->color1[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear(re->axis);

	if (cg_oldRail.integer) {
		// reimplementing the rail discs(removed in 1.30)
		if (cg_oldRail.integer > 1) {
			le = CG_AllocLocalEntity();
			re = &le->refEntity;

			VectorCopy(start, re->origin);
			VectorCopy(end, re->oldorigin);

			le->leType = LE_FADE_RGB;
			le->startTime = cg.time;
			le->endTime = cg.time + cg_railTrailTime.value;
			le->lifeRate = 1.0 / (le->endTime - le->startTime);

			re->shaderTime = cg.time / 1000.0f;
			re->reType = RT_RAIL_RINGS;
			re->customShader = cgs.media.railRingsShader;
			re->shaderRGBA[0] = ci->color1[0] * 255;
			re->shaderRGBA[1] = ci->color1[1] * 255;
			re->shaderRGBA[2] = ci->color1[2] * 255;
			re->shaderRGBA[3] = 255;
	
			le->color[0] = ci->color1[0] * 0.75;
			le->color[1] = ci->color1[1] * 0.75;
			le->color[2] = ci->color1[2] * 0.75;
			le->color[3] = 1.0f;
			// alternatively, use the secondary color
			if (cg_oldRail.integer > 2) {
				re->shaderRGBA[0] = ci->color2[0] * 255;
				re->shaderRGBA[1] = ci->color2[1] * 255;
				re->shaderRGBA[2] = ci->color2[2] * 255;
				re->shaderRGBA[3] = 255;
		
				le->color[0] = ci->color2[0] * 0.75;
				le->color[1] = ci->color2[1] * 0.75;
				le->color[2] = ci->color2[2] * 0.75;
				le->color[3] = 1.0f;
			}
		}

		return;
	}

	VectorCopy(start, move);
	VectorSubtract(end, start, vec);

	len = VectorNormalize(vec);

	PerpendicularVector(temp, vec);

	for (i = 0; i < 36; i++) {
		RotatePointAroundVector(axis[i], vec, temp, i * 10); // banshee 2.4 was 10
	}

	VectorMA(move, 20, vec, move);
	VectorScale(vec, SPACING, vec);

	skip = -1;
	j = 18;

	for (i = 0; i < len; i += SPACING) {
		if (i != skip) {
			skip = i + SPACING;

			le = CG_AllocLocalEntity();

			re = &le->refEntity;

			le->leFlags = LEF_PUFF_DONT_SCALE;
			le->leType = LE_MOVE_SCALE_FADE;
			le->startTime = cg.time;
			le->endTime = cg.time + (i >> 1) + 600;
			le->lifeRate = 1.0 / (le->endTime - le->startTime);

			re->shaderTime = cg.time / 1000.0f;
			re->reType = RT_SPRITE;
			re->radius = 1.1f;
			re->customShader = cgs.media.railRingsShader;
			re->shaderRGBA[0] = ci->color2[0] * 255;
			re->shaderRGBA[1] = ci->color2[1] * 255;
			re->shaderRGBA[2] = ci->color2[2] * 255;
			re->shaderRGBA[3] = 255;

			le->color[0] = ci->color2[0] * 0.75;
			le->color[1] = ci->color2[1] * 0.75;
			le->color[2] = ci->color2[2] * 0.75;
			le->color[3] = 1.0f;
			le->pos.trType = TR_LINEAR;
			le->pos.trTime = cg.time;

			VectorCopy(move, move2);
			VectorMA(move2, RADIUS, axis[j], move2);
			VectorCopy(move2, le->pos.trBase);

			le->pos.trDelta[0] = axis[j][0] * 6;
			le->pos.trDelta[1] = axis[j][1] * 6;
			le->pos.trDelta[2] = axis[j][2] * 6;
		}

		VectorAdd(move, vec, move);

		j = (j + ROTATION) % 36;
	}
}

/*
=======================================================================================================================================
CG_PlasmaTrail
=======================================================================================================================================
*/
static void CG_PlasmaTrail(centity_t *cent, const weaponInfo_t *wi) {
	localEntity_t *le;
	refEntity_t *re;
	entityState_t *es;
	vec3_t velocity, xvelocity, origin;
	vec3_t offset, xoffset;
	vec3_t v[3];

	float waterScale = 1.0f;

	if (cg_noProjectileTrail.integer || cg_oldPlasma.integer) {
		return;
	}

	es = &cent->currentState;

	BG_EvaluateTrajectory(&es->pos, cg.time, origin, qfalse, es->effect2Time);

	le = CG_AllocLocalEntity();

	re = &le->refEntity;

	velocity[0] = 60 - 120 * crandom();
	velocity[1] = 40 - 80 * crandom();
	velocity[2] = 100 - 200 * crandom();

	le->leType = LE_MOVE_SCALE_FADE;
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;
	le->startTime = cg.time;
	le->endTime = le->startTime + 600;
	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time;

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 2;
	offset[1] = 2;
	offset[2] = 2;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd(origin, xoffset, re->origin);
	VectorCopy(re->origin, le->pos.trBase);

	if (CG_PointContents(re->origin, -1) & CONTENTS_WATER) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];

	VectorScale(xvelocity, waterScale, le->pos.trDelta);
	AxisCopy(axisDefault, re->axis);

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_SPRITE;
	re->radius = 0.25f;
	re->customShader = cgs.media.railRingsShader;

	le->bounceFactor = 0.3f;

	re->shaderRGBA[0] = wi->flashDlightColor[0] * 63;
	re->shaderRGBA[1] = wi->flashDlightColor[1] * 63;
	re->shaderRGBA[2] = wi->flashDlightColor[2] * 63;
	re->shaderRGBA[3] = 63;

	le->color[0] = wi->flashDlightColor[0] * 0.2;
	le->color[1] = wi->flashDlightColor[1] * 0.2;
	le->color[2] = wi->flashDlightColor[2] * 0.2;
	le->color[3] = 0.25f;
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 1;
	le->angles.trDelta[1] = 0.5;
	le->angles.trDelta[2] = 0;
}

/*
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	BRASS

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

/*
=======================================================================================================================================
CG_MachineGunEjectBrass
=======================================================================================================================================
*/
static void CG_MachineGunEjectBrass(centity_t *cent) {
	localEntity_t *le;
	refEntity_t *re;
	vec3_t velocity, xvelocity;
	vec3_t offset, xoffset;
	float waterScale = 1.0f;
	vec3_t v[3];

	if (cg_brassTime.integer <= 0) {
		return;
	}

	le = CG_AllocLocalEntity();

	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + (cg_brassTime.integer / 4) * random();
	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd(cent->lerpOrigin, xoffset, re->origin);
	VectorCopy(re->origin, le->pos.trBase);

	if (CG_PointContents(re->origin, -1) & CONTENTS_WATER) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];

	VectorScale(xvelocity, waterScale, le->pos.trDelta);
	AxisCopy(axisDefault, re->axis);

	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;
	le->leFlags = LEF_BRASS_MG;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

/*
=======================================================================================================================================
CG_ShotgunEjectBrass
=======================================================================================================================================
*/
static void CG_ShotgunEjectBrass(centity_t *cent) {
	localEntity_t *le;
	refEntity_t *re;
	vec3_t velocity, xvelocity;
	vec3_t offset, xoffset;
	vec3_t v[3];
	int i;

	if (cg_brassTime.integer <= 0) {
		return;
	}

	for (i = 0; i < 2; i++) {
		float waterScale = 1.0f;

		le = CG_AllocLocalEntity();

		re = &le->refEntity;

		velocity[0] = 60 + 60 * crandom();

		if (i == 0) {
			velocity[1] = 40 + 10 * crandom();
		} else {
			velocity[1] = -40 + 10 * crandom();
		}

		velocity[2] = 100 + 50 * crandom();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer * 3 + cg_brassTime.integer * random();
		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis(cent->lerpAngles, v);

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

		VectorAdd(cent->lerpOrigin, xoffset, re->origin);
		VectorCopy(re->origin, le->pos.trBase);

		if (CG_PointContents(re->origin, -1) & CONTENTS_WATER) {
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];

		VectorScale(xvelocity, waterScale, le->pos.trDelta);
		AxisCopy(axisDefault, re->axis);

		re->hModel = cgs.media.shotgunBrassModel;

		le->bounceFactor = 0.3f;
		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand()&31;
		le->angles.trBase[1] = rand()&31;
		le->angles.trBase[2] = rand()&31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;
		le->leFlags = LEF_BRASS_SG;
		le->leBounceSoundType = LEBS_BRASS;
		le->leMarkType = LEMT_NONE;
	}
}

/*
=======================================================================================================================================
CG_NailgunEjectBrass
=======================================================================================================================================
*/
static void CG_NailgunEjectBrass(centity_t *cent) {
	localEntity_t *smoke;
	vec3_t origin;
	vec3_t v[3];
	vec3_t offset;
	vec3_t xoffset;
	vec3_t up;

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd(cent->lerpOrigin, xoffset, origin);
	VectorSet(up, 0, 0, 64);

	smoke = CG_SmokePuff(origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader);
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}

/*
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	WEAPON EVENTS

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

/*
=======================================================================================================================================

	BULLETS

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_Tracer
=======================================================================================================================================
*/
void CG_Tracer(vec3_t source, vec3_t dest) {
	vec3_t forward, right;
	polyVert_t verts[4];
	vec3_t line;
	float len, begin, end;
	vec3_t start, finish;
	vec3_t midpoint;

	// tracer
	VectorSubtract(dest, source, forward);

	len = VectorNormalize(forward);
	// start at least a little ways from the muzzle
	if (len < 100) {
		return;
	}

	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;

	if (end > len) {
		end = len;
	}

	VectorMA(source, begin, forward, start);
	VectorMA(source, end, forward, finish);

	line[0] = DotProduct(forward, cg.refdef.viewaxis[1]);
	line[1] = DotProduct(forward, cg.refdef.viewaxis[2]);

	VectorScale(cg.refdef.viewaxis[1], line[1], right);
	VectorMA(right, -line[0], cg.refdef.viewaxis[2], right);
	VectorNormalize(right);

	VectorMA(finish, cg_tracerWidth.value, right, verts[0].xyz);
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA(finish, -cg_tracerWidth.value, right, verts[1].xyz);
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA(start, -cg_tracerWidth.value, right, verts[2].xyz);
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA(start, cg_tracerWidth.value, right, verts[3].xyz);
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene(cgs.media.tracerShader, 4, verts);

	midpoint[0] = (start[0] + finish[0]) * 0.5;
	midpoint[1] = (start[1] + finish[1]) * 0.5;
	midpoint[2] = (start[2] + finish[2]) * 0.5;
	// add the tracer sound
	trap_S_StartSound(midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound, 64, 255);
}

/*
=======================================================================================================================================
CG_CalcMuzzlePoint
=======================================================================================================================================
*/
static qboolean CG_CalcMuzzlePoint(int entityNum, vec3_t muzzle) {
	vec3_t forward;
	centity_t *cent;
	int anim;

	if (entityNum == cg.snap->ps.clientNum) {
		VectorCopy(cg.snap->ps.origin, muzzle);

		muzzle[2] += cg.snap->ps.viewheight;

		AngleVectorsForward(cg.snap->ps.viewangles, forward);
		VectorMA(muzzle, 14, forward, muzzle);
		return qtrue;
	}

	cent = &cg_entities[entityNum];

	if (!cent->currentValid) {
		return qfalse;
	}

	VectorCopy(cent->currentState.pos.trBase, muzzle);
	AngleVectorsForward(cent->currentState.apos.trBase, forward);

	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

	if (anim == LEGS_WALKCR || anim == LEGS_IDLECR) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA(muzzle, 14, forward, muzzle);
	return qtrue;
}

/*
=======================================================================================================================================
CG_Bullet

Renders bullet effects.
=======================================================================================================================================
*/
void CG_Bullet(vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum) {
	trace_t trace;
	int sourceContentType, destContentType;
	vec3_t start;

	// if the shooter is currently valid, calc a source point and possibly do trail effects
	if (sourceEntityNum >= 0 && cg_tracerChance.value > 0) {
		if (CG_CalcMuzzlePoint(sourceEntityNum, start)) {
			sourceContentType = CG_PointContents(start, 0);
			destContentType = CG_PointContents(end, 0);
			// do a complete bubble trail if necessary
			if ((sourceContentType == destContentType) && (sourceContentType & CONTENTS_WATER)) {
				CG_BubbleTrail(start, end, 32);
			// bubble trail from water into air
			} else if ((sourceContentType & CONTENTS_WATER)) {
				trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
				CG_BubbleTrail(start, trace.endpos, 32);
			// bubble trail from air into water
			} else if ((destContentType & CONTENTS_WATER)) {
				trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
				CG_BubbleTrail(trace.endpos, end, 32);
			}
			// draw a tracer
			if (random() < cg_tracerChance.value) {
				CG_Tracer(start, end);
			}
		}
	}
	// impact splash and mark
	if (flesh) {
		CG_Bleed(end, fleshEntityNum);
	} else {
		CG_MissileHitWall(WP_MACHINEGUN, 0, end, normal);
	}
}

/*
=======================================================================================================================================

	SHOTGUN TRACING

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_ShotgunPellet
=======================================================================================================================================
*/
static void CG_ShotgunPellet(vec3_t start, vec3_t end, int skipNum) {
	trace_t tr;
	int sourceContentType, destContentType;

	CG_Trace(&tr, start, NULL, NULL, end, skipNum, MASK_SHOT);

	sourceContentType = CG_PointContents(start, 0);
	destContentType = CG_PointContents(tr.endpos, 0);
	// FIXME: should probably move this cruft into CG_BubbleTrail
	if (sourceContentType == destContentType) {
		if (sourceContentType & CONTENTS_WATER) {
			CG_BubbleTrail(start, tr.endpos, 32);
		}
	} else if (sourceContentType & CONTENTS_WATER) {
		trace_t trace;

		trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
		CG_BubbleTrail(start, trace.endpos, 32);
	} else if (destContentType & CONTENTS_WATER) {
		trace_t trace;

		trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
		CG_BubbleTrail(tr.endpos, trace.endpos, 32);
	}

	if (tr.surfaceFlags & SURF_NOIMPACT) {
		return;
	}

	if (cg_entities[tr.entityNum].currentState.eType == ET_PLAYER) {
		CG_MissileHitPlayer(WP_SHOTGUN, tr.endpos, tr.plane.normal, tr.entityNum);
	} else {
		if (tr.surfaceFlags & SURF_NOIMPACT) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}

		CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal);
	}
}

/*
=======================================================================================================================================
CG_ShotgunPattern

Perform the same traces the server did to locate the hit splashes.
=======================================================================================================================================
*/
static void CG_ShotgunPattern(vec3_t origin, vec3_t origin2, int seed, int otherEntNum) {
	int i;
	float r, u;
	vec3_t end;
	vec3_t forward, right, up;

	// derive the right and up vectors from the forward vector, because the client won't have any other information
	VectorNormalize2(origin2, forward);
	PerpendicularVector(right, forward);
	CrossProduct(forward, right, up);
	// generate the "random" spread pattern
	for (i = 0; i < DEFAULT_SHOTGUN_COUNT; i++) {
		r = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;

		VectorMA(origin, 131072, forward, end);
		VectorMA(end, r, right, end);
		VectorMA(end, u, up, end);
		CG_ShotgunPellet(origin, end, otherEntNum);
	}
}

/*
=======================================================================================================================================
CG_ShotgunFire
=======================================================================================================================================
*/
void CG_ShotgunFire(entityState_t *es) {
	vec3_t v;
	int contents;

	VectorSubtract(es->origin2, es->pos.trBase, v);
	VectorNormalize(v);
	VectorScale(v, 32, v);
	VectorAdd(es->pos.trBase, v, v);

	if (cgs.glconfig.hardwareType != GLHW_RAGEPRO) {
		// ragepro can't alpha fade, so don't even bother with smoke
		vec3_t up;

		contents = CG_PointContents(es->pos.trBase, 0);

		if (!(contents & CONTENTS_WATER)) {
			VectorSet(up, 0, 0, 8);
			CG_SmokePuff(v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
		}
	}

	CG_ShotgunPattern(es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum);
}

/*
=======================================================================================================================================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event.
=======================================================================================================================================
*/
void CG_FireWeapon(centity_t *cent) {
	entityState_t *ent;
	int c;
	weaponInfo_t *weap;

	ent = &cent->currentState;

	if (ent->weapon == WP_NONE) {
		return;
	}

	if (ent->weapon >= WP_NUM_WEAPONS) {
		CG_Error("CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS");
		return;
	}

	weap = &cg_weapons[ent->weapon];
	// mark the entity as muzzle flashing, so when it is added it will append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;
	// beam gun only does this this on initial press
	if (ent->weapon == WP_BEAMGUN) {
		if (cent->pe.beamgunFiring) {
			return;
		}
	}

	if (ent->weapon == WP_RAILGUN) {
		cent->pe.railFireTime = cg.time;
	}
	// play quad sound if needed
	if (cent->currentState.powerups & (1 << PW_QUAD)) {
		trap_S_StartSound(NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound, 128, 255);
	}
	// play a sound
	for (c = 0; c < 4; c++) {
		if (!weap->flashSound[c]) {
			break;
		}
	}

	if (c > 0) {
		c = rand() % c;

		if (weap->flashSound[c]) {
			trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[c], 128, 255);
		}
	}
	// do brass ejection
	if (weap->ejectBrassFunc && cg_brassTime.integer > 0) {
		weap->ejectBrassFunc(cent);
	}
}

/*
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	REGISTER WEAPON

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

/*
=======================================================================================================================================
CG_RegisterWeapon

The server says this item is used on this level.
=======================================================================================================================================
*/
void CG_RegisterWeapon(int weaponNum) {
	weaponInfo_t *weaponInfo;
	gitem_t *item, *ammo;
	char path[MAX_QPATH];
	vec3_t mins, maxs;
	int i;

	weaponInfo = &cg_weapons[weaponNum];

	if (weaponNum == 0) {
		return;
	}

	if (weaponInfo->registered) {
		return;
	}

	memset(weaponInfo, 0, sizeof(*weaponInfo));

	weaponInfo->registered = qtrue;

	for (item = bg_itemlist + 1; item->classname; item++) {
		if (item->giType == IT_WEAPON && item->giTag == weaponNum) {
			weaponInfo->item = item;
			break;
		}
	}

	if (!item->classname) {
		CG_Error("Couldn't find weapon %i", weaponNum);
	}

	CG_RegisterItemVisuals(item - bg_itemlist);
	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel(item->world_model[0]);
	// calc midpoint for rotation
	trap_R_ModelBounds(weaponInfo->weaponModel, mins, maxs);

	for (i = 0; i < 3; i++) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * (maxs[i] - mins[i]);
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader(item->icon);
	weaponInfo->ammoIcon = trap_R_RegisterShader(item->icon);

	for (ammo = bg_itemlist + 1; ammo->classname; ammo++) {
		if (ammo->giType == IT_AMMO && ammo->giTag == weaponNum) {
			break;
		}
	}

	if (ammo->classname && ammo->world_model[0]) {
		weaponInfo->ammoModel = trap_R_RegisterModel(ammo->world_model[0]);
	}

	COM_StripExtension(item->world_model[0], path, sizeof(path));
	Q_strcat(path, sizeof(path), "_flash.md3");
	weaponInfo->flashModel = trap_R_RegisterModel(path);

	COM_StripExtension(item->world_model[0], path, sizeof(path));
	Q_strcat(path, sizeof(path), "_barrel.md3");
	weaponInfo->barrelModel = trap_R_RegisterModel(path);

	COM_StripExtension(item->world_model[0], path, sizeof(path));
	Q_strcat(path, sizeof(path), "_hand.md3");
	weaponInfo->handsModel = trap_R_RegisterModel(path);

	if (!weaponInfo->handsModel) {
		weaponInfo->handsModel = trap_R_RegisterModel("models/weapons2/shotgun/shotgun_hand.md3");
	}

	switch (weaponNum) {
		case WP_GAUNTLET:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 1.0f, 1.0f);
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/melee/fstrun.wav", qfalse);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/melee/fstatck.wav", qfalse);
			break;
		case WP_HANDGUN:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.75f, 0);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/handgun/hf1.wav", qfalse);
			weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/handgun/hf2.wav", qfalse);
			weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/handgun/hf3.wav", qfalse);
			weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/handgun/hf4.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
			cgs.media.bulletExplosionShader = trap_R_RegisterShader("bulletExplosion");
			break;
		case WP_MACHINEGUN:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.75f, 0);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/machinegun/machgf1b.wav", qfalse);
			weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/machinegun/machgf2b.wav", qfalse);
			weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/machinegun/machgf3b.wav", qfalse);
			weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/machinegun/machgf4b.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
			cgs.media.bulletExplosionShader = trap_R_RegisterShader("bulletExplosion");
			break;
		case WP_HEAVY_MACHINEGUN:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.75f, 0);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/hmg/hmgf1.wav", qfalse);
			weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/hmg/hmgf2.wav", qfalse);
			weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/hmg/hmgf3.wav", qfalse);
			weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/hmg/hmgf4.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
			cgs.media.bulletExplosionShader = trap_R_RegisterShader("bulletExplosion");
			break;
		case WP_CHAINGUN:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.8f, 0.2f);
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/vulcan/wvulfire.wav", qfalse);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf1b.wav", qfalse);
			weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf2b.wav", qfalse);
			weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf3b.wav", qfalse);
			weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf4b.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
			cgs.media.bulletExplosionShader = trap_R_RegisterShader("bulletExplosion");
			break;
		case WP_SHOTGUN:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.7f, 0);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/shotgun/sshotf1b.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
			break;
		case WP_NAILGUN:
			MAKERGB(weaponInfo->flashDlightColor, 1, 0.7f, 0);
			weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
			weaponInfo->missileTrailFunc = CG_NailTrail;
//			weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/nailgun/wnalflit.wav", qfalse);
			weaponInfo->trailRadius = 16;
			weaponInfo->wiTrailTime = 250;
			weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/nail.md3");
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/nailgun/wnalfire.wav", qfalse);
			break;
		case WP_PHOSPHORGUN:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.8f, 0.2f);
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/phosphorgun/pfire.wav", qfalse);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/phosphorgun/pf1.wav", qfalse);
			weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/phosphorgun/pf1.wav", qfalse);
			weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/phosphorgun/pf1.wav", qfalse);
			weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/phosphorgun/pf1.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
			cgs.media.phosphorExplosionShader = trap_R_RegisterShader("phosphorExplosion");
			break;
		case WP_PROXLAUNCHER:
			weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/proxmine.md3");
			weaponInfo->missileTrailFunc = CG_GrenadeTrail;
			weaponInfo->wiTrailTime = 700;
			weaponInfo->trailRadius = 32;
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/proxmine/wstbfire.wav", qfalse);
			cgs.media.grenadeExplosionShader = trap_R_RegisterShader("grenadeExplosion");
			break;
		case WP_GRENADELAUNCHER:
			weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/grenade1.md3");
			weaponInfo->missileTrailFunc = CG_GrenadeTrail;
			weaponInfo->wiTrailTime = 700;
			weaponInfo->trailRadius = 32;
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/grenade/grenlf1a.wav", qfalse);
			cgs.media.grenadeExplosionShader = trap_R_RegisterShader("grenadeExplosion");
			break;
		case WP_NAPALMLAUNCHER:
			weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/grenade1.md3");
			weaponInfo->missileTrailFunc = CG_GrenadeTrail;
			weaponInfo->wiTrailTime = 700;
			weaponInfo->trailRadius = 32;
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/napalm/napalmf1.wav", qfalse);
			cgs.media.grenadeExplosionShader = trap_R_RegisterShader("grenadeExplosion");
			break;
		case WP_ROCKETLAUNCHER:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.75f, 0);
			weaponInfo->missileDlight = 100;
			MAKERGB(weaponInfo->missileDlightColor, 1.0f, 0.75f, 0);
			weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/rocket/rocket.md3");
			weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/rocket/rockfly.wav", qfalse);
			weaponInfo->missileTrailFunc = CG_RocketTrail;
			weaponInfo->wiTrailTime = 2000;
			weaponInfo->trailRadius = 64;
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.wav", qfalse);
			cgs.media.rocketExplosionShader = trap_R_RegisterShader("rocketExplosion");
			break;
		case WP_BEAMGUN:
			MAKERGB(weaponInfo->flashDlightColor, 0.45f, 0.7f, 1.0f);
			weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/melee/fsthum.wav", qfalse);
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/lightning/lg_hum.wav", qfalse);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/lightning/lg_fire.wav", qfalse);
			cgs.media.lightningShader = trap_R_RegisterShader("lightningBoltNew");
			cgs.media.lightningExplosionModel = trap_R_RegisterModel("models/weaphits/crackle.md3");
			cgs.media.sfx_lghit1 = trap_S_RegisterSound("sound/weapons/lightning/lg_hit.wav", qfalse);
			cgs.media.sfx_lghit2 = trap_S_RegisterSound("sound/weapons/lightning/lg_hit2.wav", qfalse);
			cgs.media.sfx_lghit3 = trap_S_RegisterSound("sound/weapons/lightning/lg_hit3.wav", qfalse);
			break;
		case WP_RAILGUN:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0, 0.7f);
			weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/railgun/rg_hum.wav", qfalse);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/railgun/railgf1a.wav", qfalse);
			cgs.media.railExplosionShader = trap_R_RegisterShader("railExplosion");
			cgs.media.railRingsShader = trap_R_RegisterShader("railDisc");
			cgs.media.railCoreShader = trap_R_RegisterShader("railCore");
			break;
		case WP_PLASMAGUN:
			MAKERGB(weaponInfo->flashDlightColor, 0.7f, 0.8f, 1.0f);
			weaponInfo->missileDlight = 100;
			MAKERGB(weaponInfo->missileDlightColor, 0.7f, 0.8f, 1.0f);
			weaponInfo->missileTrailFunc = CG_PlasmaTrail;
			weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/plasma/hyprbf1a.wav", qfalse);
			cgs.media.plasmaExplosionShader = trap_R_RegisterShader("plasmaExplosion");
			cgs.media.railRingsShader = trap_R_RegisterShader("railDisc");
			break;
		case WP_BFG:
			MAKERGB(weaponInfo->flashDlightColor, 0.65f, 1.0f, 0.7f);
			weaponInfo->missileDlight = 100;
			MAKERGB(weaponInfo->missileDlightColor, 0.65f, 1.0f, 0.7f);
			weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/bfg/bfg_hum.wav", qfalse);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/bfg/bfg_fire.wav", qfalse);
			cgs.media.bfgExplosionShader = trap_R_RegisterShader("bfgExplosion");
			weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/bfg.md3");
			weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/rocket/rockfly.wav", qfalse);
			break;
		case WP_MISSILELAUNCHER:
			MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.75f, 0);
			weaponInfo->missileDlight = 100;
			MAKERGB(weaponInfo->missileDlightColor, 1.0f, 0.75f, 0);
			weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/rocket/rocket.md3");
			weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/rocket/rockfly.wav", qfalse);
			weaponInfo->missileTrailFunc = CG_RocketTrail;
			weaponInfo->wiTrailTime = 2000;
			weaponInfo->trailRadius = 64;
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.wav", qfalse);
			cgs.media.rocketExplosionShader = trap_R_RegisterShader("rocketExplosion");
			break;
		default:
			MAKERGB(weaponInfo->flashDlightColor, 0, 0, 0);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.wav", qfalse);
			break;
	}
}

/*
=======================================================================================================================================
CG_RegisterItemVisuals

The server says this item is used on this level.
=======================================================================================================================================
*/
void CG_RegisterItemVisuals(int itemNum) {
	itemInfo_t *itemInfo;
	gitem_t *item;

	if (itemNum < 0 || itemNum >= bg_numItems) {
		CG_Error("CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems - 1);
	}

	itemInfo = &cg_items[itemNum];

	if (itemInfo->registered) {
		return;
	}

	item = &bg_itemlist[itemNum];

	memset(itemInfo, 0, sizeof(*itemInfo));

	itemInfo->registered = qtrue;
	itemInfo->models[0] = trap_R_RegisterModel(item->world_model[0]);
	itemInfo->icon = trap_R_RegisterShader(item->icon);

	if (item->giType == IT_WEAPON) {
		CG_RegisterWeapon(item->giTag);
	}
	// powerups have an accompanying ring or sphere
	if (item->giType == IT_POWERUP || item->giType == IT_HEALTH) {
		if (item->world_model[1]) {
			itemInfo->models[1] = trap_R_RegisterModel(item->world_model[1]);
		}
	}
}

/*
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	VIEW WEAPON

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

/*
=======================================================================================================================================
CG_MapTorsoToWeaponFrame
=======================================================================================================================================
*/
static int CG_MapTorsoToWeaponFrame(clientInfo_t *ci, int frame) {

	// change weapon
	if (frame >= ci->animations[TORSO_DROP].firstFrame && frame < ci->animations[TORSO_DROP].firstFrame + 9) {
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}
	// stand attack
	if (frame >= ci->animations[TORSO_ATTACK].firstFrame && frame < ci->animations[TORSO_ATTACK].firstFrame + 6) {
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}
	// stand attack 2
	if (frame >= ci->animations[TORSO_ATTACK2].firstFrame && frame < ci->animations[TORSO_ATTACK2].firstFrame + 6) {
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}

	return 0;
}

/*
=======================================================================================================================================
CG_CalculateWeaponPosition
=======================================================================================================================================
*/
static void CG_CalculateWeaponPosition(vec3_t origin, vec3_t angles) {
	float scale;
	int delta;
	float fracsin;

	VectorCopy(cg.refdef.vieworg, origin);
	VectorCopy(cg.refdefViewAngles, angles);
	// on odd legs, invert some angles
	if (cg.bobcycle & 1) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}
	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;
	// drop the weapon when landing
	delta = cg.time - cg.landTime;

	if (delta < LAND_DEFLECT_TIME) {
		origin[2] += cg.landChange * 0.25 * delta / LAND_DEFLECT_TIME;
	} else if (delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME) {
		origin[2] += cg.landChange * 0.25 * (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}
#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;

	if (delta < STEP_TIME / 2) {
		origin[2] -= cg.stepChange * 0.25 * delta / (STEP_TIME / 2);
	} else if (delta < STEP_TIME) {
		origin[2] -= cg.stepChange * 0.25 * (STEP_TIME - delta) / (STEP_TIME / 2);
	}
#endif
	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin(cg.time * 0.001);
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}

#define SPIN_SPEED 0.9
#define COAST_TIME 1000
/*
=======================================================================================================================================
CG_MachinegunSpinAngle
=======================================================================================================================================
*/
static float CG_MachinegunSpinAngle(centity_t *cent) {
	int delta;
	float angle;
	float speed;

	delta = cg.time - cent->pe.barrelTime;

	if (cent->pe.barrelSpinning) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	} else {
		if (delta > COAST_TIME) {
			delta = COAST_TIME;
		}

		speed = 0.5 * (SPIN_SPEED + (float)(COAST_TIME - delta) / COAST_TIME);
		angle = cent->pe.barrelAngle + delta * speed;
	}

	if (cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING)) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod(angle);
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);

		if (cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning) {
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_WEAPON, cgs.media.sfx_chgstop, 64, 255);
		}

		if (cent->currentState.weapon == WP_HEAVY_MACHINEGUN && !cent->pe.barrelSpinning) {
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_WEAPON, cgs.media.sfx_hmgstop, 64, 255);
		}
	}

	return angle;
}

/*
=======================================================================================================================================
CG_AddWeaponWithPowerups
=======================================================================================================================================
*/
static void CG_AddWeaponWithPowerups(refEntity_t *gun, int powerups, int team) {

	// add powerup effects
	if (powerups & (1 << PW_INVIS)) {
		if (team == TEAM_RED) {
			gun->customShader = cgs.media.invisRedShader;
		} else if (team == TEAM_BLUE) {
			gun->customShader = cgs.media.invisBlueShader;
		} else {
			gun->customShader = cgs.media.invisShader;
		}

		trap_R_AddRefEntityToScene(gun);
	} else {
		trap_R_AddRefEntityToScene(gun);

		if (powerups & (1 << PW_QUAD)) {
			gun->customShader = cgs.media.quadWeaponShader;
			trap_R_AddRefEntityToScene(gun);
		}
	}
}

/*
=======================================================================================================================================
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL).
The main player will have this called for BOTH cases, so effects like light and sound should only be done on the world model case.
=======================================================================================================================================
*/
void CG_AddPlayerWeapon(refEntity_t *parent, playerState_t *ps, centity_t *cent, int team) {
	refEntity_t gun;
	refEntity_t barrel;
	refEntity_t flash;
	vec3_t angles;
	weapon_t weaponNum;
	weaponInfo_t *weapon;
	centity_t *nonPredictedCent;
	orientation_t lerped;
	clientInfo_t *ci;

	weaponNum = cent->currentState.weapon;

	CG_RegisterWeapon(weaponNum);

	weapon = &cg_weapons[weaponNum];
	ci = &cgs.clientinfo[cent->currentState.clientNum];
	// add the weapon
	memset(&gun, 0, sizeof(gun));

	VectorCopy(parent->lightingOrigin, gun.lightingOrigin);

	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;
	// set custom shading for railgun refire rate
	if (weaponNum == WP_RAILGUN && cent->pe.railFireTime + 1500 > cg.time) {
		int scale = 255 * (cg.time - cent->pe.railFireTime) / 1500;

		gun.shaderRGBA[0] = (ci->c1RGBA[0] * scale) >> 8;
		gun.shaderRGBA[1] = (ci->c1RGBA[1] * scale) >> 8;
		gun.shaderRGBA[2] = (ci->c1RGBA[2] * scale) >> 8;
		gun.shaderRGBA[3] = 255;
	} else {
		Byte4Copy(ci->c1RGBA, gun.shaderRGBA);
	}
	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	if (!weapon->weaponModel || !weapon->flashModel) {
		// use default flash origin when no flash model
		VectorCopy(cent->lerpOrigin, nonPredictedCent->pe.flashOrigin);
		nonPredictedCent->pe.flashOrigin[2] += DEFAULT_VIEWHEIGHT - 6;
	}

	gun.hModel = weapon->weaponModel;

	if (!gun.hModel) {
		return;
	}

	if (!ps) {
		// add weapon ready sound
		cent->pe.beamgunFiring = qfalse;

		if ((cent->currentState.eFlags & EF_FIRING) && weapon->firingSound) {
			// beam gun and gauntlet make a different sound when fire is held down
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound, 128, 255);
			cent->pe.beamgunFiring = qtrue;
		} else if (weapon->readySound) {
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound, 32, 255);
		}
	}

	trap_R_LerpTag(&lerped, parent->hModel, parent->oldframe, parent->frame, 1.0 - parent->backlerp, "tag_weapon");
	VectorCopy(parent->origin, gun.origin);
	VectorMA(gun.origin, lerped.origin[0], parent->axis[0], gun.origin);
	// Make weapon appear left-handed for 2 and centered for 3
	if (ps && cg_drawGun.integer == 2) {
		VectorMA(gun.origin, -lerped.origin[1], parent->axis[1], gun.origin);
	} else if (!ps || cg_drawGun.integer != 3) {
		VectorMA(gun.origin, lerped.origin[1], parent->axis[1], gun.origin);
	}

	VectorMA(gun.origin, lerped.origin[2], parent->axis[2], gun.origin);
	MatrixMultiply(lerped.axis, ((refEntity_t *)parent)->axis, gun.axis);

	gun.backlerp = parent->backlerp;

	CG_AddWeaponWithPowerups(&gun, cent->currentState.powerups, ci->team);
	// add the spinning barrel
	if (weapon->barrelModel) {
		memset(&barrel, 0, sizeof(barrel));

		VectorCopy(parent->lightingOrigin, barrel.lightingOrigin);

		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;
		barrel.hModel = weapon->barrelModel;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = CG_MachinegunSpinAngle(cent);

		AnglesToAxis(angles, barrel.axis);
		CG_PositionRotatedEntityOnTag(&barrel, &gun, weapon->weaponModel, "tag_barrel");
		CG_AddWeaponWithPowerups(&barrel, cent->currentState.powerups, ci->team);
	}
	// if the index of the nonPredictedCent is not the same as the clientNum then this is a fake player (like on the single player podiums), so go ahead and use the cent
	if ((nonPredictedCent - cg_entities) != cent->currentState.clientNum) {
		nonPredictedCent = cent;
	}

	memset(&flash, 0, sizeof(flash));

	VectorCopy(parent->lightingOrigin, flash.lightingOrigin);

	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;
	flash.hModel = weapon->flashModel;

	if (!flash.hModel) {
		return;
	}

	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;

	AnglesToAxis(angles, flash.axis);
	// colorize the railgun blast
	Byte4Copy(ci->c1RGBA, flash.shaderRGBA);
	CG_PositionRotatedEntityOnTag(&flash, &gun, weapon->weaponModel, "tag_flash");
	// update muzzle origin
	if (ps) {
		VectorCopy(flash.origin, cg.flashOrigin);
	} else {
		VectorCopy(flash.origin, nonPredictedCent->pe.flashOrigin);
	}
	// add the flash
	if ((weaponNum == WP_GAUNTLET || weaponNum == WP_BEAMGUN) && (nonPredictedCent->currentState.eFlags & EF_FIRING)) {
		// continuous flash
	} else {
		// impulse flash
		if (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME) {
			return;
		}
	}

	trap_R_AddRefEntityToScene(&flash);

	if (ps || cg.renderingThirdPerson || cent->currentState.number != cg.predictedPlayerState.clientNum) {
		// add beam gun bolt
		CG_BeamgunBolt(nonPredictedCent, flash.origin);

		if (weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2]) {
			trap_R_AddLightToScene(flash.origin, 100 + (rand()&31), weapon->flashDlightColor[0], weapon->flashDlightColor[1], weapon->flashDlightColor[2]);
		}
	}
}

/*
=======================================================================================================================================
CG_AddViewWeapon

Add the weapon, and flash for the player's view.
=======================================================================================================================================
*/
void CG_AddViewWeapon(playerState_t *ps) {
	refEntity_t hand;
	centity_t *cent;
	clientInfo_t *ci;
	vec3_t fovOffset, angles;
	weaponInfo_t *weapon;

	if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if (ps->pm_type == PM_INTERMISSION) {
		return;
	}
	// no gun if in third person view or a camera is active
	//if (cg.renderingThirdPerson || cg.cameraMode) {
	if (cg.renderingThirdPerson) {
		return;
	}
	// allow the gun to be completely removed
	if (!cg_drawGun.integer) {
		// use default flash origin
		VectorCopy(cg_entities[ps->clientNum].lerpOrigin, cg.flashOrigin);
		cg.flashOrigin[2] += DEFAULT_VIEWHEIGHT - 6;

		if (cg.predictedPlayerState.eFlags & EF_FIRING) {
			// special hack for beam gun...
			CG_BeamgunBolt(&cg_entities[ps->clientNum], cg.flashOrigin);
		}

		return;
	}
	// don't draw if testing a gun model
	if (cg.testGun) {
		return;
	}

	VectorClear(fovOffset);

	if (cg_fovGunAdjust.integer) {
		if (cg.fov > 90) {
			// drop gun lower at higher fov
			fovOffset[2] = -0.2 * (cg.fov - 90) * cg.refdef.fov_x / cg.fov;
		} else if (cg.fov < 90) {
			// move gun forward at lowerer fov
			fovOffset[0] = -0.2 * (cg.fov - 90) * cg.refdef.fov_x / cg.fov;
		}
	} else if (cg_fov.integer > 90) {
		// Q3A's auto adjust
		fovOffset[2] = -0.2 * (cg_fov.integer - 90);
	}

	cent = &cg.predictedPlayerEntity; //&cg_entities[cg.snap->ps.clientNum];

	CG_RegisterWeapon(ps->weapon);

	weapon = &cg_weapons[ps->weapon];

	memset(&hand, 0, sizeof(hand));
	// set up gun position
	CG_CalculateWeaponPosition(hand.origin, angles);
	VectorMA(hand.origin, (cg_gun_x.value + fovOffset[0]), cg.refdef.viewaxis[0], hand.origin);
	VectorMA(hand.origin, (cg_gun_y.value + fovOffset[1]), cg.refdef.viewaxis[1], hand.origin);
	VectorMA(hand.origin, (cg_gun_z.value + fovOffset[2]), cg.refdef.viewaxis[2], hand.origin);
	AnglesToAxis(angles, hand.axis);
	// map torso animations to weapon animations
	if (cg_gun_frame.integer) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
		// get clientinfo for animation map
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		hand.frame = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.frame);
		hand.oldframe = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.oldFrame);
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = RF_DEPTHHACK|RF_FIRST_PERSON|RF_MINLIGHT;
	// add everything onto the hand
	CG_AddPlayerWeapon(&hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM]);
}

/*
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	WEAPON SELECTION

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

#define HUD_ICONSIZE 24
#define HUD_ICONSIZESEL 8
#define HUD_ICONSPACE 4
#define HUD_X (320 - (HUD_ICONSIZE / 2))
#define HUD_FADE_DIST 160

/*
=======================================================================================================================================
CG_DrawWeaponSelect
=======================================================================================================================================
*/
void CG_DrawWeaponSelect(void) {
	int bits[MAX_WEAPONS / (sizeof(int) * 8)], count, i, x, y, diff, weap;
	float *color, dist;
	vec4_t fadecolor = {1.0f, 1.0f, 1.0f, 1.0f};
	char *name;

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_BOTTOM);
	// don't display if dead
	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0) {
		return;
	}

	color = CG_FadeColor(cg.weaponSelectTime, WEAPON_SELECT_TIME * 2);

	if (!color) {
		cg.bar_offset = 0;
		return;
	}

	cg.bar_offset = color[3] * color[3];

	trap_R_SetColor(color);
#ifndef BASEGAME
	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;
#endif
	// count the number of weapons owned
	memcpy(bits, cg.snap->ps.weapons, sizeof(bits));

	count = 0;

	for (i = 1; i < MAX_WEAPONS; i++) {
		if (COM_BitCheck(bits, i)) {
			count++;
		}
	}

	cg.bar_count = count;
	y = 380;

	if (count <= 0) {
		return;
	}
	// draw current selection:
	CG_DrawPic(HUD_X - HUD_ICONSIZESEL / 2, y - HUD_ICONSIZESEL / 2, HUD_ICONSIZE + HUD_ICONSIZESEL, HUD_ICONSIZE + HUD_ICONSIZESEL, cg_weapons[cg.weaponSelect].weaponIcon);
	CG_DrawPic(HUD_X - HUD_ICONSIZESEL / 2, y - HUD_ICONSIZESEL / 2, HUD_ICONSIZE + HUD_ICONSIZESEL, HUD_ICONSIZE + HUD_ICONSIZESEL, cgs.media.selectShader);

	diff = 1;
	weap = 0;

	for (i = 0; i < 17; i++) {
		weap = cg.weaponSelect + i;

		if (!COM_BitCheck(bits, weap)) {
			continue;
		}

		if (COM_BitCheck(bits, i)) {
			CG_RegisterWeapon(i);
		}

		if (weap == cg.weaponSelect) {
			continue;
		}

		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * (diff++);
		dist = abs(x - HUD_X);

		if (dist > HUD_FADE_DIST) {
			dist = HUD_FADE_DIST;
		}

		fadecolor[3] = 1.0f - (dist / HUD_FADE_DIST);

		trap_R_SetColor(fadecolor);

		CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[weap].weaponIcon);
		// no ammo cross on top
		if (!cg.snap->ps.ammo[weap]) {
			CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cgs.media.noammoShader);
		}

		trap_R_SetColor(color);

		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * (diff - count - 1);
		dist = abs(x - HUD_X);

		if (dist > HUD_FADE_DIST) {
			dist = HUD_FADE_DIST;
		}

		fadecolor[3] = 1.0f - (dist / HUD_FADE_DIST);

		trap_R_SetColor(fadecolor);

		CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[weap].weaponIcon);
		// no ammo cross on top
		if (!cg.snap->ps.ammo[weap]) {
			CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cgs.media.noammoShader);
		}

		trap_R_SetColor(color);

	}

	diff = -1;
	weap = 0;

	for (i = 0; i < 17; i++) {
		weap = cg.weaponSelect - i;

		if (!COM_BitCheck(bits, weap)) {
			continue;
		}

		if (COM_BitCheck(bits, i)) {
			CG_RegisterWeapon(i);
		}

		if (weap == cg.weaponSelect) {
			continue;
		}

		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * (diff--);
		dist = abs(x - HUD_X);

		if (dist > HUD_FADE_DIST) {
			dist = HUD_FADE_DIST;
		}

		fadecolor[3] = 1.0f - (dist / HUD_FADE_DIST);

		trap_R_SetColor(fadecolor);

		CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[weap].weaponIcon);
		// no ammo cross on top
		if (!cg.snap->ps.ammo[weap]) {
			CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cgs.media.noammoShader);
		}

		trap_R_SetColor(color);

		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * (diff + count + 1);
		dist = abs(x - HUD_X);

		if (dist > HUD_FADE_DIST) {
			dist = HUD_FADE_DIST;
		}

		fadecolor[3] = 1.0f - (dist / HUD_FADE_DIST);

		trap_R_SetColor(fadecolor);

		CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[weap].weaponIcon);
		// no ammo cross on top
		if (!cg.snap->ps.ammo[weap]) {
			CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cgs.media.noammoShader);
		}

		trap_R_SetColor(color);

	}
	// draw the selected name
	if (cg_weapons[cg.weaponSelect].item) {
		name = cg_weapons[cg.weaponSelect].item->pickup_name;

		if (name) {
			CG_DrawString(SCREEN_WIDTH / 2, y - 6, name, UI_CENTER|UI_VA_BOTTOM|UI_DROPSHADOW|UI_SMALLFONT, color);
		}
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
CG_WeaponSelectable
=======================================================================================================================================
*/
static qboolean CG_WeaponSelectable(int i) {

	if (!cg.snap->ps.ammo[i]) {
		return qfalse;
	}
	// check for weapon
	if (!(COM_BitCheck(cg.predictedPlayerState.weapons, i))) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
CG_NextWeapon_f
=======================================================================================================================================
*/
void CG_NextWeapon_f(void) {
	int i;
	int original;

	if (!cg.snap) {
		return;
	}

	if (cg.snap->ps.pm_flags & PMF_FOLLOW) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for (i = 0; i < MAX_WEAPONS; i++) {
		cg.weaponSelect++;

		if (cg.weaponSelect == MAX_WEAPONS) {
			cg.weaponSelect = 0;
		}

		if (cg.weaponSelect == WP_GAUNTLET && cg_cyclePastGauntlet.integer) {
			continue; // never cycle to gauntlet
		}

		if (CG_WeaponSelectable(cg.weaponSelect)) {
			break;
		}
	}

	if (i == MAX_WEAPONS) {
		cg.weaponSelect = original;
	}
}

/*
=======================================================================================================================================
CG_PrevWeapon_f
=======================================================================================================================================
*/
void CG_PrevWeapon_f(void) {
	int i;
	int original;

	if (!cg.snap) {
		return;
	}

	if (cg.snap->ps.pm_flags & PMF_FOLLOW) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for (i = 0; i < MAX_WEAPONS; i++) {
		cg.weaponSelect--;

		if (cg.weaponSelect == -1) {
			cg.weaponSelect = MAX_WEAPONS - 1;
		}

		if (cg.weaponSelect == WP_GAUNTLET && cg_cyclePastGauntlet.integer) {
			continue; // never cycle to gauntlet
		}

		if (CG_WeaponSelectable(cg.weaponSelect)) {
			break;
		}
	}

	if (i == MAX_WEAPONS) {
		cg.weaponSelect = original;
	}
}

/*
=======================================================================================================================================
CG_Weapon_f
=======================================================================================================================================
*/
void CG_Weapon_f(void) {
	int num;

	if (!cg.snap) {
		return;
	}

	if (cg.snap->ps.pm_flags & PMF_FOLLOW) {
		return;
	}

	num = atoi(CG_Argv(1));

	if (num < 1 || num > MAX_WEAPONS - 1) {
		return;
	}

	cg.weaponSelectTime = cg.time;

	if (!(COM_BitCheck(cg.snap->ps.weapons, num))) {
		return; // don't have the weapon
	}

	cg.weaponSelect = num;
}

/*
=======================================================================================================================================
CG_OutOfAmmoChange

The current weapon has just run out of ammo.
=======================================================================================================================================
*/
void CG_OutOfAmmoChange(void) {
	int i;

	cg.weaponSelectTime = cg.time;

	for (i = MAX_WEAPONS - 1; i > 0; i--) {
		if (CG_WeaponSelectable(i)) {
			cg.weaponSelect = i;
			break;
		}
	}
}
