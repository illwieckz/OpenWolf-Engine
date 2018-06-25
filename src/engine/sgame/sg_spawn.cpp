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
// File name:   sg_spawn.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

bool idGameLocal::SpawnString( StringEntry key, StringEntry defaultString, UTF8** out )
{
    S32   i;
    
    if( !level.spawning )
    {
        *out = ( UTF8* )defaultString;
        //Error( "idGameLocal::SpawnString() called while not spawning" );
    }
    
    for( i = 0; i < level.numSpawnVars; i++ )
    {
        if( !Q_stricmp( key, level.spawnVars[ i ][ 0 ] ) )
        {
            *out = level.spawnVars[ i ][ 1 ];
            return true;
        }
    }
    
    *out = ( UTF8* )defaultString;
    return false;
}

bool idGameLocal::SpawnFloat( StringEntry key, StringEntry defaultString, F32* out )
{
    UTF8*    s;
    bool  present;
    
    present = SpawnString( key, defaultString, &s );
    *out = atof( s );
    return present;
}

bool idGameLocal::SpawnInt( StringEntry key, StringEntry defaultString, S32* out )
{
    UTF8*      s;
    bool  present;
    
    present = SpawnString( key, defaultString, &s );
    *out = atoi( s );
    return present;
}

bool idGameLocal::SpawnVector( StringEntry key, StringEntry defaultString, F32* out )
{
    UTF8*    s;
    bool  present;
    
    present = SpawnString( key, defaultString, &s );
    sscanf( s, "%f %f %f", &out[ 0 ], &out[ 1 ], &out[ 2 ] );
    return present;
}

bool idGameLocal::SpawnVector4( StringEntry key, StringEntry defaultString, F32* out )
{
    UTF8*    s;
    bool  present;
    
    present = SpawnString( key, defaultString, &s );
    sscanf( s, "%f %f %f %f", &out[ 0 ], &out[ 1 ], &out[ 2 ], &out[ 3 ] );
    return present;
}

//
// fields are needed for spawning from the entity string
//
typedef enum
{
    F_INT,
    F_FLOAT,
    F_LSTRING,      // string on disk, pointer in memory, TAG_LEVEL
    F_GSTRING,      // string on disk, pointer in memory, TAG_GAME
    F_VECTOR,
    F_VECTOR4,
    F_ANGLEHACK,
    F_ENTITY,     // index on disk, pointer in memory
    F_ITEM,       // index on disk, pointer in memory
    F_CLIENT,     // index on disk, pointer in memory
    F_IGNORE
} fieldtype_t;

typedef struct
{
    UTF8*  name;
    S32   ofs;
    fieldtype_t type;
    S32   flags;
} field_t;

field_t fields[ ] =
{
    {"classname", FOFS( classname ), F_LSTRING},
    {"origin", FOFS( s.origin ), F_VECTOR},
    {"model", FOFS( model ), F_LSTRING},
    {"model2", FOFS( model2 ), F_LSTRING},
    {"spawnflags", FOFS( spawnflags ), F_INT},
    {"speed", FOFS( speed ), F_FLOAT},
    {"target", FOFS( target ), F_LSTRING},
    {"targetname", FOFS( targetname ), F_LSTRING},
    {"message", FOFS( message ), F_LSTRING},
    {"team", FOFS( team ), F_LSTRING},
    {"wait", FOFS( wait ), F_FLOAT},
    {"random", FOFS( random ), F_FLOAT},
    {"count", FOFS( count ), F_INT},
    {"health", FOFS( health ), F_INT},
    {"light", 0, F_IGNORE},
    {"dmg", FOFS( damage ), F_INT},
    {"angles", FOFS( s.angles ), F_VECTOR},
    {"angle", FOFS( s.angles ), F_ANGLEHACK},
    {"bounce", FOFS( physicsBounce ), F_FLOAT},
    {"alpha", FOFS( pos1 ), F_VECTOR},
    {"radius", FOFS( pos2 ), F_VECTOR},
    {"acceleration", FOFS( acceleration ), F_VECTOR},
    {"animation", FOFS( animation ), F_VECTOR4},
    {"rotatorAngle", FOFS( rotatorAngle ), F_FLOAT},
    {"targetShaderName", FOFS( targetShaderName ), F_LSTRING},
    {"targetShaderNewName", FOFS( targetShaderNewName ), F_LSTRING},
    
    {NULL}
};

typedef struct
{
    UTF8*  name;
    void ( *spawn )( gentity_t* ent );
} spawn_t;

spawn_t spawns[ ] =
{
    // info entities don't do anything at all, but provide positional
    // information for things controlled by other processes
    { "info_player_start",        &idGameLocal::SP_info_player_start },
    { "info_player_deathmatch",   &idGameLocal::SP_info_player_deathmatch },
    { "info_player_intermission", &idGameLocal::SP_info_player_intermission },
    
    { "info_alien_intermission",  &idGameLocal::SP_info_alien_intermission },
    { "info_human_intermission",  &idGameLocal::SP_info_human_intermission },
    
    { "info_null",                &idGameLocal::SP_info_null },
    { "info_notnull",             &idGameLocal::SP_info_notnull },    // use target_position instead
    
    { "func_plat",                &idGameLocal::SP_func_plat },
    { "func_button",              &idGameLocal::SP_func_button },
    { "func_door",                &idGameLocal::SP_func_door },
    { "func_door_rotating",       &idGameLocal::SP_func_door_rotating },
    { "func_door_model",          &idGameLocal::SP_func_door_model },
    { "func_static",              &idGameLocal::SP_func_static },
    { "func_rotating",            &idGameLocal::SP_func_rotating },
    { "func_bobbing",             &idGameLocal::SP_func_bobbing },
    { "func_pendulum",            &idGameLocal::SP_func_pendulum },
    { "func_train",               &idGameLocal::SP_func_train },
    { "func_group",               &idGameLocal::SP_info_null },
    { "func_timer",               &idGameLocal::SP_func_timer },      // rename trigger_timer?
    
    // Triggers are brush objects that cause an effect when contacted
    // by a living player, usually involving firing targets.
    // While almost everything could be done with
    // a single trigger class and different targets, triggered effects
    // could not be client side predicted (push and teleport).
    { "trigger_always",           &idGameLocal::SP_trigger_always },
    { "trigger_multiple",         &idGameLocal::SP_trigger_multiple },
    { "trigger_push",             &idGameLocal::SP_trigger_push },
    { "trigger_teleport",         &idGameLocal::SP_trigger_teleport },
    { "trigger_hurt",             &idGameLocal::SP_trigger_hurt },
    { "trigger_stage",            &idGameLocal::SP_trigger_stage },
    { "trigger_win",              &idGameLocal::SP_trigger_win },
    { "trigger_buildable",        &idGameLocal::SP_trigger_buildable },
    { "trigger_class",            &idGameLocal::SP_trigger_class },
    { "trigger_equipment",        &idGameLocal::SP_trigger_equipment },
    { "trigger_gravity",          &idGameLocal::SP_trigger_gravity },
    { "trigger_heal",             &idGameLocal::SP_trigger_heal },
    { "trigger_ammo",             &idGameLocal::SP_trigger_ammo },
    
    // targets perform no action by themselves, but must be triggered
    // by another entity
    { "target_delay",             &idGameLocal::SP_target_delay },
    { "target_speaker",           &idGameLocal::SP_target_speaker },
    { "target_print",             &idGameLocal::SP_target_print },
    { "target_score",             &idGameLocal::SP_target_score },
    { "target_teleporter",        &idGameLocal::SP_target_teleporter },
    { "target_relay",             &idGameLocal::SP_target_relay },
    { "target_kill",              &idGameLocal::SP_target_kill },
    { "target_position",          &idGameLocal::SP_target_position },
    { "target_location",          &idGameLocal::SP_target_location },
    { "target_push",              &idGameLocal::SP_target_push },
    { "target_rumble",            &idGameLocal::SP_target_rumble },
    { "target_alien_win",         &idGameLocal::SP_target_alien_win },
    { "target_human_win",         &idGameLocal::SP_target_human_win },
    { "target_hurt",              &idGameLocal::SP_target_hurt },
    
    { "light",                    &idGameLocal::SP_light },
    { "path_corner",              &idGameLocal::SP_path_corner },
    
    { "misc_teleporter_dest",     &idGameLocal::SP_misc_teleporter_dest },
    { "misc_model",               &idGameLocal::SP_misc_model },
    { "misc_portal_surface",      &idGameLocal::SP_misc_portal_surface },
    { "misc_portal_camera",       &idGameLocal::SP_misc_portal_camera },
    
    { "misc_particle_system",     &idGameLocal::SP_misc_particle_system },
    { "misc_anim_model",          &idGameLocal::SP_misc_anim_model },
    { "misc_light_flare",         &idGameLocal::SP_misc_light_flare },
    
    { "misc_cubemap",             &idGameLocal::SP_misc_cubemap },
    
    { NULL, 0 }
};

/*
===============
idGameLocal::CallSpawn

Finds the spawn function for the entity and calls it,
returning false if not found
===============
*/
bool idGameLocal::CallSpawn( gentity_t* ent )
{
    spawn_t*     s;
    buildable_t buildable;
    
    if( !ent->classname )
    {
        Printf( "G_CallSpawn: NULL classname\n" );
        return false;
    }
    
    //check buildable spawn functions
    buildable = bggame->BuildableByEntityName( ent->classname )->number;
    if( buildable != BA_NONE )
    {
        // don't spawn built-in buildings if we are using a custom layout
        if( level.layout[ 0 ] && Q_stricmp( level.layout, "*BUILTIN*" ) )
            return true;
            
        if( buildable == BA_A_SPAWN || buildable == BA_H_SPAWN )
        {
            ent->s.angles[ YAW ] += 180.0f;
            AngleNormalize360( ent->s.angles[ YAW ] );
        }
        
        SpawnBuildable( ent, buildable );
        return true;
    }
    
    // check normal spawn functions
    for( s = spawns; s->name; s++ )
    {
        if( !strcmp( s->name, ent->classname ) )
        {
            // found it
            s->spawn( ent );
            return true;
        }
    }
    
    Printf( "%s doesn't have a spawn function\n", ent->classname );
    return false;
}

/*
=============
idGameLocal::NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
UTF8* idGameLocal::NewString( StringEntry string )
{
    UTF8*  newb, *new_p;
    S32   i, l;
    
    l = strlen( string ) + 1;
    
    newb = ( UTF8* )bggame->Alloc( l );
    
    new_p = newb;
    
    // turn \n into a real linefeed
    for( i = 0 ; i < l ; i++ )
    {
        if( string[ i ] == '\\' && i < l - 1 )
        {
            i++;
            if( string[ i ] == 'n' )
                *new_p++ = '\n';
            else
                *new_p++ = '\\';
        }
        else
            *new_p++ = string[ i ];
    }
    
    return newb;
}

/*
===============
idGameLocal::ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/
void idGameLocal::ParseField( StringEntry key, StringEntry value, gentity_t* ent )
{
    field_t* f;
    U8* b;
    F32 v;
    vec3_t vec;
    vec4_t vec4;
    
    for( f = fields; f->name; f++ )
    {
        if( !Q_stricmp( f->name, key ) )
        {
            // found it
            b = ( U8* )ent;
            
            switch( f->type )
            {
                case F_LSTRING:
                    *( UTF8** )( b + f->ofs ) = NewString( value );
                    break;
                    
                case F_VECTOR:
                    sscanf( value, "%f %f %f", &vec[ 0 ], &vec[ 1 ], &vec[ 2 ] );
                    
                    ( ( F32* )( b + f->ofs ) )[ 0 ] = vec[ 0 ];
                    ( ( F32* )( b + f->ofs ) )[ 1 ] = vec[ 1 ];
                    ( ( F32* )( b + f->ofs ) )[ 2 ] = vec[ 2 ];
                    break;
                    
                case F_VECTOR4:
                    sscanf( value, "%f %f %f %f", &vec4[ 0 ], &vec4[ 1 ], &vec4[ 2 ], &vec4[ 3 ] );
                    
                    ( ( F32* )( b + f->ofs ) )[ 0 ] = vec4[ 0 ];
                    ( ( F32* )( b + f->ofs ) )[ 1 ] = vec4[ 1 ];
                    ( ( F32* )( b + f->ofs ) )[ 2 ] = vec4[ 2 ];
                    ( ( F32* )( b + f->ofs ) )[ 3 ] = vec4[ 3 ];
                    break;
                    
                case F_INT:
                    *( S32* )( b + f->ofs ) = atoi( value );
                    break;
                    
                case F_FLOAT:
                    *( F32* )( b + f->ofs ) = atof( value );
                    break;
                    
                case F_ANGLEHACK:
                    v = atof( value );
                    ( ( F32* )( b + f->ofs ) )[ 0 ] = 0;
                    ( ( F32* )( b + f->ofs ) )[ 1 ] = v;
                    ( ( F32* )( b + f->ofs ) )[ 2 ] = 0;
                    break;
                    
                default:
                case F_IGNORE:
                    break;
            }
            
            return;
        }
    }
}

/*
===================
idGameLocal::SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
void idGameLocal::SpawnGEntityFromSpawnVars( void )
{
    S32 i;
    gentity_t* ent;
    
    // get the next free entity
    ent = Spawn( );
    
    for( i = 0; i < level.numSpawnVars; i++ )
    {
        ParseField( level.spawnVars[i][0], level.spawnVars[i][1], ent );
    }
    
    SpawnInt( "notq3a", "0", &i );
    
    if( i )
    {
        FreeEntity( ent );
        return;
    }
    
    // move editor origin to pos
    VectorCopy( ent->s.origin, ent->s.pos.trBase );
    VectorCopy( ent->s.origin, ent->r.currentOrigin );
    
    // if we didn't get a classname, don't bother spawning anything
    if( !CallSpawn( ent ) )
    {
        FreeEntity( ent );
    }
}

/*
====================
idGameLocal::AddSpawnVarToken
====================
*/
UTF8* idGameLocal::AddSpawnVarToken( StringEntry string )
{
    S32 l;
    UTF8*  dest;
    
    l = strlen( string );
    
    if( level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS )
    {
        Error( "G_AddSpawnVarToken: MAX_SPAWN_CHARS" );
    }
    
    dest = level.spawnVarChars + level.numSpawnVarChars;
    ::memcpy( dest, string, l + 1 );
    
    level.numSpawnVarChars += l + 1;
    
    return dest;
}

/*
====================
idGameLocal::ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
bool idGameLocal::ParseSpawnVars( void )
{
    UTF8 keyname[ MAX_TOKEN_CHARS ];
    UTF8 com_token[ MAX_TOKEN_CHARS ];
    
    level.numSpawnVars = 0;
    level.numSpawnVarChars = 0;
    
    // parse the opening brace
    if( !trap_GetEntityToken( com_token, sizeof( com_token ) ) )
    {
        // end of spawn string
        return false;
    }
    
    if( com_token[0] != '{' )
    {
        Error( "idGameLocal::ParseSpawnVars: found %s when expecting {", com_token );
    }
    
    // go through all the key / value pairs
    while( 1 )
    {
        // parse key
        if( !trap_GetEntityToken( keyname, sizeof( keyname ) ) )
        {
            Error( "idGameLocal::ParseSpawnVars: EOF without closing brace" );
        }
        
        if( keyname[0] == '}' )
        {
            break;
        }
        
        // parse value
        if( !trap_GetEntityToken( com_token, sizeof( com_token ) ) )
        {
            Error( "idGameLocal::ParseSpawnVars: EOF without closing brace" );
        }
        
        if( com_token[0] == '}' )
        {
            Error( "idGameLocal::ParseSpawnVars: closing brace without data" );
        }
        
        if( level.numSpawnVars == MAX_SPAWN_VARS )
        {
            Error( "idGameLocal::ParseSpawnVars: MAX_SPAWN_VARS" );
        }
        
        level.spawnVars[ level.numSpawnVars ][ 0 ] = AddSpawnVarToken( keyname );
        level.spawnVars[ level.numSpawnVars ][ 1 ] = AddSpawnVarToken( com_token );
        level.numSpawnVars++;
    }
    
    return true;
}

/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"   music wav file
"gravity" 800 is default gravity
"message" Text to print during connection process
*/
void idGameLocal::SP_worldspawn( void )
{
    UTF8* s;
    
    SpawnString( "classname", "", &s );
    
    if( Q_stricmp( s, "worldspawn" ) )
        Error( "SP_worldspawn: The first entity isn't 'worldspawn'" );
        
    // make some data visible to connecting client
    trap_SetConfigstring( CS_GAME_VERSION, GAME_VERSION );
    
    trap_SetConfigstring( CS_LEVEL_START_TIME, va( "%i", level.startTime ) );
    
    SpawnString( "music", "", &s );
    trap_SetConfigstring( CS_MUSIC, s );
    
    SpawnString( "message", "", &s );
    trap_SetConfigstring( CS_MESSAGE, s );        // map specific message
    
    trap_SetConfigstring( CS_MOTD, g_motd.string );   // message of the day
    
    SpawnString( "gravity", "800", &s );
    trap_Cvar_Set( "g_gravity", s );
    
    SpawnString( "humanBuildPoints", DEFAULT_HUMAN_BUILDPOINTS, &s );
    trap_Cvar_Set( "g_humanBuildPoints", s );
    
    SpawnString( "humanMaxStage", DEFAULT_HUMAN_MAX_STAGE, &s );
    trap_Cvar_Set( "g_humanMaxStage", s );
    
    //for compatibility with 1.1 maps
    if( SpawnString( "humanStage2Threshold", DEFAULT_HUMAN_STAGE_THRESH, &s ) )
        trap_Cvar_Set( "g_humanStageThreshold", s );
    else
    {
        //proper way
        SpawnString( "humanStageThreshold", DEFAULT_HUMAN_STAGE_THRESH, &s );
        trap_Cvar_Set( "g_humanStageThreshold", s );
    }
    SpawnString( "alienBuildPoints", DEFAULT_ALIEN_BUILDPOINTS, &s );
    trap_Cvar_Set( "g_alienBuildPoints", s );
    
    SpawnString( "alienMaxStage", DEFAULT_ALIEN_MAX_STAGE, &s );
    trap_Cvar_Set( "g_alienMaxStage", s );
    
    if( SpawnString( "alienStage2Threshold", DEFAULT_ALIEN_STAGE_THRESH, &s ) )
    {
        trap_Cvar_Set( "g_alienStageThreshold", s );
    }
    else
    {
        SpawnString( "alienStage2Threshold", DEFAULT_ALIEN_STAGE_THRESH, &s );
        trap_Cvar_Set( "g_alienStageThreshold", s );
    }
    
    SpawnString( "enableDust", "0", &s );
    trap_Cvar_Set( "g_enableDust", s );
    
    SpawnString( "enableBreath", "0", &s );
    trap_Cvar_Set( "g_enableBreath", s );
    
    SpawnString( "disabledEquipment", "", &s );
    trap_Cvar_Set( "g_disabledEquipment", s );
    
    SpawnString( "disabledClasses", "", &s );
    trap_Cvar_Set( "g_disabledClasses", s );
    
    SpawnString( "disabledBuildables", "", &s );
    trap_Cvar_Set( "g_disabledBuildables", s );
    
    g_entities[ ENTITYNUM_WORLD ].s.number = ENTITYNUM_WORLD;
    g_entities[ ENTITYNUM_WORLD ].classname = "worldspawn";
    
    // see if we want a warmup time
    trap_SetConfigstring( CS_WARMUP, "" );
    
    if( g_restarted.integer )
    {
        trap_Cvar_Set( "g_restarted", "0" );
        level.warmupTime = 0;
    }
    else if( g_doWarmup.integer )
    {
        // Turn it on
        level.warmupTime = -1;
        trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );
        LogPrintf( "Warmup:\n" );
    }
    
}

/*
==============
idGameLocal::SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void idGameLocal::SpawnEntitiesFromString( void )
{
    // allow calls to idGameLocal::Spawn*()
    level.spawning = true;
    level.numSpawnVars = 0;
    
    // the worldspawn is not an actual entity, but it still
    // has a "spawn" function to perform any global setup
    // needed by a level (setting configstrings or cvars, etc)
    if( !ParseSpawnVars( ) )
        Error( "idGameLocal::SpawnEntitiesFromString: no entities" );
        
    SP_worldspawn( );
    
    // parse ents
    while( ParseSpawnVars() )
    {
        SpawnGEntityFromSpawnVars();
    }
    
    level.spawning = false;      // any future calls to G_Spawn*() will be errors
}

