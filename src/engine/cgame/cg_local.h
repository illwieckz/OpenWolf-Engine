////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2005 Id Software, Inc.
// Copyright(C) 2000 - 2006 Tim Angus
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of OpenWolf.
//
// OpenWolf is free software; you can redistribute it
// and / or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// OpenWolf is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 - 1301  USA
//
// -------------------------------------------------------------------------------------
// File name:   cg_local.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_LOCAL_H__
#define __CG_LOCAL_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef __BG_PUBLIC_H__
#include <bgame/bg_public.h>
#endif
#ifndef __CG_API_H__
#include <client/cg_api.h>
#endif
#ifndef __GUI_PUBLIC_H__
#include <GUI/gui_api.h>
#endif
#ifndef __GUI_SHARED_H__
#include <GUI/gui_shared.h>
#endif

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#define CG_FONT_THRESHOLD 0.1

#define POWERUP_BLINKS      5

#define POWERUP_BLINK_TIME  1000
#define FADE_TIME           200
#define PULSE_TIME          200
#define DAMAGE_DEFLECT_TIME 100
#define DAMAGE_RETURN_TIME  400
#define DAMAGE_TIME         500
#define LAND_DEFLECT_TIME   150
#define LAND_RETURN_TIME    300
#define DUCK_TIME           100
#define PAIN_TWITCH_TIME    200
#define WEAPON_SELECT_TIME  1400
#define ITEM_SCALEUP_TIME   1000
#define ZOOM_TIME           150
#define ITEM_BLOB_TIME      200
#define MUZZLE_FLASH_TIME   20
#define SINK_TIME           1000    // time for fragments to sink into ground before going away
#define ATTACKER_HEAD_TIME  10000
#define REWARD_TIME         3000

#define PULSE_SCALE         1.5     // amount to scale up the icons when activating

#define MAX_STEP_CHANGE     32

#define MAX_VERTS_ON_POLY   10
#define MAX_MARK_POLYS      256

#define STAT_MINUS          10  // num frame for '-' stats digit

#define ICON_SIZE           48
#define CHAR_WIDTH          32
#define CHAR_HEIGHT         48
#define TEXT_ICON_SPACE     4

// very large characters
#define GIANT_WIDTH         32
#define GIANT_HEIGHT        48

#define NUM_CROSSHAIRS      10

#define TEAM_OVERLAY_MAXNAME_WIDTH  12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH  16

typedef enum
{
    FOOTSTEP_NORMAL,
    FOOTSTEP_FLESH,
    FOOTSTEP_METAL,
    FOOTSTEP_SPLASH,
    FOOTSTEP_CUSTOM,
    FOOTSTEP_NONE,
    
    FOOTSTEP_TOTAL
} footstep_t;

typedef enum
{
    IMPACTSOUND_DEFAULT,
    IMPACTSOUND_METAL,
    IMPACTSOUND_FLESH
} impactSound_t;

typedef enum
{
    JPS_OFF,
    JPS_DESCENDING,
    JPS_HOVERING,
    JPS_ASCENDING
} jetPackState_t;

//======================================================================

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct
{
    S32         oldFrame;
    S32         oldFrameTime;     // time when ->oldFrame was exactly on
    
    S32         frame;
    S32         frameTime;        // time when ->frame will be exactly on
    
    F32       backlerp;
    
    F32       yawAngle;
    bool    yawing;
    F32       pitchAngle;
    bool    pitching;
    
    S32         animationNumber;  // may include ANIM_TOGGLEBIT
    animation_t* animation;
    S32         animationTime;    // time when the first frame of the animation will be exact
} lerpFrame_t;

//======================================================================

//attachment system
typedef enum
{
    AT_STATIC,
    AT_TAG,
    AT_CENT,
    AT_PARTICLE
} attachmentType_t;

//forward declaration for particle_t
struct particle_s;

typedef struct attachment_s
{
    attachmentType_t  type;
    bool          attached;
    
    bool          staticValid;
    bool          tagValid;
    bool          centValid;
    bool          particleValid;
    
    bool          hasOffset;
    vec3_t            offset;
    
    vec3_t            lastValidAttachmentPoint;
    
    //AT_STATIC
    vec3_t            origin;
    
    //AT_TAG
    refEntity_t       re;     //FIXME: should be pointers?
    refEntity_t       parent; //
    qhandle_t         model;
    UTF8              tagName[ MAX_STRING_CHARS ];
    
    //AT_CENT
    S32               centNum;
    
    //AT_PARTICLE
    struct particle_s* particle;
} attachment_t;

//======================================================================

//particle system stuff
#define MAX_PARTICLE_FILES        128

#define MAX_PS_SHADER_FRAMES      32
#define MAX_PS_MODELS             8
#define MAX_EJECTORS_PER_SYSTEM   4
#define MAX_PARTICLES_PER_EJECTOR 4

#define MAX_BASEPARTICLE_SYSTEMS  192
#define MAX_BASEPARTICLE_EJECTORS MAX_BASEPARTICLE_SYSTEMS*MAX_EJECTORS_PER_SYSTEM
#define MAX_BASEPARTICLES         MAX_BASEPARTICLE_EJECTORS*MAX_PARTICLES_PER_EJECTOR

#define MAX_PARTICLE_SYSTEMS      48
#define MAX_PARTICLE_EJECTORS     MAX_PARTICLE_SYSTEMS*MAX_EJECTORS_PER_SYSTEM
#define MAX_PARTICLES             MAX_PARTICLE_EJECTORS*5

#define PARTICLES_INFINITE        -1
#define PARTICLES_SAME_AS_INITIAL -2

//COMPILE TIME STRUCTURES
typedef enum
{
    PMT_STATIC,
    PMT_STATIC_TRANSFORM,
    PMT_TAG,
    PMT_CENT_ANGLES,
    PMT_NORMAL
} pMoveType_t;

typedef enum
{
    PMD_LINEAR,
    PMD_POINT
} pDirType_t;

typedef struct pMoveValues_u
{
    pDirType_t  dirType;
    
    //PMD_LINEAR
    vec3_t      dir;
    F32       dirRandAngle;
    
    //PMD_POINT
    vec3_t      point;
    F32       pointRandAngle;
    
    F32       mag;
    F32       magRandFrac;
    
    F32       parentVelFrac;
    F32       parentVelFracRandFrac;
} pMoveValues_t;

typedef struct pLerpValues_s
{
    S32   delay;
    F32 delayRandFrac;
    
    F32 initial;
    F32 initialRandFrac;
    
    F32 final;
    F32 finalRandFrac;
    
    F32 randFrac;
} pLerpValues_t;

//particle template
typedef struct baseParticle_s
{
    vec3_t          displacement;
    F32           randDisplacement;
    F32           normalDisplacement;
    
    pMoveType_t     velMoveType;
    pMoveValues_t   velMoveValues;
    
    pMoveType_t     accMoveType;
    pMoveValues_t   accMoveValues;
    
    S32             lifeTime;
    F32           lifeTimeRandFrac;
    
    F32           bounceFrac;
    F32           bounceFracRandFrac;
    bool        bounceCull;
    
    UTF8            bounceMarkName[ MAX_QPATH ];
    qhandle_t       bounceMark;
    F32           bounceMarkRadius;
    F32           bounceMarkRadiusRandFrac;
    F32           bounceMarkCount;
    F32           bounceMarkCountRandFrac;
    
    UTF8            bounceSoundName[ MAX_QPATH ];
    qhandle_t       bounceSound;
    F32           bounceSoundCount;
    F32           bounceSoundCountRandFrac;
    
    pLerpValues_t   radius;
    S32             physicsRadius;
    pLerpValues_t   alpha;
    pLerpValues_t   rotation;
    
    bool        dynamicLight;
    pLerpValues_t   dLightRadius;
    U8            dLightColor[ 3 ];
    
    S32             colorDelay;
    F32           colorDelayRandFrac;
    U8            initialColor[ 3 ];
    U8            finalColor[ 3 ];
    
    UTF8            childSystemName[ MAX_QPATH ];
    qhandle_t       childSystemHandle;
    
    UTF8            onDeathSystemName[ MAX_QPATH ];
    qhandle_t       onDeathSystemHandle;
    
    UTF8            childTrailSystemName[ MAX_QPATH ];
    qhandle_t       childTrailSystemHandle;
    
    //particle invariant stuff
    UTF8            shaderNames[ MAX_PS_SHADER_FRAMES ][ MAX_QPATH ];
    qhandle_t       shaders[ MAX_PS_SHADER_FRAMES ];
    S32             numFrames;
    F32           framerate;
    
    UTF8            modelNames[ MAX_PS_MODELS ][ MAX_QPATH ];
    qhandle_t       models[ MAX_PS_MODELS ];
    S32             numModels;
    animation_t     modelAnimation;
    
    bool        overdrawProtection;
    bool        realLight;
    bool        cullOnStartSolid;
    
    F32           scaleWithCharge;
} baseParticle_t;


//ejector template
typedef struct baseParticleEjector_s
{
    baseParticle_t*  particles[ MAX_PARTICLES_PER_EJECTOR ];
    S32             numParticles;
    
    pLerpValues_t   eject;          //zero period indicates creation of all particles at once
    
    S32             totalParticles;         //can be infinite
    F32           totalParticlesRandFrac;
} baseParticleEjector_t;


//particle system template
typedef struct baseParticleSystem_s
{
    UTF8                  name[ MAX_QPATH ];
    baseParticleEjector_t* ejectors[ MAX_EJECTORS_PER_SYSTEM ];
    S32                   numEjectors;
    
    bool              thirdPersonOnly;
    bool              registered; //whether or not the assets for this particle have been loaded
} baseParticleSystem_t;


//RUN TIME STRUCTURES
typedef struct particleSystem_s
{
    baseParticleSystem_t*  _class;
    
    attachment_t          attachment;
    
    bool              valid;
    bool              lazyRemove; //mark this system for later removal
    
    //for PMT_NORMAL
    bool              normalValid;
    vec3_t                normal;
    
    S32                   charge;
} particleSystem_t;


typedef struct particleEjector_s
{
    baseParticleEjector_t* _class;
    particleSystem_t*      parent;
    
    pLerpValues_t         ejectPeriod;
    
    S32                   count;
    S32                   totalParticles;
    
    S32                   nextEjectionTime;
    
    bool              valid;
} particleEjector_t;


//used for actual particle evaluation
typedef struct particle_s
{
    baseParticle_t*    _class;
    particleEjector_t* parent;
    
    S32               birthTime;
    S32               lifeTime;
    
    F32             bounceMarkRadius;
    S32               bounceMarkCount;
    S32               bounceSoundCount;
    bool          atRest;
    
    vec3_t            origin;
    vec3_t            velocity;
    
    pMoveType_t       accMoveType;
    pMoveValues_t     accMoveValues;
    
    S32               lastEvalTime;
    
    S32               nextChildTime;
    
    pLerpValues_t     radius;
    pLerpValues_t     alpha;
    pLerpValues_t     rotation;
    
    pLerpValues_t     dLightRadius;
    
    S32               colorDelay;
    
    qhandle_t         model;
    lerpFrame_t       lf;
    vec3_t            lastAxis[ 3 ];
    
    bool          valid;
    S32               frameWhenInvalidated;
    
    S32               sortKey;
} particle_t;

//======================================================================

//trail system stuff
#define MAX_TRAIL_FILES           128

#define MAX_BEAMS_PER_SYSTEM      4

#define MAX_BASETRAIL_SYSTEMS     64
#define MAX_BASETRAIL_BEAMS       MAX_BASETRAIL_SYSTEMS*MAX_BEAMS_PER_SYSTEM

#define MAX_TRAIL_SYSTEMS         32
#define MAX_TRAIL_BEAMS           MAX_TRAIL_SYSTEMS*MAX_BEAMS_PER_SYSTEM
#define MAX_TRAIL_BEAM_NODES      128

#define MAX_TRAIL_BEAM_JITTERS    4

typedef enum
{
    TBTT_STRETCH,
    TBTT_REPEAT
} trailBeamTextureType_t;

typedef struct baseTrailJitter_s
{
    F32   magnitude;
    S32     period;
} baseTrailJitter_t;

//beam template
typedef struct baseTrailBeam_s
{
    S32                     numSegments;
    F32                   frontWidth;
    F32                   backWidth;
    F32                   frontAlpha;
    F32                   backAlpha;
    U8                    frontColor[ 3 ];
    U8                    backColor[ 3 ];
    
    // the time it takes for a segment to vanish (single attached only)
    S32                     segmentTime;
    
    // the time it takes for a beam to fade out (double attached only)
    S32                     fadeOutTime;
    
    UTF8                    shaderName[ MAX_QPATH ];
    qhandle_t               shader;
    
    trailBeamTextureType_t  textureType;
    
    //TBTT_STRETCH
    F32                   frontTextureCoord;
    F32                   backTextureCoord;
    
    //TBTT_REPEAT
    F32                   repeatLength;
    bool                clampToBack;
    
    bool                realLight;
    
    S32                     numJitters;
    baseTrailJitter_t       jitters[ MAX_TRAIL_BEAM_JITTERS ];
    bool                jitterAttachments;
} baseTrailBeam_t;


//trail system template
typedef struct baseTrailSystem_s
{
    UTF8            name[ MAX_QPATH ];
    baseTrailBeam_t* beams[ MAX_BEAMS_PER_SYSTEM ];
    S32             numBeams;
    
    S32             lifeTime;
    bool        thirdPersonOnly;
    bool        registered; //whether or not the assets for this trail have been loaded
} baseTrailSystem_t;

typedef struct trailSystem_s
{
    baseTrailSystem_t*   _class;
    
    attachment_t        frontAttachment;
    attachment_t        backAttachment;
    
    S32                 birthTime;
    S32                 destroyTime;
    bool            valid;
} trailSystem_t;

typedef struct trailBeamNode_s
{
    vec3_t                  refPosition;
    vec3_t                  position;
    
    S32                     timeLeft;
    
    F32                   textureCoord;
    F32                   halfWidth;
    U8                    alpha;
    U8                    color[ 3 ];
    
    vec2_t                  jitters[ MAX_TRAIL_BEAM_JITTERS ];
    
    struct trailBeamNode_s*  prev;
    struct trailBeamNode_s*  next;
    
    bool                used;
} trailBeamNode_t;

typedef struct trailBeam_s
{
    baseTrailBeam_t*   _class;
    trailSystem_t*     parent;
    
    trailBeamNode_t   nodePool[ MAX_TRAIL_BEAM_NODES ];
    trailBeamNode_t*   nodes;
    
    S32               lastEvalTime;
    
    bool          valid;
    
    S32               nextJitterTimes[ MAX_TRAIL_BEAM_JITTERS ];
} trailBeam_t;

//======================================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// smoothing of view and model for WW transitions
#define   MAXSMOOTHS          32

typedef struct
{
    F32     time;
    F32     timeMod;
    
    vec3_t    rotAxis;
    F32     rotAngle;
} smooth_t;


typedef struct
{
    lerpFrame_t legs, torso, nonseg, weapon;
    S32         painTime;
    S32         painDirection;  // flip from 0 to 1
    
    bool    squadMarked;    // player has been marked as a squadmember
    
    // machinegun spinning
    F32       barrelAngle;
    S32         barrelTime;
    bool    barrelSpinning;
    
    refEntity_t bodyRefEnt, headRefEnt, gunRefEnt;
    vec3_t      lastNormal;
    vec3_t      lastAxis[ 3 ];
    smooth_t    sList[ MAXSMOOTHS ];
} playerEntity_t;

typedef struct lightFlareStatus_s
{
    F32     lastRadius;    //caching of likely flare radius
    F32     lastRatio;     //caching of likely flare ratio
    S32       lastTime;      //last time flare was visible/occluded
    bool  status;        //flare is visble?
} lightFlareStatus_t;

typedef struct buildableStatus_s
{
    S32       lastTime;      // Last time status was visible
    bool  visible;       // Status is visble?
} buildableStatus_t;

typedef struct buildableCache_s
{
    vec3_t   cachedOrigin;   // If either the cached entity origin or the
    vec3_t   cachedNormal;   // cached surfNormal change the cache is invalid
    vec3_t   axis[ 3 ];
    vec3_t   origin;
} buildableCache_t;

//=================================================

// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s
{
    entityState_t         currentState;     // from cg.frame
    entityState_t         nextState;        // from cg.nextFrame, if available
    bool              interpolate;      // true if next is valid to interpolate to
    bool              currentValid;     // true if cg.frame holds this entity
    
    S32                   muzzleFlashTime;  // move to playerEntity?
    S32                   muzzleFlashTime2; // move to playerEntity?
    S32                   muzzleFlashTime3; // move to playerEntity?
    S32                   previousEvent;
    S32                   teleportFlag;
    
    S32                   trailTime;        // so missile trails can handle dropped initial packets
    S32                   dustTrailTime;
    S32                   miscTime;
    S32                   soundTime;
    S32                   snapShotTime;     // last time this entity was found in a snapshot
    
    playerEntity_t        pe;
    
    S32                   errorTime;        // decay the error from this time
    vec3_t                errorOrigin;
    vec3_t                errorAngles;
    
    bool              extrapolated;     // false if origin / angles is an interpolation
    vec3_t                rawOrigin;
    vec3_t                rawAngles;
    
    vec3_t                beamEnd;
    
    // exact interpolated position of entity on this frame
    vec3_t                lerpOrigin;
    vec3_t                lerpAngles;
    
    lerpFrame_t           lerpFrame;
    
    buildableAnimNumber_t buildableAnim;    //persistant anim number
    buildableAnimNumber_t oldBuildableAnim; //to detect when new anims are set
    particleSystem_t*      buildablePS;     //handles things like smoke/blood when heavily damaged
    particleSystem_t*      buildableHitPS;  //handles when a buildable is hit
    buildableStatus_t     buildableStatus;
    buildableCache_t      buildableCache;   // so we don't recalculate things
    F32                 lastBuildableHealthScale;
    S32                   lastBuildableDamageSoundTime;
    
    lightFlareStatus_t    lfs;
    
    bool              doorState;
    
    bool              animInit;
    bool              animPlaying;
    bool              animLastState;
    
    particleSystem_t*      muzzlePS;
    bool              muzzlePsTrigger;
    
    particleSystem_t*      jetPackPS;
    jetPackState_t        jetPackState;
    
    particleSystem_t*      poisonCloudedPS;
    
    particleSystem_t*      entityPS;
    bool              entityPSMissing;
    
    trailSystem_t*         level2ZapTS[ 3 ];
    S32                   level2ZapTime;
    
    trailSystem_t*         muzzleTS; //used for the tesla and reactor
    S32                   muzzleTSDeathTime;
    
    bool              valid;
    bool              oldValid;
    struct centity_s*      nextLocation;
} centity_t;


//======================================================================

typedef struct markPoly_s
{
    struct markPoly_s* prevMark, *nextMark;
    S32               time;
    qhandle_t         markShader;
    bool          alphaFade;    // fade alpha instead of rgb
    F32             color[ 4 ];
    poly_t            poly;
    polyVert_t        verts[ MAX_VERTS_ON_POLY ];
} markPoly_t;

//======================================================================


typedef struct
{
    S32       client;
    S32       score;
    S32       ping;
    S32       time;
    S32       team;
    weapon_t  weapon;
    upgrade_t upgrade;
} score_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define MAX_CUSTOM_SOUNDS 32
typedef struct
{
    bool    infoValid;
    
    S32 clientNum;
    
    UTF8        name[ MAX_QPATH ];
    team_t      team;
    
    S32         botSkill;               // 0 = not bot, 1-5 = bot
    
    vec3_t      color1;
    vec3_t      color2;
    
    S32         score;                      // updated by score servercmds
    S32         location;                   // location index for team mode
    S32         health;                     // you only get this info about your teammates
    S32         armor;
    S32         curWeapon;
    
    S32         handicap;
    
    // when clientinfo is changed, the loading of models/skins/sounds
    // can be deferred until you are dead, to prevent hitches in
    // gameplay
    UTF8        modelName[ MAX_QPATH ];
    UTF8        skinName[ MAX_QPATH ];
    
    bool    newAnims;                   // true if using the new mission pack animations
    bool    fixedlegs;                  // true if legs yaw is always the same as torso yaw
    bool    fixedtorso;                 // true if torso never changes yaw
    bool    nonsegmented;               // true if model is Q2 style nonsegmented
    
    vec3_t      headOffset;                 // move head in icon views
    footstep_t  footsteps;
    gender_t    gender;                     // from model
    
    qhandle_t   legsModel;
    qhandle_t   legsSkin;
    
    qhandle_t   torsoModel;
    qhandle_t   torsoSkin;
    
    qhandle_t   headModel;
    qhandle_t   headSkin;
    
    qhandle_t   nonSegModel;                //non-segmented model system
    qhandle_t   nonSegSkin;                 //non-segmented model system
    
    qhandle_t   modelIcon;
    
    animation_t animations[ MAX_PLAYER_TOTALANIMATIONS ];
    
    sfxHandle_t sounds[ MAX_CUSTOM_SOUNDS ];
    
    sfxHandle_t customFootsteps[ 4 ];
    sfxHandle_t customMetalFootsteps[ 4 ];
    
    UTF8        voice[ MAX_VOICE_NAME_LEN ];
    S32         voiceTime;
} clientInfo_t;


typedef struct weaponInfoMode_s
{
    F32       flashDlight;
    vec3_t      flashDlightColor;
    sfxHandle_t flashSound[ 4 ];  // fast firing weapons randomly choose
    bool    continuousFlash;
    
    qhandle_t   missileModel;
    sfxHandle_t missileSound;
    F32       missileDlight;
    vec3_t      missileDlightColor;
    S32         missileRenderfx;
    bool    usesSpriteMissle;
    qhandle_t   missileSprite;
    S32         missileSpriteSize;
    F32       missileSpriteCharge;
    qhandle_t   missileParticleSystem;
    qhandle_t   missileTrailSystem;
    bool    missileRotates;
    bool    missileAnimates;
    S32         missileAnimStartFrame;
    S32         missileAnimNumFrames;
    S32         missileAnimFrameRate;
    S32         missileAnimLooping;
    
    sfxHandle_t firingSound;
    bool    loopFireSound;
    
    qhandle_t   muzzleParticleSystem;
    
    bool    alwaysImpact;
    qhandle_t   impactParticleSystem;
    qhandle_t   impactMark;
    qhandle_t   impactMarkSize;
    sfxHandle_t impactSound[ 4 ]; //random impact sound
    sfxHandle_t impactFleshSound[ 4 ]; //random impact sound
} weaponInfoMode_t;

// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s
{
    bool          registered;
    UTF8*              humanName;
    
    qhandle_t         handsModel;       // the hands don't actually draw, they just position the weapon
    qhandle_t         weaponModel;
    qhandle_t         barrelModel;
    qhandle_t         flashModel;
    
    qhandle_t         weaponModel3rdPerson;
    qhandle_t         barrelModel3rdPerson;
    qhandle_t         flashModel3rdPerson;
    
    animation_t       animations[ MAX_WEAPON_ANIMATIONS ];
    bool          noDrift;
    
    vec3_t            weaponMidpoint;   // so it will rotate centered instead of by tag
    
    qhandle_t         weaponIcon;
    qhandle_t         ammoIcon;
    
    qhandle_t         crossHair;
    S32               crossHairSize;
    
    sfxHandle_t       readySound;
    
    bool          disableIn3rdPerson;
    
    weaponInfoMode_t  wim[ WPM_NUM_WEAPONMODES ];
} weaponInfo_t;

typedef struct upgradeInfo_s
{
    bool    registered;
    UTF8*        humanName;
    
    qhandle_t   upgradeIcon;
} upgradeInfo_t;

typedef struct
{
    bool    looped;
    bool    enabled;
    
    sfxHandle_t sound;
} sound_t;

typedef struct
{
    qhandle_t   models[ MAX_BUILDABLE_MODELS ];
    animation_t animations[ MAX_BUILDABLE_ANIMATIONS ];
    
    //same number of sounds as animations
    sound_t     sounds[ MAX_BUILDABLE_ANIMATIONS ];
} buildableInfo_t;

#define MAX_REWARDSTACK   10
#define MAX_SOUNDBUFFER   20

//======================================================================

typedef struct
{
    vec3_t    alienBuildablePos[ MAX_GENTITIES ];
    S32       alienBuildableTimes[ MAX_GENTITIES ];
    S32       numAlienBuildables;
    
    vec3_t    humanBuildablePos[ MAX_GENTITIES ];
    S32       numHumanBuildables;
    
    vec3_t    alienClientPos[ MAX_CLIENTS ];
    S32       numAlienClients;
    
    vec3_t    humanClientPos[ MAX_CLIENTS ];
    S32       numHumanClients;
    
    S32       lastUpdateTime;
    vec3_t    origin;
    vec3_t    vangles;
} entityPos_t;

typedef struct
{
    S32 time;
    S32 length;
} consoleLine_t;

#define MAX_CONSOLE_TEXT  8192
#define MAX_CONSOLE_LINES 32

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS  16

#define NUM_SAVED_STATES ( CMD_BACKUP + 2 )

// After this many msec the crosshair name fades out completely
#define CROSSHAIR_CLIENT_TIMEOUT 1000

typedef struct
{
    S32           clientFrame;                        // incremented each frame
    
    S32           clientNum;
    
    bool      demoPlayback;
    bool      levelShot;                          // taking a level menu screenshot
    S32           deferredPlayerLoading;
    bool      loading;                            // don't defer players at initial startup
    bool      intermissionStarted;                // don't play voice rewards, because game will end shortly
    
    // there are only one or two snapshot_t that are relevent at a time
    S32           latestSnapshotNum;                  // the number of snapshots the client system has received
    S32           latestSnapshotTime;                 // the time from latestSnapshotNum, so we don't need to read the snapshot yet
    
    snapshot_t*    snap;                              // cg.snap->serverTime <= cg.time
    snapshot_t*    nextSnap;                          // cg.nextSnap->serverTime > cg.time, or NULL
    snapshot_t    activeSnapshots[ 2 ];
    
    F32         frameInterpolation;                 // (F32)( cg.time - cg.frame->serverTime ) /
    // (cg.nextFrame->serverTime - cg.frame->serverTime)
    
    bool      thisFrameTeleport;
    bool      nextFrameTeleport;
    
    S32           frametime;                          // cg.time - cg.oldTime
    
    S32           time;                               // this is the time value that the client
    // is rendering at.
    S32           oldTime;                            // time at last frame, used for missile trails and prediction checking
    
    S32           physicsTime;                        // either cg.snap->time or cg.nextSnap->time
    
    S32           timelimitWarnings;                  // 5 min, 1 min, overtime
    S32           fraglimitWarnings;
    
    bool      mapRestart;                         // set on a map restart to set back the weapon
    
    bool      renderingThirdPerson;               // during deaths, chasecams, etc
    
    // prediction state
    bool      hyperspace;                         // true if prediction has hit a trigger_teleport
    playerState_t predictedPlayerState;
    pmoveExt_t    pmext;
    centity_t     predictedPlayerEntity;
    bool      validPPS;                           // clear until the first call to CG_PredictPlayerState
    S32           predictedErrorTime;
    vec3_t        predictedError;
    
    S32           eventSequence;
    S32           predictableEvents[MAX_PREDICTED_EVENTS];
    
    F32         stepChange;                         // for stair up smoothing
    S32           stepTime;
    
    F32         duckChange;                         // for duck viewheight smoothing
    S32           duckTime;
    
    F32         landChange;                         // for landing hard
    S32           landTime;
    
    // input state sent to server
    S32           weaponSelect;
    
    // auto rotating items
    vec3_t        autoAngles;
    vec3_t        autoAxis[ 3 ];
    vec3_t        autoAnglesFast;
    vec3_t        autoAxisFast[ 3 ];
    
    // view rendering
    refdef_t      refdef;
    vec3_t        refdefViewAngles;                   // will be converted to refdef.viewaxis
    
    // zoom key
    bool      zoomed;
    S32           zoomTime;
    F32         zoomSensitivity;
    
    S32			identifyClientRequest;
    
    // information screen text during loading
    UTF8          infoScreenText[ MAX_STRING_CHARS ];
    
    // scoreboard
    S32           scoresRequestTime;
    S32           numScores;
    S32           selectedScore;
    S32           teamScores[ 2 ];
    score_t       scores[MAX_CLIENTS];
    bool      showScores;
    bool      scoreBoardShowing;
    S32           scoreFadeTime;
    UTF8          killerName[ MAX_NAME_LENGTH ];
    UTF8          spectatorList[ MAX_STRING_CHARS ];  // list of names
    S32           spectatorLen;                       // length of list
    F32         spectatorWidth;                     // width in device units
    S32           spectatorTime;                      // next time to offset
    S32           spectatorPaintX;                    // current paint x
    S32           spectatorPaintX2;                   // current paint x
    S32           spectatorOffset;                    // current offset from start
    S32           spectatorPaintLen;                  // current offset from start
    
    // centerprinting
    S32           centerPrintTime;
    S32           centerPrintCharWidth;
    S32           centerPrintY;
    UTF8          centerPrint[ 1024 ];
    S32           centerPrintLines;
    
    // low ammo warning state
    S32           lowAmmoWarning;   // 1 = low, 2 = empty
    
    // kill timers for carnage reward
    S32           lastKillTime;
    
    // crosshair client ID
    S32           crosshairBuildable;
    S32           crosshairClientNum;
    S32           crosshairClientTime;
    
    // powerup active flashing
    S32           powerupActive;
    S32           powerupTime;
    
    // attacking player
    S32           attackerTime;
    S32           voiceTime;
    
    // reward medals
    S32           rewardStack;
    S32           rewardTime;
    S32           rewardCount[ MAX_REWARDSTACK ];
    qhandle_t     rewardShader[ MAX_REWARDSTACK ];
    qhandle_t     rewardSound[ MAX_REWARDSTACK ];
    
    // sound buffer mainly for announcer sounds
    S32           soundBufferIn;
    S32           soundBufferOut;
    S32           soundTime;
    qhandle_t     soundBuffer[ MAX_SOUNDBUFFER ];
    
    // for voice chat buffer
    S32           voiceChatTime;
    S32           voiceChatBufferIn;
    S32           voiceChatBufferOut;
    
    // warmup countdown
    S32           warmup;
    S32           warmupCount;
    
    //==========================
    
    S32           itemPickup;
    S32           itemPickupTime;
    S32           itemPickupBlendTime;                // the pulse around the crosshair is timed seperately
    
    S32           weaponSelectTime;
    S32           feedbackAnimation;
    S32           feedbackAnimationType;
    
    // blend blobs
    F32         damageTime;
    F32         damageX, damageY, damageValue;
    
    // status bar head
    F32         headYaw;
    F32         headEndPitch;
    F32         headEndYaw;
    S32           headEndTime;
    F32         headStartPitch;
    F32         headStartYaw;
    S32           headStartTime;
    
    // view movement
    F32         v_dmg_time;
    F32         v_dmg_pitch;
    F32         v_dmg_roll;
    
    vec3_t        kick_angles;                        // weapon kicks
    vec3_t        kick_origin;
    
    bool      chaseFollow;
    
    // temp working variables for player view
    F32         bobfracsin;
    S32           bobcycle;
    F32         xyspeed;
    S32           nextOrbitTime;
    
    // development tool
    refEntity_t   testModelEntity;
    refEntity_t   testModelBarrelEntity;
    UTF8          testModelName[MAX_QPATH];
    UTF8          testModelBarrelName[MAX_QPATH];
    bool      testGun;
    
    S32           spawnTime;                          // fovwarp
    S32           weapon1Time;                        // time when BUTTON_ATTACK went t->f f->t
    S32           weapon2Time;                        // time when BUTTON_ATTACK2 went t->f f->t
    S32           weapon3Time;                        // time when BUTTON_USE_HOLDABLE went t->f f->t
    bool      weapon1Firing;
    bool      weapon2Firing;
    bool      weapon3Firing;
    
    S32           poisonedTime;
    
    vec3_t        lastNormal;                         // view smoothage
    vec3_t        lastVangles;                        // view smoothage
    smooth_t      sList[ MAXSMOOTHS ];                // WW smoothing
    
    S32           forwardMoveTime;                    // for struggling
    S32           rightMoveTime;
    S32           upMoveTime;
    
    F32         charModelFraction;                  // loading percentages
    F32         mediaFraction;
    F32         buildablesFraction;
    
    S32           lastBuildAttempt;
    S32           lastEvolveAttempt;
    
    UTF8          consoleText[ MAX_CONSOLE_TEXT ];
    consoleLine_t consoleLines[ MAX_CONSOLE_LINES ];
    S32           numConsoleLines;
    
    particleSystem_t*  poisonCloudPS;
    particleSystem_t*  poisonCloudedPS;
    
    F32         painBlendValue;
    F32         painBlendTarget;
    F32         healBlendValue;
    S32           lastHealth;
    
    S32           lastPredictedCommand;
    S32           lastServerTime;
    playerState_t savedPmoveStates[ NUM_SAVED_STATES ];
    S32           stateHead, stateTail;
    S32           ping;
    
    F32         chargeMeterAlpha;
    F32         chargeMeterValue;
    qhandle_t     lastHealthCross;
    F32         healthCrossFade;
    
    S32           nextWeaponClickTime;
    
    centity_t*     locationHead;
} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct
{
    qhandle_t   charsetShader;
    qhandle_t   whiteShader;
    qhandle_t   outlineShader;
    
    qhandle_t   level2ZapTS;
    
    qhandle_t   balloonShader;
    qhandle_t   connectionShader;
    
    qhandle_t   viewBloodShader;
    qhandle_t   tracerShader;
    qhandle_t   crosshairShader[ WP_NUM_WEAPONS ];
    qhandle_t   backTileShader;
    
    qhandle_t   creepShader;
    
    qhandle_t   scannerShader;
    qhandle_t   scannerBlipShader;
    qhandle_t   scannerLineShader;
    
    
    qhandle_t   numberShaders[ 11 ];
    
    qhandle_t   shadowMarkShader;
    qhandle_t   wakeMarkShader;
    
    // buildable shaders
    qhandle_t   greenBuildShader;
    qhandle_t   redBuildShader;
    qhandle_t   humanSpawningShader;
    
    // disconnect
    qhandle_t   disconnectPS;
    qhandle_t   disconnectSound;
    
    // sounds
    sfxHandle_t tracerSound;
    sfxHandle_t weaponEmptyClick;
    sfxHandle_t selectSound;
    sfxHandle_t footsteps[ FOOTSTEP_TOTAL ][ 4 ];
    sfxHandle_t talkSound;
    sfxHandle_t alienTalkSound;
    sfxHandle_t humanTalkSound;
    sfxHandle_t landSound;
    sfxHandle_t fallSound;
    sfxHandle_t turretSpinupSound;
    
    sfxHandle_t hardBounceSound1;
    sfxHandle_t hardBounceSound2;
    
    sfxHandle_t voteNow;
    sfxHandle_t votePassed;
    sfxHandle_t voteFailed;
    
    sfxHandle_t watrInSound;
    sfxHandle_t watrOutSound;
    sfxHandle_t watrUnSound;
    
    sfxHandle_t jetpackDescendSound;
    sfxHandle_t jetpackIdleSound;
    sfxHandle_t jetpackAscendSound;
    
    qhandle_t   jetPackDescendPS;
    qhandle_t   jetPackHoverPS;
    qhandle_t   jetPackAscendPS;
    
    sfxHandle_t medkitUseSound;
    
    sfxHandle_t alienStageTransition;
    sfxHandle_t humanStageTransition;
    
    sfxHandle_t alienOvermindAttack;
    sfxHandle_t alienOvermindDying;
    sfxHandle_t alienOvermindSpawns;
    
    sfxHandle_t alienBuildableExplosion;
    sfxHandle_t alienBuildableDamage;
    sfxHandle_t alienBuildablePrebuild;
    sfxHandle_t humanBuildableExplosion;
    sfxHandle_t humanBuildablePrebuild;
    sfxHandle_t humanBuildableDamage[ 4 ];
    
    sfxHandle_t alienL1Grab;
    sfxHandle_t alienL4ChargePrepare;
    sfxHandle_t alienL4ChargeStart;
    
    qhandle_t   cursor;
    qhandle_t   selectCursor;
    qhandle_t   sizeCursor;
    
    //light armour
    qhandle_t larmourHeadSkin;
    qhandle_t larmourLegsSkin;
    qhandle_t larmourTorsoSkin;
    
    qhandle_t jetpackModel;
    qhandle_t jetpackFlashModel;
    qhandle_t battpackModel;
    
    sfxHandle_t repeaterUseSound;
    
    sfxHandle_t buildableRepairSound;
    sfxHandle_t buildableRepairedSound;
    
    qhandle_t   poisonCloudPS;
    qhandle_t   poisonCloudedPS;
    qhandle_t   alienEvolvePS;
    qhandle_t   alienAcidTubePS;
    
    sfxHandle_t alienEvolveSound;
    
    qhandle_t   humanBuildableDamagedPS;
    qhandle_t   humanBuildableHitSmallPS;
    qhandle_t   humanBuildableHitLargePS;
    qhandle_t   humanBuildableDestroyedPS;
    qhandle_t   alienBuildableDamagedPS;
    qhandle_t   alienBuildableHitSmallPS;
    qhandle_t   alienBuildableHitLargePS;
    qhandle_t   alienBuildableDestroyedPS;
    
    qhandle_t   alienBleedPS;
    qhandle_t   humanBleedPS;
    
    qhandle_t alienAttackFeedbackShaders[11];
    qhandle_t alienAttackFeedbackShadersFlipped[11];
    qhandle_t alienRangedAttackFeedbackShaders[11];
    
    qhandle_t   teslaZapTS;
    qhandle_t   massDriverTS;
    
    sfxHandle_t lCannonWarningSound;
    sfxHandle_t lCannonWarningSound2;
    
    qhandle_t   buildWeaponTimerPie[ 8 ];
    qhandle_t   upgradeClassIconShader;
    qhandle_t   healthCross;
    qhandle_t   healthCross2X;
    qhandle_t   healthCross3X;
    qhandle_t   healthCrossMedkit;
    qhandle_t   healthCrossPoisoned;
    
    qhandle_t   squadMarkerH;
    qhandle_t   squadMarkerV;
} cgMedia_t;

typedef struct
{
    qhandle_t     frameShader;
    qhandle_t     overlayShader;
    qhandle_t     noPowerShader;
    qhandle_t     markedShader;
    vec4_t        healthSevereColor;
    vec4_t        healthHighColor;
    vec4_t        healthElevatedColor;
    vec4_t        healthGuardedColor;
    vec4_t        healthLowColor;
    S32           frameHeight;
    S32           frameWidth;
    S32           healthPadding;
    S32           overlayHeight;
    S32           overlayWidth;
    F32         verticalMargin;
    F32         horizontalMargin;
    vec4_t        foreColor;
    vec4_t        backColor;
    bool      loaded;
} buildStat_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct
{
    gameState_t   gameState;              // gamestate from server
    glconfig_t    glconfig;               // rendering configuration
    F32         screenXScale;           // derived from glconfig
    F32         screenYScale;
    F32         screenXBias;
    
    S32           serverCommandSequence;  // reliable command stream counter
    S32           processedSnapshotNum;   // the number of snapshots cgame has requested
    
    bool      localServer;            // detected on startup by checking sv_running
    
    // parsed from serverinfo
    S32           timelimit;
    S32           maxclients;
    UTF8          mapname[ MAX_QPATH ];
    bool      markDeconstruct;        // Whether or not buildables are marked
    
    S32           voteTime;
    S32           voteYes;
    S32           voteNo;
    bool      voteModified;           // beep whenever changed
    UTF8          voteString[ MAX_STRING_TOKENS ];
    
    S32           teamVoteTime[ 2 ];
    S32           teamVoteYes[ 2 ];
    S32           teamVoteNo[ 2 ];
    bool      teamVoteModified[ 2 ];  // beep whenever changed
    UTF8          teamVoteString[ 2 ][ MAX_STRING_TOKENS ];
    
    S32           levelStartTime;
    
    S32           scores1, scores2;   // from configstrings
    
    bool      newHud;
    
    S32           alienBuildPoints;
    S32           alienBuildPointsTotal;
    S32           humanBuildPoints;
    S32           humanBuildPointsTotal;
    S32           humanBuildPointsPowered;
    
    S32           alienStage;
    S32           humanStage;
    S32           alienCredits;
    S32           humanCredits;
    S32           alienNextStageThreshold;
    S32           humanNextStageThreshold;
    
    //
    // locally derived information from gamestate
    //
    qhandle_t     gameModels[ MAX_MODELS ];
    qhandle_t     gameShaders[ MAX_GAME_SHADERS ];
    qhandle_t     gameParticleSystems[ MAX_GAME_PARTICLE_SYSTEMS ];
    sfxHandle_t   gameSounds[ MAX_SOUNDS ];
    
    S32           numInlineModels;
    qhandle_t     inlineDrawModel[ MAX_MODELS ];
    vec3_t        inlineModelMidpoints[ MAX_MODELS ];
    
    clientInfo_t  clientinfo[ MAX_CLIENTS ];
    
    // corpse info
    clientInfo_t  corpseinfo[ MAX_CLIENTS ];
    
    S32           cursorX;
    S32           cursorY;
    bool      eventHandling;
    bool      mouseCaptured;
    bool      sizingHud;
    void*          capturedItem;
    qhandle_t     activeCursor;
    
    buildStat_t   alienBuildStat;
    buildStat_t   humanBuildStat;
    
    // media
    cgMedia_t           media;
    
    voice_t*       voices;
    clientList_t  ignoreList;
} cgs_t;

typedef struct
{
    UTF8* cmd;
    void ( *function )( void );
    void( *completer )( void );
} consoleCommand_t;

//==============================================================================

extern  cgs_t     cgs;
extern  cg_t      cg;
extern  centity_t cg_entities[ MAX_GENTITIES ];
extern  displayContextDef_t  cgDC;

extern  weaponInfo_t    cg_weapons[ 32 ];
extern  upgradeInfo_t   cg_upgrades[ 32 ];

extern  buildableInfo_t cg_buildables[ BA_NUM_BUILDABLES ];

extern  markPoly_t      cg_markPolys[ MAX_MARK_POLYS ];

extern  vmCvar_t    cg_version;
extern  vmCvar_t    cg_teslaTrailTime;
extern  vmCvar_t    cg_centertime;
extern  vmCvar_t    cg_runpitch;
extern  vmCvar_t    cg_runroll;
extern  vmCvar_t    cg_swingSpeed;
extern  vmCvar_t    cg_shadows;
extern  vmCvar_t    cg_drawTimer;
extern  vmCvar_t    cg_drawClock;
extern  vmCvar_t    cg_drawFPS;
extern  vmCvar_t    cg_drawSpeed;
extern  vmCvar_t    cg_drawDemoState;
extern  vmCvar_t    cg_drawSnapshot;
extern  vmCvar_t    cg_drawChargeBar;
extern  vmCvar_t    cg_drawCrosshair;
extern  vmCvar_t    cg_drawCrosshairNames;
extern  vmCvar_t    cg_crosshairSize;
extern  vmCvar_t    cg_drawAmmoStack;
extern  vmCvar_t    cg_draw2D;
extern  vmCvar_t    cg_drawStatus;
extern  vmCvar_t    cg_animSpeed;
extern  vmCvar_t    cg_debugAnim;
extern  vmCvar_t    cg_debugAlloc;
extern  vmCvar_t    cg_debugPosition;
extern  vmCvar_t    cg_debugEvents;
extern  vmCvar_t    cg_errorDecay;
extern  vmCvar_t    cg_nopredict;
extern  vmCvar_t    cg_debugMove;
extern  vmCvar_t    cg_noPlayerAnims;
extern  vmCvar_t    cg_showmiss;
extern  vmCvar_t    cg_footsteps;
extern  vmCvar_t    cg_addMarks;
extern  vmCvar_t    cg_brassTime;
extern  vmCvar_t    cg_viewsize;
extern  vmCvar_t    cg_drawGun;
extern  vmCvar_t    cg_gun_frame;
extern  vmCvar_t    cg_gun_x;
extern  vmCvar_t    cg_gun_y;
extern  vmCvar_t    cg_gun_z;
extern  vmCvar_t    cg_tracerChance;
extern  vmCvar_t    cg_tracerWidth;
extern  vmCvar_t    cg_tracerLength;
extern  vmCvar_t    cg_autoswitch;
extern  vmCvar_t    cg_thirdPerson;
extern  vmCvar_t    cg_thirdPersonShoulderViewMode;
extern  vmCvar_t    cg_thirdPersonPitchFollow;
extern  vmCvar_t    cg_thirdPersonRange;
extern  vmCvar_t    cg_shoulderViewOverride;
extern  vmCvar_t    cg_shoulderViewUp;
extern  vmCvar_t    cg_shoulderViewRight;
extern  vmCvar_t    cg_shoulderViewForward;
extern  vmCvar_t    cg_stereoSeparation;
extern  vmCvar_t    cg_lagometer;
extern  vmCvar_t    cg_synchronousClients;
extern  vmCvar_t    cg_stats;
extern  vmCvar_t    cg_paused;
extern  vmCvar_t    cg_blood;
extern  vmCvar_t    cg_drawFriend;
extern  vmCvar_t    cg_teamChatsOnly;
extern  vmCvar_t    cg_noVoiceChats;
extern  vmCvar_t    cg_noVoiceText;
extern  vmCvar_t    cg_hudFiles;
extern  vmCvar_t    cg_smoothClients;
extern  vmCvar_t    pmove_fixed;
extern  vmCvar_t    pmove_msec;
extern  vmCvar_t    cg_cameraMode;
extern  vmCvar_t    cg_timescaleFadeEnd;
extern  vmCvar_t    cg_timescaleFadeSpeed;
extern  vmCvar_t    cg_timescale;
extern  vmCvar_t    cg_noTaunt;
extern  vmCvar_t    cg_drawSurfNormal;
extern  vmCvar_t    cg_drawBBOX;
extern  vmCvar_t    cg_wwSmoothTime;
extern  vmCvar_t    cg_flySpeed;
extern  vmCvar_t    cg_depthSortParticles;
extern  vmCvar_t    cg_bounceParticles;
extern  vmCvar_t    cg_consoleLatency;
extern  vmCvar_t    cg_lightFlare;
extern  vmCvar_t    cg_debugParticles;
extern  vmCvar_t    cg_debugTrails;
extern  vmCvar_t    cg_debugPVS;
extern  vmCvar_t    cg_disableWarningDialogs;
extern  vmCvar_t    cg_disableUpgradeDialogs;
extern  vmCvar_t    cg_disableBuildDialogs;
extern  vmCvar_t    cg_disableCommandDialogs;
extern  vmCvar_t    cg_disableScannerPlane;
extern  vmCvar_t    cg_tutorial;

extern  vmCvar_t    cg_painBlendUpRate;
extern  vmCvar_t    cg_painBlendDownRate;
extern  vmCvar_t    cg_painBlendMax;
extern  vmCvar_t    cg_painBlendScale;
extern  vmCvar_t    cg_painBlendZoom;

extern  vmCvar_t    cg_stickySpec;
extern  vmCvar_t    cg_alwaysSprint;
extern  vmCvar_t    cg_unlagged;

extern  vmCvar_t    cg_debugVoices;

extern  vmCvar_t    ui_currentClass;
extern  vmCvar_t    ui_carriage;
extern  vmCvar_t    ui_stage;
extern  vmCvar_t    ui_dialog;
extern  vmCvar_t    ui_voteActive;
extern  vmCvar_t    ui_alienTeamVoteActive;
extern  vmCvar_t    ui_humanTeamVoteActive;

extern  vmCvar_t    cg_debugRandom;

extern  vmCvar_t    cg_optimizePrediction;
extern  vmCvar_t    cg_projectileNudge;

extern  vmCvar_t    cg_drawBuildableStatus;
extern  vmCvar_t    cg_hideHealthyBuildableStatus;
extern  vmCvar_t    cg_drawTeamStatus;
extern  vmCvar_t    cg_hideHealthyTeamStatus;

extern  vmCvar_t    cg_drawAlienFeedback;

extern  vmCvar_t    cg_voice;

extern  vmCvar_t    cg_suppressWAnimWarnings;

extern  vmCvar_t    cg_emoticons;

#define MAGIC_TRACE_HACK -2

// cg_drawCrosshair settings
#define CROSSHAIR_ALWAYSOFF       0
#define CROSSHAIR_RANGEDONLY      1
#define CROSSHAIR_ALWAYSON        2

// menu types for cg_disable*Dialogs
typedef enum
{
    DT_INTERACTIVE, // team, class, armoury
    DT_ARMOURYEVOLVE, // Insufficient funds et al
    DT_BUILD, // build errors
    DT_COMMAND, // You must be living/human/spec etc.
    
} dialogType_t;

//
// idCGameLocal
//
class idCGameLocal : public idCGame
{
public:
    idCGameLocal();
    ~idCGameLocal();
    
    virtual void Init( S32 serverMessageNum, S32 serverCommandSequence, S32 clientNum, bool demoPlayback );
    // called when the level loads or when the renderer is restarted
    // all media should be registered at this time
    // cgame will display loading status by calling SCR_Update, which
    // will call CG_DrawInformation during the loading process
    // reliableCommandSequence will be 0 on fresh loads, but higher for
    // demos, tourney restarts, or vid_restarts
    
    virtual void Shutdown();
    // oportunity to flush and close any open files
    
    virtual bool ConsoleCommand();
    // a console command has been issued locally that is not recognized by the
    // main game system.
    // use Cmd_Argc() / Cmd_Argv() to read the command, return false if the
    // command is not known to the game
    
    virtual void DrawActiveFrame( S32 serverTime, stereoFrame_t stereoView, bool demoPlayback );
    // Generates and draws a game scene and status information at the given time.
    // If demoPlayback is set, local movement prediction will not be enabled
    
    virtual void ConsoleText( void );
    //	pass text that has been printed to the console to cgame
    //	use Cmd_Argc() / Cmd_Argv() to read it
    
    virtual S32 CrosshairPlayer( void );
    
    virtual S32 LastAttacker( void );
    
    virtual void KeyEvent( S32 key, bool down );
    
    virtual void MouseEvent( S32 dx, S32 dy );
    
    virtual void EventHandling( S32 type, bool fForced );
    
    virtual bool GetTag( S32 clientNum, UTF8* tagname, orientation_t* or );
    
    virtual bool CheckExecKey( S32 key );
    
    virtual bool WantsBindKeys( void );
    
    virtual void CompleteCommand( S32 argNum );
    // will callback on all availible completions
    // use Cmd_Argc() / Cmd_Argv() to read the command
public:
    static void RegisterUpgrade( S32 upgradeNum );
    static void InitUpgrades( void );
    static bool ParseWeaponAnimationFile( StringEntry filename, weaponInfo_t* weapon );
    static bool ParseWeaponModeSection( weaponInfoMode_t* wim, UTF8** text_p );
    static bool ParseWeaponFile( StringEntry filename, weaponInfo_t* wi );
    static void RegisterWeapon( S32 weaponNum );
    void InitWeapons( void );
    static void SetWeaponLerpFrameAnimation( weapon_t weapon, lerpFrame_t* lf, S32 newAnimation );
    static void WeaponAnimation( centity_t* cent, S32* old, S32* now, F32* backLerp );
    static S32 MapTorsoToWeaponFrame( clientInfo_t* ci, S32 frame );
    static void CalculateWeaponPosition( vec3_t origin, vec3_t angles );
    static F32 MachinegunSpinAngle( centity_t* cent, bool firing );
    static void AddPlayerWeapon( refEntity_t* parent, playerState_t* ps, centity_t* cent );
    static void AddViewWeapon( playerState_t* ps );
    static bool WeaponSelectable( weapon_t weapon );
    static bool UpgradeSelectable( upgrade_t upgrade );
    static void DrawItemSelect( rectDef_t* rect, vec4_t color );
    static void DrawItemSelectText( rectDef_t* rect, F32 scale, S32 textStyle );
    static void NextWeapon_f( void );
    static void PrevWeapon_f( void );
    static void Weapon_f( void );
    static void FireWeapon( centity_t* cent, weaponMode_t weaponMode );
    static void HandleAlienFeedback( centity_t* cent, alienFeedback_t feedbackType );
    static void MissileHitWall( weapon_t weaponNum, weaponMode_t weaponMode, S32 clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType, S32 charge );
    static void MissileHitPlayer( weapon_t weaponNum, weaponMode_t weaponMode, vec3_t origin, vec3_t dir, S32 entityNum, S32 charge );
    static void MassDriverFire( entityState_t* es );
    static void Tracer( vec3_t source, vec3_t dest );
    static bool CalcMuzzlePoint( S32 entityNum, vec3_t muzzle );
    static void Bullet( vec3_t end, S32 sourceEntityNum, vec3_t normal, bool flesh, S32 fleshEntityNum );
    static void ShotgunPattern( vec3_t origin, vec3_t origin2, S32 seed, S32 otherEntNum );
    static void ShotgunFire( entityState_t* es );
    static void TestModel_f( void );
    static void TestGun_f( void );
    static void TestModelNextFrame_f( void );
    static void TestModelPrevFrame_f( void );
    static void TestModelNextSkin_f( void );
    static void TestModelPrevSkin_f( void );
    static void AddTestModel( void );
    static void CalcVrect( void );
    static void OffsetThirdPersonView( void );
    static void OffsetShoulderView( void );
    static void OffsetFirstPersonView( void );
    static void StepOffset( void );
    static S32 CalcFov( void );
    static void DrawSurfNormal( void );
    static void addSmoothOp( vec3_t rotAxis, F32 rotAngle, F32 timeMod );
    static void smoothWWTransitions( playerState_t* ps, const vec3_t in, vec3_t out );
    static void smoothWJTransitions( playerState_t* ps, const vec3_t in, vec3_t out );
    static S32 CalcViewValues( void );
    static void AddBufferedSound( sfxHandle_t sfx );
    static void PlayBufferedSounds( void );
    static void GetBindings( void );
    static void FixBindings( void );
    static StringEntry KeyNameForCommand( StringEntry command );
    static entityState_t* BuildableInRange( playerState_t* ps, F32* healthFraction );
    static void AlienBuilderText( UTF8* text, playerState_t* ps );
    static void AlienLevel0Text( UTF8* text, playerState_t* ps );
    static void AlienLevel1Text( UTF8* text, playerState_t* ps );
    static void AlienLevel2Text( UTF8* text, playerState_t* ps );
    static void AlienLevel3Text( UTF8* text, playerState_t* ps );
    static void AlienLevel4Text( UTF8* text, playerState_t* ps );
    static void HumanCkitText( UTF8* text, playerState_t* ps );
    static void HumanText( UTF8* text, playerState_t* ps );
    static void SpectatorText( UTF8* text, playerState_t* ps );
    static StringEntry TutorialText( void );
    static void CalculateBeamNodeProperties( trailBeam_t* tb );
    static void LightVertex( vec3_t point, U8 alpha, U8* rgba );
    static void RenderBeam( trailBeam_t* tb );
    static trailBeamNode_t* AllocateBeamNode( trailBeam_t* tb );
    static trailBeamNode_t* DestroyBeamNode( trailBeamNode_t* tbn );
    static trailBeamNode_t* FindLastBeamNode( trailBeam_t* tb );
    static S32 CountBeamNodes( trailBeam_t* tb );
    static trailBeamNode_t* PrependBeamNode( trailBeam_t* tb );
    static trailBeamNode_t* AppendBeamNode( trailBeam_t* tb );
    static void ApplyJitters( trailBeam_t* tb );
    static void UpdateBeam( trailBeam_t* tb );
    static bool ParseTrailBeamColor( U8* c, UTF8** text_p );
    static bool ParseTrailBeam( baseTrailBeam_t* btb, UTF8** text_p );
    static void InitialiseBaseTrailBeam( baseTrailBeam_t* btb );
    static bool ParseTrailSystem( baseTrailSystem_t* bts, UTF8** text_p, StringEntry name );
    static bool ParseTrailFile( StringEntry fileName );
    void LoadTrailSystems( void );
    static qhandle_t RegisterTrailSystem( UTF8* name );
    static trailBeam_t* SpawnNewTrailBeam( baseTrailBeam_t* btb, trailSystem_t* parent );
    static trailSystem_t* SpawnNewTrailSystem( qhandle_t psHandle );
    static void DestroyTrailSystem( trailSystem_t** ts );
    static bool IsTrailSystemValid( trailSystem_t** ts );
    static void GarbageCollectTrailSystems( void );
    void AddTrails( void );
    static void DestroyTestTS_f( void );
    static void TestTS_f( void );
    static void ResetEntity( centity_t* cent );
    static void TransitionEntity( centity_t* cent );
    void SetInitialSnapshot( snapshot_t* snap );
    static void TransitionSnapshot( void );
    static void SetNextSnap( snapshot_t* snap );
    static snapshot_t* ReadNextSnapshot( void );
    void ProcessSnapshots( void );
    static void ParseScores( void );
    static void ParseTeamInfo( void );
    static void ParseServerinfo( void );
    static void ParseWarmup( void );
    void SetConfigValues( void );
    static void ShaderStateChanged( void );
    static void AnnounceAlienStageTransistion( stage_t from, stage_t to );
    static void AnnounceHumanStageTransistion( stage_t from, stage_t to );
    static void ConfigStringModified( void );
    static void MapRestart( void );
    static void Menu( S32 menu, S32 arg );
    static void Say( S32 clientNum, UTF8* text );
    static void SayTeam( S32 clientNum, UTF8* text );
    static voiceTrack_t* VoiceTrack( UTF8* voice, S32 cmd, S32 track );
    static void ParseVoice( void );
    static void CenterPrint_f( void );
    static void Print_f( void );
    static void Chat_f( void );
    static void ClientLevelShot_f( void );
    static void ServerMenu_f( void );
    static void ServerCloseMenus_f( void );
    static void PoisonCloud_f( void );
    static void PTRRequest_f( void );
    static void PTRIssue_f( void );
    static void PTRConfirm_f( void );
    static void ServerCommand( void );
    static void ExecuteNewServerCommands( S32 latestSequence );
    void UpdateEntityPositions( void );
    static void DrawBlips( rectDef_t* rect, vec3_t origin, vec4_t colour );
    static void DrawDir( rectDef_t* rect, vec3_t origin, vec4_t colour );
    static void AlienSense( rectDef_t* rect );
    static void Scanner( rectDef_t* rect, qhandle_t shader, vec4_t color );
    static S32 ReadPTRCode( void );
    static void WritePTRCode( S32 code );
    static void BuildSolidList( void );
    static void ClipMoveToEntities( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 skipNumber, S32 mask, trace_t* tr, traceType_t collisionType );
    static void Trace( trace_t* result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 skipNumber, S32 mask );
    static void CapTrace( trace_t* result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 skipNumber, S32 mask );
    static void BiSphereTrace( trace_t* result, const vec3_t start, const vec3_t end, const F32 startRadius, const F32 endRadius, S32 skipNumber, S32 mask );
    static S32 PointContents( const vec3_t point, S32 passEntityNum );
    static void InterpolatePlayerState( bool grabAngles );
    static void TouchTriggerPrediction( void );
    static S32 IsUnacceptableError( playerState_t* ps, playerState_t* pps );
    void PredictPlayerState( void );
    static void DamageFeedback( S32 yawByte, S32 pitchByte, S32 damage );
    static void Respawn( void );
    static void CheckPlayerstateEvents( playerState_t* ps, playerState_t* ops );
    void CheckChangedPredictableEvents( playerState_t* ps );
    static void CheckLocalSounds( playerState_t* ps, playerState_t* ops );
    static void TransitionPlayerState( playerState_t* ps, playerState_t* ops );
    static sfxHandle_t CustomSound( S32 clientNum, StringEntry soundName );
    static bool ParseAnimationFile( StringEntry filename, clientInfo_t* ci );
    static bool RegisterClientSkin( clientInfo_t* ci, StringEntry modelName, StringEntry skinName );
    static bool RegisterClientModelname( clientInfo_t* ci, StringEntry modelName, StringEntry skinName );
    static void ColorFromString( StringEntry v, vec3_t color );
    static void LoadClientInfo( clientInfo_t* ci );
    static void CopyClientInfoModel( clientInfo_t* from, clientInfo_t* to );
    static S32 GetCorpseNum( class_t _class );
    static bool ScanForExistingClientInfo( clientInfo_t* ci );
    static void PrecacheClientInfo( class_t _class, UTF8* model, UTF8* skin );
    static void TeamJoinMessage( clientInfo_t* newInfo, clientInfo_t* ci );
    static void NewClientInfo( S32 clientNum );
    static void SetLerpFrameAnimation( clientInfo_t* ci, lerpFrame_t* lf, S32 newAnimation );
    static void RunPlayerLerpFrame( clientInfo_t* ci, lerpFrame_t* lf, S32 newAnimation, F32 speedScale );
    static void ClearLerpFrame( clientInfo_t* ci, lerpFrame_t* lf, S32 animationNumber );
    static void PlayerAnimation( centity_t* cent, S32* legsOld, S32* legs, F32* legsBackLerp, S32* torsoOld, S32* torso, F32* torsoBackLerp );
    static void PlayerNonSegAnimation( centity_t* cent, S32* nonSegOld, S32* nonSeg, F32* nonSegBackLerp );
    static void SwingAngles( F32 destination, F32 swingTolerance, F32 clampTolerance, F32 speed, F32* angle, bool* swinging );
    static void AddPainTwitch( centity_t* cent, vec3_t torsoAngles );
    static void PlayerAngles( centity_t* cent, vec3_t srcAngles, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] );
    static void PlayerWWSmoothing( centity_t* cent, vec3_t in[3], vec3_t out[3] );
    static void PlayerNonSegAngles( centity_t* cent, vec3_t srcAngles, vec3_t nonSegAxis[3] );
    static void PlayerUpgrades( centity_t* cent, refEntity_t* torso );
    static void PlayerFloatSprite( centity_t* cent, qhandle_t shader );
    static void PlayerSprites( centity_t* cent );
    static bool PlayerShadow( centity_t* cent, F32* shadowPlane, class_t _class );
    static void PlayerSplash( centity_t* cent, class_t _class );
    S32 LightVerts( vec3_t normal, S32 numVerts, polyVert_t* verts );
    S32 LightFromDirection( vec3_t point, vec3_t direction );
    S32 AmbientLight( vec3_t point );
    static void Player( centity_t* cent );
    static void Corpse( centity_t* cent );
    static void ResetPlayerEntity( centity_t* cent );
    static void PlayerDisconnect( vec3_t org );
    static void Bleed( vec3_t origin, vec3_t normal, S32 entityNum );
    static void TeamStatusDisplay( centity_t* cent );
    static S32 SortDistance2( const void* a, const void* b );
    static void DrawTeamStatus( void );
    static centity_t* GetPlayerLocation( void );
    static F32 LerpValues( F32 a, F32 b, F32 f );
    static F32 RandomiseValue( F32 value, F32 variance );
    static void SpreadVector( vec3_t v, F32 spread );
    static void DestroyParticle( particle_t* p, vec3_t impactNormal );
    static particle_t* SpawnNewParticle( baseParticle_t* bp, particleEjector_t* parent );
    static void SpawnNewParticles( void );
    static particleEjector_t* SpawnNewParticleEjector( baseParticleEjector_t* bpe, particleSystem_t* parent );
    static particleSystem_t* SpawnNewParticleSystem( qhandle_t psHandle );
    static qhandle_t RegisterParticleSystem( UTF8* name );
    static void ParseValueAndVariance( UTF8* token, F32* value, F32* variance, bool allowNegative );
    static bool ParseColor( U8* c, UTF8** text_p );
    static bool ParseParticle( baseParticle_t* bp, UTF8** text_p );
    static void InitialiseBaseParticle( baseParticle_t* bp );
    static bool ParseParticleEjector( baseParticleEjector_t* bpe, UTF8** text_p );
    static bool ParseParticleSystem( baseParticleSystem_t* bps, UTF8** text_p, StringEntry name );
    static bool ParseParticleFile( StringEntry fileName );
    void LoadParticleSystems( void );
    static void SetParticleSystemNormal( particleSystem_t* ps, vec3_t normal );
    static void DestroyParticleSystem( particleSystem_t** ps );
    static bool IsParticleSystemInfinite( particleSystem_t* ps );
    static bool IsParticleSystemValid( particleSystem_t** ps );
    static void GarbageCollectParticleSystems( void );
    static F32 CalculateTimeFrac( S32 birth, S32 life, S32 delay );
    static void EvaluateParticlePhysics( particle_t* p );
    static void Radix( S32 bits, S32 size, particle_t** source, particle_t** dest );
    static void RadixSort( particle_t** source, particle_t** temp, S32 size );
    static void CompactAndSortParticles( void );
    static void RenderParticle( particle_t* p );
    void AddParticles( void );
    static void ParticleSystemEntity( centity_t* cent );
    static void DestroyTestPS_f( void );
    static void TestPS_f( void );
    void* Alloc( S32 size );
    void Free( void* ptr );
    void InitMemory( void );
    void DefragmentMemory( void );
    static void InitMarkPolys( void );
    static void FreeMarkPoly( markPoly_t* le );
    static markPoly_t* AllocMark( void );
    static void ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir, F32 orientation, F32 red, F32 green, F32 blue, F32 alpha, bool alphaFade, F32 radius, bool temporary );
    void AddMarks( void );
    void RegisterCvars( void );
    static void SetUIVars( void );
    static void SetPVars( void );
    void UpdateCvars( void );
    void RemoveNotifyLine( void );
    void AddNotifyText( void );
    static void Printf( StringEntry msg, ... );
    static void Error( StringEntry msg, ... );
    static StringEntry Argv( S32 arg );
    static bool FileExists( UTF8* filename );
    static void RegisterSounds( void );
    static void RegisterGraphics( void );
    static void BuildSpectatorString( void );
    static void RegisterClients( void );
    static StringEntry ConfigString( S32 index );
    static void StartMusic( void );
    S32 PlayerCount( void );
    UTF8* GetMenuBuffer( StringEntry filename );
    static bool Asset_Parse( S32 handle );
    static void ParseMenu( StringEntry menuFile );
    static bool Load_Menu( UTF8** p );
    static void LoadMenus( StringEntry menuFile );
    static bool OwnerDrawHandleKey( S32 ownerDraw, S32 flags, F32* special, S32 key );
    static S32 FeederCount( F32 feederID );
    static void SetScoreSelection( void* p );
    static clientInfo_t* InfoFromScoreIndex( S32 index, S32 team, S32* scoreIndex );
    static bool ClientIsReady( S32 clientNum );
    static StringEntry FeederItemText( F32 feederID, S32 index, S32 column, qhandle_t* handle );
    static qhandle_t FeederItemImage( F32 feederID, S32 index );
    static void FeederSelection( F32 feederID, S32 index );
    static F32 Cvar_Get( StringEntry cvar );
    void Text_PaintWithCursor( F32 x, F32 y, F32 scale, vec4_t color, StringEntry text, S32 cursorPos, UTF8 cursor, S32 limit, S32 style );
    static S32 OwnerDrawWidth( S32 ownerDraw, F32 scale );
    static S32 PlayCinematic( StringEntry name, F32 x, F32 y, F32 w, F32 h );
    static void StopCinematic( S32 handle );
    static void DrawCinematic( S32 handle, F32 x, F32 y, F32 w, F32 h );
    static void RunCinematicFrame( S32 handle );
    static bool OwnerDrawVisible( S32 parameter );
    static void LoadHudMenu( void );
    void AssetCache( void );
    static void Obituary( entityState_t* ent );
    static void PainEvent( centity_t* cent, S32 health );
    static void Level2Zap( entityState_t* es );
    static void EntityEvent( centity_t* cent, vec3_t position );
    static void CheckEvents( centity_t* cent );
    static void DrawBoxFace( vec3_t a, vec3_t b, vec3_t c, vec3_t d );
    static void DrawBoundingBox( vec3_t origin, vec3_t mins, vec3_t maxs );
    static void PositionEntityOnTag( refEntity_t* entity, const refEntity_t* parent, qhandle_t parentModel, UTF8* tagName );
    static void PositionRotatedEntityOnTag( refEntity_t* entity, const refEntity_t* parent, qhandle_t parentModel, UTF8* tagName );
    static void SetEntitySoundPosition( centity_t* cent );
    static void EntityEffects( centity_t* cent );
    static void General( centity_t* cent );
    static void Speaker( centity_t* cent );
    static void LaunchMissile( centity_t* cent );
    static void Missile( centity_t* cent );
    static void Mover( centity_t* cent );
    static void Beam( centity_t* cent );
    static void Portal( centity_t* cent );
    static void LightFlare( centity_t* cent );
    static void AdjustPositionForMover( const vec3_t in, S32 moverNum, S32 fromTime, S32 toTime, vec3_t out );
    static void InterpolateEntityPosition( centity_t* cent );
    static void CalcEntityLerpPositions( centity_t* cent );
    static void CEntityPVSEnter( centity_t* cent );
    static void CEntityPVSLeave( centity_t* cent );
    static void AddCEntity( centity_t* cent );
    void AddPacketEntities( void );
    static void LinkLocation( centity_t* cent );
    void DrawPlane( vec3_t origin, vec3_t down, vec3_t right, qhandle_t shader );
    static void AdjustFrom640( F32* x, F32* y, F32* w, F32* h );
    static void DrawSides( F32 x, F32 y, F32 w, F32 h, F32 size );
    static void DrawTopBottom( F32 x, F32 y, F32 w, F32 h, F32 size );
    static void DrawCorners( F32 x, F32 y, F32 w, F32 h, F32 size, qhandle_t pic );
    static void DrawRect( F32 x, F32 y, F32 width, F32 height, F32 size, const F32* color );
    static void DrawRoundedRect( F32 x, F32 y, F32 width, F32 height, F32 size, const F32* color );
    static void FillRect( F32 x, F32 y, F32 width, F32 height, const F32* color );
    static void FillRoundedRect( F32 x, F32 y, F32 width, F32 height, F32 size, const F32* color );
    static void DrawPic( F32 x, F32 y, F32 width, F32 height, qhandle_t hShader );
    void DrawFadePic( F32 x, F32 y, F32 width, F32 height, vec4_t fcolor, vec4_t tcolor, F32 amount, qhandle_t hShader );
    static S32 DrawStrlen( StringEntry str );
    static void TileClearBox( S32 x, S32 y, S32 w, S32 h, qhandle_t hShader );
    void TileClear( void );
    static F32* FadeColor( S32 startMsec, S32 totalMsec );
    static bool WorldToScreen( vec3_t poS32, F32* x, F32* y );
    static bool WorldToScreenWrap( vec3_t poS32, F32* x, F32* y );
    static UTF8* KeyBinding( StringEntry bind );
    static void AlignText( rectDef_t* rect, StringEntry text, F32 scale, F32 w, F32 h, S32 align, S32 valign, F32* x, F32* y );
    static void DrawFieldPadded( S32 x, S32 y, S32 width, S32 cw, S32 ch, S32 value );
    static void DrawField( F32 x, F32 y, S32 width, F32 cw, F32 ch, S32 value );
    static void DrawProgressBar( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special, F32 progress );
    static void DrawPlayerCreditsValue( rectDef_t* rect, vec4_t color, bool padding );
    static void DrawAttackFeedback( rectDef_t* rect );
    static void DrawPlayerStamina( S32 ownerDraw, rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader );
    static void DrawPlayerStaminaBolt( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader );
    static void DrawPlayerClipsRing( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader );
    static void DrawPlayerBuildTimerRing( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader );
    static void DrawPlayerBoosted( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader );
    static void DrawPlayerBoosterBolt( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader );
    static void DrawPlayerPoisonBarbs( rectDef_t* rect, vec4_t color, qhandle_t shader );
    static void DrawPlayerWallclimbing( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, qhandle_t shader );
    static void DrawPlayerAmmoValue( rectDef_t* rect, vec4_t color );
    static void DrawStack( rectDef_t* rect, vec4_t color, F32 fill, S32 align, S32 valign, F32 val, S32 max );
    static void DrawPlayerAmmoStack( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, S32 textalign, S32 textvalign );
    static void DrawPlayerClipsStack( rectDef_t* rect, vec4_t backColor, vec4_t foreColor, S32 textalign, S32 textvalign );
    static void DrawAlienSense( rectDef_t* rect );
    static void DrawHumanScanner( rectDef_t* rect, qhandle_t shader, vec4_t color );
    static void DrawUsableBuildable( rectDef_t* rect, qhandle_t shader, vec4_t color );
    static void DrawPlayerBuildTimer( rectDef_t* rect, vec4_t color );
    static void DrawPlayerClipsValue( rectDef_t* rect, vec4_t color );
    static void DrawPlayerHealthValue( rectDef_t* rect, vec4_t color );
    static void DrawPlayerHealthCross( rectDef_t* rect, vec4_t ref_color );
    static F32 ChargeProgress( void );
    static void DrawPlayerChargeBarBG( rectDef_t* rect, vec4_t ref_color, qhandle_t shader );
    static void DrawPlayerChargeBar( rectDef_t* rect, vec4_t ref_color, qhandle_t shader );
    static void DrawProgressLabel( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, StringEntry s, F32 fraction );
    static void DrawMediaProgress( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special );
    static void DrawMediaProgressLabel( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign );
    static void DrawBuildablesProgress( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special );
    static void DrawBuildablesProgressLabel( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign );
    static void DrawCharModelProgress( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special );
    static void DrawCharModelProgressLabel( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign );
    static void DrawOverallProgress( rectDef_t* rect, vec4_t color, F32 scale, S32 align, S32 textalign, S32 textStyle, S32 special );
    static void DrawLevelShot( rectDef_t* rect );
    static void DrawLevelName( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawMOTD( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawHostname( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawDemoPlayback( rectDef_t* rect, vec4_t color, qhandle_t shader );
    static void DrawDemoRecording( rectDef_t* rect, vec4_t color, qhandle_t shader );
    static void UpdateMediaFraction( F32 newFract );
    void DrawLoadingScreen( void );
    static F32 GetValue( S32 ownerDraw );
    static StringEntry GetKillerText( void );
    static void DrawKiller( rectDef_t* rect, F32 scale, vec4_t color, qhandle_t shader, S32 textStyle );
    static void DrawTeamSpectators( rectDef_t* rect, F32 scale, S32 textvalign, vec4_t color, qhandle_t shader );
    static void DrawTeamLabel( rectDef_t* rect, team_t team, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawStageReport( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawFPS( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle, bool scalableText );
    static void DrawSpeed( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawTimerMins( rectDef_t* rect, vec4_t color );
    static void DrawTimerSecs( rectDef_t* rect, vec4_t color );
    static void DrawTimer( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawClock( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawSnapshot( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t color, S32 textalign, S32 textvalign, S32 textStyle );
    static void AddLagometerFrameInfo( void );
    static void AddLagometerSnapshotInfo( snapshot_t* snap );
    static void DrawDisconnect( void );
    static void DrawLagometer( rectDef_t* rect, F32 text_x, F32 text_y, F32 scale, vec4_t textColor );
    static void DrawConsole( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawTutorial( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color, F32 scale, S32 textalign, S32 textvalign, S32 textStyle );
    static void DrawWeaponIcon( rectDef_t* rect, vec4_t color );
    static void DrawCrosshair( rectDef_t* rect, vec4_t color );
    static void ScanForCrosshairEntity( void );
    static void DrawLocation( rectDef_t* rect, F32 scale, S32 textalign, vec4_t color );
    static void DrawCrosshairNames( rectDef_t* rect, F32 scale, S32 textStyle );
    static void DrawSquadMarkers( vec4_t color );
    static void OwnerDraw( F32 x, F32 y, F32 w, F32 h, F32 text_x, F32 text_y, S32 ownerDraw, S32 ownerDrawFlags, S32 align, S32 textalign, S32 textvalign, F32 special, F32 scale, vec4_t foreColor, vec4_t backColor, qhandle_t shader, S32 textStyle );
    void HideTeamMenu( void );
    void ShowTeamMenu( void );
    S32 ClientNumFromName( StringEntry p );
    static void RunMenuScript( UTF8** args );
    static void DrawLighting( void );
    static void CenterPrint( StringEntry str, S32 y, S32 charWidth );
    static void DrawCenterString( void );
    static void DrawVote( void );
    static void DrawTeamVote( void );
    static bool DrawScoreboard( void );
    static void DrawIntermission( void );
    static bool DrawFollow( void );
    static bool DrawQueue( void );
    static void DrawBotInfo( void );
    static void Draw2D( void );
    static void ScalePainBlendTCs( F32* s1, F32* t1, F32* s2, F32* t2 );
    static void PainBlend( void );
    static void ResetPainBlend( void );
    void DrawActive( stereoFrame_t stereoView );
    static void SizeUp_f( void );
    static void SizeDown_f( void );
    static void Viewpos_f( void );
    static bool RequestScores( void );
    static void scrollScoresDown_f( void );
    static void scrollScoresUp_f( void );
    static void ScoresDown_f( void );
    static void ScoresUp_f( void );
    static void TellTarget_f( void );
    static void TellAttacker_f( void );
    static void SquadMark_f( void );
    void InitConsoleCommands( void );
    static void AlienBuildableExplosion( vec3_t origin, vec3_t dir );
    static void HumanBuildableExplosion( vec3_t origin, vec3_t dir );
    static void Creep( centity_t* cent );
    static bool ParseBuildableAnimationFile( StringEntry filename, buildable_t buildable );
    static bool ParseBuildableSoundFile( StringEntry filename, buildable_t buildable );
    void InitBuildables( void );
    static void SetBuildableLerpFrameAnimation( buildable_t buildable, lerpFrame_t* lf, S32 newAnimation );
    static void RunBuildableLerpFrame( centity_t* cent );
    static void BuildableAnimation( centity_t* cent, S32* old, S32* now, F32* backLerp );
    static void PositionAndOrientateBuildable( const vec3_t angles, const vec3_t inOrigin, const vec3_t normal, const S32 skipNumber, const vec3_t mins, const vec3_t maxs, vec3_t outAxis[3], vec3_t outOrigin );
    static void GhostBuildable( buildable_t buildable );
    static void BuildableParticleEffects( centity_t* cent );
    static void BuildableStatusParse( StringEntry filename, buildStat_t* bs );
    static void BuildableStatusDisplay( centity_t* cent );
    static S32 SortDistance( const void* a, const void* b );
    static bool PlayerIsBuilder( buildable_t buildable );
    static void DrawBuildableStatus( void );
    static void Buildable( centity_t* cent );
    static bool AttachmentPoint( attachment_t* a, vec3_t v );
    static bool AttachmentDir( attachment_t* a, vec3_t v );
    static bool AttachmentAxis( attachment_t* a, vec3_t axis[3] );
    static bool AttachmentVelocity( attachment_t* a, vec3_t v );
    static S32 AttachmentCentNum( attachment_t* a );
    static bool Attached( attachment_t* a );
    static void AttachToPoint( attachment_t* a );
    static void AttachToCent( attachment_t* a );
    static void AttachToTag( attachment_t* a );
    static void AttachToParticle( attachment_t* a );
    static void SetAttachmentPoint( attachment_t* a, vec3_t v );
    static void SetAttachmentCent( attachment_t* a, centity_t* cent );
    static void SetAttachmentTag( attachment_t* a, refEntity_t parent, qhandle_t model, UTF8* tagName );
    static void SetAttachmentParticle( attachment_t* a, particle_t* p );
    static void SetAttachmentOffset( attachment_t* a, vec3_t v );
    static void DoorAnimation( centity_t* cent, S32* old, S32* now, F32* backLerp );
    static void ModelDoor( centity_t* cent );
    static void AMOAnimation( centity_t* cent, S32* old, S32* now, F32* backLerp );
    static void AnimMapObj( centity_t* cent );
    static void RunLerpFrame( lerpFrame_t* lf, F32 scale );
};

extern idCGameLocal cgameLocal;

#endif //!__CG_LOCAL_H__//
