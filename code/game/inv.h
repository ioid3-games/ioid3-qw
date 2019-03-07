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

#define INVENTORY_NONE				  0
// health
#define INVENTORY_HEALTH			  1
// armor
#define INVENTORY_ARMOR				  2
// weapons
#define INVENTORY_GAUNTLET			  3
#define INVENTORY_MACHINEGUN		  4
#define INVENTORY_CHAINGUN			  5
#define INVENTORY_SHOTGUN			  6
#define INVENTORY_NAILGUN			  7
#define INVENTORY_PROXLAUNCHER		  8
#define INVENTORY_GRENADELAUNCHER	  9
#define INVENTORY_ROCKETLAUNCHER	 10
#define INVENTORY_BEAMGUN			 11
#define INVENTORY_RAILGUN			 12
#define INVENTORY_PLASMAGUN			 13
#define INVENTORY_BFG10K			 14
#define INVENTORY_GRAPPLINGHOOK		 15
// ammo
#define INVENTORY_BULLETS			 16
#define INVENTORY_BELT				 17
#define INVENTORY_SHELLS			 18
#define INVENTORY_NAILS				 19
#define INVENTORY_GRENADES			 20
#define INVENTORY_MINES				 21
#define INVENTORY_ROCKETS			 22
#define INVENTORY_BEAMGUN_AMMO		 23
#define INVENTORY_SLUGS				 24
#define INVENTORY_CELLS				 25
#define INVENTORY_BFG_AMMO			 26
// holdables
#define INVENTORY_TELEPORTER		 27
#define INVENTORY_MEDKIT			 28
#define INVENTORY_KAMIKAZE			 29
#define INVENTORY_PORTAL			 30
#define INVENTORY_INVULNERABILITY	 31
// powerups
#define INVENTORY_QUAD				 32
#define INVENTORY_ENVIRONMENTSUIT	 33
#define INVENTORY_HASTE				 34
#define INVENTORY_INVISIBILITY		 35
#define INVENTORY_REGEN				 36
#define INVENTORY_FLIGHT			 37
#define INVENTORY_AMMOREGEN			 38
#define INVENTORY_GUARD				 39
#define INVENTORY_DOUBLER			 40
#define INVENTORY_SCOUT				 41
#define INVENTORY_REDFLAG			 42
#define INVENTORY_BLUEFLAG			 43
#define INVENTORY_NEUTRALFLAG		 44
#define INVENTORY_REDCUBE			 45
#define INVENTORY_BLUECUBE			 46
// enemy stuff
#define ENEMY_HORIZONTAL_DIST		200
#define ENEMY_HEIGHT				201
#define NUM_VISIBLE_ENEMIES			202
#define NUM_VISIBLE_TEAMMATES		203

//************************************************************************
// ITEM NUMBERS (make sure they are in sync with bg_itemlist in bg_misc.c)
//************************************************************************
#define MODELINDEX_HEALTHSMALL		1
#define MODELINDEX_HEALTH			2
#define MODELINDEX_HEALTHLARGE		3
#define MODELINDEX_HEALTHMEGA		4
#define MODELINDEX_ARMORSHARD		5
#define MODELINDEX_ARMORCOMBAT		6
#define MODELINDEX_ARMORBODY		7
#define MODELINDEX_GAUNTLET			8
#define MODELINDEX_MACHINEGUN		9
#define MODELINDEX_CHAINGUN			10
#define MODELINDEX_SHOTGUN			11
#define MODELINDEX_NAILGUN			12
#define MODELINDEX_PROXLAUNCHER		13
#define MODELINDEX_GRENADELAUNCHER	14
#define MODELINDEX_ROCKETLAUNCHER	15
#define MODELINDEX_BEAMGUN			16
#define MODELINDEX_RAILGUN			17
#define MODELINDEX_PLASMAGUN		18
#define MODELINDEX_BFG10K			19
#define MODELINDEX_GRAPPLINGHOOK	20
// ammo
#define MODELINDEX_BULLETS			21
#define MODELINDEX_BELT				22
#define MODELINDEX_SHELLS			23
#define MODELINDEX_NAILS			24
#define MODELINDEX_MINES			25
#define MODELINDEX_GRENADES			26
#define MODELINDEX_ROCKETS			27
#define MODELINDEX_BEAMGUN_AMMO		28
#define MODELINDEX_SLUGS			29
#define MODELINDEX_CELLS			30
#define MODELINDEX_BFG_AMMO			31
// holdables
#define MODELINDEX_TELEPORTER		32
#define MODELINDEX_MEDKIT			33
#define MODELINDEX_KAMIKAZE			34
#define MODELINDEX_PORTAL			35
#define MODELINDEX_INVULNERABILITY	36
// powerups
#define MODELINDEX_QUAD				37
#define MODELINDEX_ENVIRONMENTSUIT	38
#define MODELINDEX_HASTE			39
#define MODELINDEX_INVISIBILITY		40
#define MODELINDEX_REGEN			41
#define MODELINDEX_FLIGHT			42
// mission pack only defines
#define MODELINDEX_AMMOREGEN		43
#define MODELINDEX_GUARD			44
#define MODELINDEX_DOUBLER			45
#define MODELINDEX_SCOUT			46
// team items
#define MODELINDEX_REDFLAG			47
#define MODELINDEX_BLUEFLAG			48
#define MODELINDEX_NEUTRALFLAG		49
#define MODELINDEX_REDCUBE			50
#define MODELINDEX_BLUECUBE			51
//************************************************************************
// WEAPON NUMBERS
//************************************************************************
#define WEAPONINDEX_GAUNTLET			1
#define WEAPONINDEX_MACHINEGUN			2
#define WEAPONINDEX_CHAINGUN			3
#define WEAPONINDEX_SHOTGUN				4
#define WEAPONINDEX_NAILGUN				5
#define WEAPONINDEX_PROXLAUNCHER		6
#define WEAPONINDEX_GRENADELAUNCHER		7
#define WEAPONINDEX_ROCKETLAUNCHER		8
#define WEAPONINDEX_BEAMGUN				9
#define WEAPONINDEX_RAILGUN				10
#define WEAPONINDEX_PLASMAGUN			11
#define WEAPONINDEX_BFG					12
#define WEAPONINDEX_GRAPPLING_HOOK		13
