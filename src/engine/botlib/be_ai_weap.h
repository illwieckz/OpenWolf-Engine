////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2010 id Software LLC, a ZeniMax Media company.
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of the OpenWolf GPL Source Code.
// OpenWolf Source Code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenWolf Source Code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf Source Code.  If not, see <http://www.gnu.org/licenses/>.
//
// In addition, the OpenWolf Source Code is also subject to certain additional terms.
// You should have received a copy of these additional terms immediately following the
// terms and conditions of the GNU General Public License which accompanied the
// OpenWolf Source Code. If not, please request a copy in writing from id Software
// at the address below.
//
// If you have questions concerning this license or the applicable additional terms,
// you may contact in writing id Software LLC, c/o ZeniMax Media Inc.,
// Suite 120, Rockville, Maryland 20850 USA.
//
// -------------------------------------------------------------------------------------
// File name:   be_ai_weap.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AI_WEAP_H__
#define __BE_AI_WEAP_H__

//projectile flags
#define PFL_WINDOWDAMAGE			1		//projectile damages through window
#define PFL_RETURN					2		//set when projectile returns to owner
//weapon flags
#define WFL_FIRERELEASED			1		//set when projectile is fired with key-up event
//damage types
#define DAMAGETYPE_IMPACT			1		//damage on impact
#define DAMAGETYPE_RADIAL			2		//radial damage
#define DAMAGETYPE_VISIBLE			4		//damage to all entities visible to the projectile

typedef struct projectileinfo_s
{
    UTF8 name[MAX_STRINGFIELD];
    UTF8 model[MAX_STRINGFIELD];
    S32 flags;
    F32 gravity;
    S32 damage;
    F32 radius;
    S32 visdamage;
    S32 damagetype;
    S32 healthinc;
    F32 push;
    F32 detonation;
    F32 bounce;
    F32 bouncefric;
    F32 bouncestop;
} projectileinfo_t;

typedef struct weaponinfo_s
{
    S32 valid;					//true if the weapon info is valid
    S32 number;									//number of the weapon
    UTF8 name[MAX_STRINGFIELD];
    UTF8 model[MAX_STRINGFIELD];
    S32 level;
    S32 weaponindex;
    S32 flags;
    UTF8 projectile[MAX_STRINGFIELD];
    S32 numprojectiles;
    F32 hspread;
    F32 vspread;
    F32 speed;
    F32 acceleration;
    vec3_t recoil;
    vec3_t offset;
    vec3_t angleoffset;
    F32 extrazvelocity;
    S32 ammoamount;
    S32 ammoindex;
    F32 activate;
    F32 reload;
    F32 spinup;
    F32 spindown;
    projectileinfo_t proj;						//pointer to the used projectile
} weaponinfo_t;

//setup the weapon AI
S32 BotSetupWeaponAI( void );
//shut down the weapon AI
void BotShutdownWeaponAI( void );
//returns the best weapon to fight with
S32 BotChooseBestFightWeapon( S32 weaponstate, S32* inventory );
//returns the information of the current weapon
void BotGetWeaponInfo( S32 weaponstate, S32 weapon, weaponinfo_t* weaponinfo );
//loads the weapon weights
S32 BotLoadWeaponWeights( S32 weaponstate, UTF8* filename );
//returns a handle to a newly allocated weapon state
S32 BotAllocWeaponState( void );
//frees the weapon state
void BotFreeWeaponState( S32 weaponstate );
//resets the whole weapon state
void BotResetWeaponState( S32 weaponstate );

#endif //!__BE_AI_WEAP_H__
