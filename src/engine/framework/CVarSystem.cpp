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
// File name:   CVarSystem.cpp
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description: dynamic variable tracking
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEDICATED
#include <null/null_precompiled.h>
#else
#include <OWLib/precompiled.h>
#endif

cvar_t* cvar_vars;
cvar_t* cvar_cheats;
S32 cvar_modifiedFlags;

#define MAX_CVARS 2048
cvar_t cvar_indexes[MAX_CVARS];
S32 cvar_numIndexes;

#define FILE_HASH_SIZE 512
static cvar_t* hashTable[FILE_HASH_SIZE];

idCVarSystemLocal cvarSystemLocal;
idCVarSystem* cvarSystem = &cvarSystemLocal;

/*
===============
idCVarSystemLocal::idCVarSystemLocal
===============
*/
idCVarSystemLocal::idCVarSystemLocal( void )
{
}

/*
===============
idCVarSystemLocal::~idCVarSystemLocal
===============
*/
idCVarSystemLocal::~idCVarSystemLocal( void )
{
}

/*
================
idCVarSystemLocal::generateHashValue

return a hash value for the filename
================
*/
S64 idCVarSystemLocal::generateHashValue( StringEntry fname )
{
    S32 i;
    S64 hash;
    UTF8 letter;
    
    if( !fname )
    {
        Com_Error( ERR_DROP, "null name in generateHashValue" );	//gjd
    }
    
    hash = 0;
    i = 0;
    
    while( fname[i] != '\0' )
    {
        letter = tolower( fname[i] );
        hash += ( S64 )( letter ) * ( i + 119 );
        i++;
    }
    
    hash &= ( FILE_HASH_SIZE - 1 );
    
    return hash;
}

/*
============
idCVarSystemLocal::ValidateString
============
*/
bool idCVarSystemLocal::ValidateString( StringEntry s )
{
    if( !s )
    {
        return false;
    }
    if( strchr( s, '\\' ) )
    {
        return false;
    }
    if( strchr( s, '\"' ) )
    {
        return false;
    }
    if( strchr( s, ';' ) )
    {
        return false;
    }
    return true;
}

/*
============
idCVarSystemLocal::FindVar
============
*/
cvar_t* idCVarSystemLocal::FindVar( StringEntry var_name )
{
    cvar_t* var;
    S64 hash;
    
    if( !var_name )
        return NULL;
        
    hash = generateHashValue( var_name );
    
    for( var = hashTable[hash]; var; var = var->hashNext )
    {
        if( !Q_stricmp( var_name, var->name ) )
        {
            return var;
        }
    }
    
    return NULL;
}

/*
============
idCVarSystemLocal::VariableValue
============
*/
F32 idCVarSystemLocal::VariableValue( StringEntry var_name )
{
    cvar_t* var;
    
    var = FindVar( var_name );
    
    if( !var )
    {
        return 0;
    }
    
    return var->value;
}


/*
============
idCVarSystemLocal::VariableIntegerValue
============
*/
S32 idCVarSystemLocal::VariableIntegerValue( StringEntry var_name )
{
    cvar_t* var;
    
    var = idCVarSystemLocal::FindVar( var_name );
    
    if( !var )
    {
        return 0;
    }
    
    return var->integer;
}


/*
============
idCVarSystemLocal::VariableString
============
*/
UTF8* idCVarSystemLocal::VariableString( StringEntry var_name )
{
    cvar_t*         var;
    
    var = FindVar( var_name );
    
    if( !var )
    {
        return "";
    }
    
    return var->string;
}


/*
============
idCVarSystemLocal::VariableStringBuffer
============
*/
void idCVarSystemLocal::VariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize )
{
    cvar_t* var;
    
    var = FindVar( var_name );
    if( !var )
    {
        *buffer = 0;
    }
    else
    {
        Q_strncpyz( buffer, var->string, bufsize );
    }
}

/*
============
idCVarSystemLocal::VariableStringBuffer
============
*/
void idCVarSystemLocal::LatchedVariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize )
{
    cvar_t* var;
    
    var = FindVar( var_name );
    
    if( !var )
    {
        *buffer = 0;
    }
    else
    {
        if( var->latchedString )
        {
            Q_strncpyz( buffer, var->latchedString, bufsize );
        }
        else
        {
            Q_strncpyz( buffer, var->string, bufsize );
        }
    }
}

/*
============
idCVarSystemLocal::Flags
============
*/
S32 idCVarSystemLocal::Flags( StringEntry var_name )
{
    cvar_t* var;
    
    if( !( var = FindVar( var_name ) ) )
    {
        return CVAR_NONEXISTENT;
    }
    else
    {
        return var->flags;
    }
}

/*
============
idCVarSystemLocal::CommandCompletion
============
*/
void idCVarSystemLocal::CommandCompletion( void ( *callback )( StringEntry s ) )
{
    cvar_t* cvar;
    
    for( cvar = cvar_vars; cvar; cvar = cvar->next )
    {
        callback( cvar->name );
    }
}

/*
============
idCVarSystemLocal::ClearForeignCharacters
some cvar values need to be safe from foreign characters
============
*/
UTF8* idCVarSystemLocal::ClearForeignCharacters( StringEntry value )
{
    static UTF8 clean[MAX_CVAR_VALUE_STRING];
    S32 i, j;
    
    j = 0;
    
    for( i = 0; value[i] != '\0'; i++ )
    {
        //if( !(value[i] & 128) )
        if( ( ( U8* ) value )[i] != 0xFF && ( ( ( U8* ) value )[i] <= 127 || ( ( U8* ) value )[i] >= 161 ) )
        {
            clean[j] = value[i];
            j++;
        }
    }
    
    clean[j] = '\0';
    
    return clean;
}

/*
============
idCVarSystemLocal::Get

If the variable already exists, the value will not be set unless CVAR_ROM
The flags will be or'ed in if the variable exists.
============
*/
cvar_t* idCVarSystemLocal::Get( StringEntry var_name, StringEntry var_value, S32 flags )
{
    cvar_t* var;
    S64 hash;
    
    if( !var_name || !var_value )
    {
        Com_Error( ERR_FATAL, "idCVarSystemLocal::Get: NULL parameter" );
    }
    
    if( !ValidateString( var_name ) )
    {
        Com_Printf( "idCVarSystemLocal::Get: invalid cvar name string: %s\n", var_name );
        var_name = "BADNAME";
    }
    
#if 0 // FIXME: values with backslash happen
    if( !Cvar_ValidateString( var_value ) )
    {
        Com_Printf( "idCVarSystemLocal::Get: invalid cvar value string: %s\n", var_value );
        var_value = "BADVALUE";
    }
#endif
    
    var = FindVar( var_name );
    if( var )
    {
        // if the C code is now specifying a variable that the user already
        // set a value for, take the new value as the reset value
        if( ( var->flags & CVAR_USER_CREATED ) && !( flags & CVAR_USER_CREATED ) && var_value[0] )
        {
            var->flags &= ~CVAR_USER_CREATED;
            Z_Free( var->resetString );
            var->resetString = CopyString( var_value );
            
            // ZOID--needs to be set so that cvars the game sets as
            // SERVERINFO get sent to clients
            cvar_modifiedFlags |= flags;
        }
        
        var->flags |= flags;
        
        // only allow one non-empty reset string without a warning
        if( !var->resetString[0] )
        {
            // we don't have a reset string yet
            Z_Free( var->resetString );
            var->resetString = CopyString( var_value );
        }
        else if( var_value[0] && strcmp( var->resetString, var_value ) )
        {
            Com_DPrintf( "idCVarSystemLocal::Get: Warning: cvar \"%s\" given initial values: \"%s\" and \"%s\"\n", var_name, var->resetString, var_value );
        }
        
        // if we have a latched string, take that value now
        if( var->latchedString )
        {
            UTF8*           s;
            
            s = var->latchedString;
            var->latchedString = NULL;	// otherwise idCVarSystemLocal::GetSet2 would free it
            GetSet2( var_name, s, true );
            Z_Free( s );
        }
        
        // TTimo
        // if CVAR_USERINFO was toggled on for an existing cvar, check wether the value needs to be cleaned from foreigh characters
        // (for instance, seta name "name-with-foreign-chars" in the config file, and toggle to CVAR_USERINFO happens later in CL_Init)
        if( flags & CVAR_USERINFO )
        {
            UTF8* cleaned = ClearForeignCharacters( var->string );// NOTE: it is probably harmless to call idCVarSystemLocal::Set2 in all cases, but I don't want to risk it
            
            if( strcmp( var->string, cleaned ) )
            {
                GetSet2( var->name, var->string, false );	// call idCVarSystemLocal::Set2 with the value to be cleaned up for verbosity
            }
        }
        
// use a CVAR_SET for rom sets, get won't override
#if 0
        // CVAR_ROM always overrides
        if( flags & CVAR_ROM )
        {
            Set2( var_name, var_value, true );
        }
#endif
        return var;
    }
    
    //
    // allocate a new cvar
    //
    if( cvar_numIndexes >= MAX_CVARS )
    {
        Com_Error( ERR_FATAL, "idCVarSystemLocal::Get: MAX_CVARS (%d) hit -- too many cvars!", MAX_CVARS );
    }
    
    var = &cvar_indexes[cvar_numIndexes];
    cvar_numIndexes++;
    var->name = CopyString( var_name );
    var->string = CopyString( var_value );
    var->modified = true;
    var->modificationCount = 1;
    var->value = atof( var->string );
    var->integer = atoi( var->string );
    var->resetString = CopyString( var_value );
    
    // link the variable in
    var->next = cvar_vars;
    cvar_vars = var;
    
    var->flags = flags;
    
    hash = generateHashValue( var_name );
    var->hashNext = hashTable[hash];
    hashTable[hash] = var;
    
    return var;
}

/*
============
idCVarSystemLocal::GetSet2
============
*/
#define FOREIGN_MSG "Foreign characters are not allowed in userinfo variables.\n"
#ifndef DEDICATED
StringEntry     CL_TranslateStringBuf( StringEntry string );
#endif
cvar_t* idCVarSystemLocal::GetSet2( StringEntry var_name, StringEntry value, bool force )
{
    cvar_t* var;
    
    if( strcmp( "com_hunkused", var_name ) != 0 )
    {
        Com_DPrintf( "idCVarSystemLocal::Set2: %s %s\n", var_name, value );
    }
    
    if( !ValidateString( var_name ) )
    {
        Com_Printf( "invalid cvar name string: %s\n", var_name );
        var_name = "BADNAME";
    }
    
    var = FindVar( var_name );
    if( !var )
    {
        if( !value )
        {
            return NULL;
        }
        
        // create it
        if( !force )
        {
            return Get( var_name, value, CVAR_USER_CREATED );
        }
        else
        {
            return Get( var_name, value, 0 );
        }
    }
    
    if( !value )
    {
        value = var->resetString;
    }
    
    if( var->flags & CVAR_USERINFO )
    {
        UTF8* cleaned = ClearForeignCharacters( value );
        
        if( strcmp( value, cleaned ) )
        {
#ifdef DEDICATED
            Com_Printf( FOREIGN_MSG );
#else
            Com_Printf( "%s", CL_TranslateStringBuf( FOREIGN_MSG ) );
#endif
            Com_Printf( "Using %s instead of %s\n", cleaned, value );
            return GetSet2( var_name, cleaned, force );
        }
    }
    
    if( !strcmp( value, var->string ) )
    {
        if( ( var->flags & CVAR_LATCH ) && var->latchedString )
        {
            if( !strcmp( value, var->latchedString ) )
            {
                return var;
            }
        }
        else
        {
            return var;
        }
    }
    
    // note what types of cvars have been modified (userinfo, archive, serverinfo, systeminfo)
    cvar_modifiedFlags |= var->flags;
    
    if( !force )
    {
        // ydnar: don't set unsafe variables when com_crashed is set
        if( ( var->flags & CVAR_UNSAFE ) && com_crashed != NULL && com_crashed->integer )
        {
            Com_Printf( "%s is unsafe. Check com_crashed.\n", var_name );
            return var;
        }
        
        if( var->flags & CVAR_ROM )
        {
            Com_Printf( "%s is read only.\n", var_name );
            return var;
        }
        
        if( var->flags & CVAR_INIT )
        {
            Com_Printf( "%s is write protected.\n", var_name );
            return var;
        }
        
        if( ( var->flags & CVAR_CHEAT ) && !cvar_cheats->integer )
        {
            Com_Printf( "%s is cheat protected.\n", var_name );
            return var;
        }
        
        if( var->flags & CVAR_SHADER )
        {
            Com_Printf( "%s will be changed upon recompiling shaders.\n", var_name );
            Set( "r_recompileShaders", "1" );
        }
        
        if( var->flags & CVAR_LATCH )
        {
            if( var->latchedString )
            {
                if( strcmp( value, var->latchedString ) == 0 )
                {
                    return var;
                }
                
                Z_Free( var->latchedString );
            }
            else
            {
                if( strcmp( value, var->string ) == 0 )
                {
                    return var;
                }
            }
            
            Com_Printf( "%s will be changed upon restarting.\n", var_name );
            
            var->latchedString = CopyString( value );
            var->modified = true;
            var->modificationCount++;
            return var;
        }
    }
    else
    {
        if( var->latchedString )
        {
            Z_Free( var->latchedString );
            var->latchedString = NULL;
        }
    }
    
    if( !strcmp( value, var->string ) )
    {
        return var;				// not changed
        
    }
    var->modified = true;
    var->modificationCount++;
    
    Z_Free( var->string );		// free the old value string
    
    var->string = CopyString( value );
    var->value = atof( var->string );
    var->integer = atoi( var->string );
    
    return var;
}

/*
============
idCVarSystemLocal::Set
============
*/
void idCVarSystemLocal::Set( StringEntry var_name, StringEntry value )
{
    GetSet2( var_name, value, true );
}

/*
============
idCVarSystemLocal::SetLatched
============
*/
void idCVarSystemLocal::SetLatched( StringEntry var_name, StringEntry value )
{
    GetSet2( var_name, value, false );
}

/*
============
idCVarSystemLocal::SetValue
============
*/
void idCVarSystemLocal::SetValue( StringEntry var_name, F32 value )
{
    UTF8 val[32];
    
    if( value == ( S32 )value )
    {
        Com_sprintf( val, sizeof( val ), "%i", ( S32 )value );
    }
    else
    {
        Com_sprintf( val, sizeof( val ), "%f", value );
    }
    
    Set( var_name, val );
}

/*
============
idCVarSystemLocal::SetValueSafe
============
*/
void idCVarSystemLocal::SetValueSafe( StringEntry var_name, F32 value )
{
    UTF8    val[32];
    
    if( value == ( S32 )value )
    {
        Com_sprintf( val, sizeof( val ), "%i", ( S32 )value );
    }
    else
    {
        Com_sprintf( val, sizeof( val ), "%f", value );
    }
    
    GetSet2( var_name, val, false );
}

/*
============
idCVarSystemLocal::SetValueLatched
============
*/
void idCVarSystemLocal::SetValueLatched( StringEntry var_name, F32 value )
{
    UTF8 val[32];
    
    if( value == ( S32 )value )
    {
        Com_sprintf( val, sizeof( val ), "%i", ( S32 )value );
    }
    else
    {
        Com_sprintf( val, sizeof( val ), "%f", value );
    }
    
    GetSet2( var_name, val, false );
}

/*
============
idCVarSystemLocal::Reset
============
*/
void idCVarSystemLocal::Reset( StringEntry var_name )
{
    GetSet2( var_name, NULL, false );
}

/*
============
idCVarSystemLocal::SetCheatState

Any testing variables will be reset to the safe values
============
*/
void idCVarSystemLocal::SetCheatState( void )
{
    cvar_t* var;
    
    // set all default vars to the safe value
    for( var = cvar_vars; var; var = var->next )
    {
        if( var->flags & CVAR_CHEAT )
        {
            if( strcmp( var->resetString, var->string ) )
            {
                Set( var->name, var->resetString );
            }
        }
    }
}

/*
============
idCVarSystemLocal::Command

Handles variable inspection and changing from the console
============
*/
bool idCVarSystemLocal::Command( void )
{
    cvar_t* v;
    
    // check variables
    v = FindVar( Cmd_Argv( 0 ) );
    if( !v )
    {
        return false;
    }
    
    // perform a variable print or set
    if( Cmd_Argc() == 1 )
    {
        Com_Printf( "\"%s\" is:\"%s" S_COLOR_WHITE "\" default:\"%s" S_COLOR_WHITE "\"\n", v->name, v->string, v->resetString );
        
        if( v->latchedString )
        {
            Com_Printf( "latched: \"%s\"\n", v->latchedString );
        }
        
        return true;
    }
    
    // set the value if forcing isn't required
    GetSet2( v->name, Cmd_Argv( 1 ), false );
    return true;
}


/*
============
idCVarSystemLocal::Toggle_f

Toggles a cvar for easy single key binding,
optionally through a list of given values
============
*/
void idCVarSystemLocal::Toggle_f( void )
{
    S32 i, c;
    StringEntry varname, curval;
    
    c = Cmd_Argc();
    if( c < 2 )
    {
        Com_Printf( "usage: toggle <variable> [<value> ...]\n" );
        return;
    }
    
    varname = Cmd_Argv( 1 );
    
    if( c == 2 )
    {
        cvarSystemLocal.GetSet2( varname, va( "%d", !cvarSystemLocal.VariableValue( varname ) ), false );
        return;
    }
    
    curval = cvarSystemLocal.VariableString( Cmd_Argv( 1 ) );
    
    // don't bother checking the last value for a match, since the desired
    //  behaviour is the same as if the last value didn't match:
    //  set the variable to the first value
    for( i = 2; i < c - 1; ++i )
    {
        if( !strcmp( curval, Cmd_Argv( i ) ) )
        {
            cvarSystemLocal.GetSet2( varname, Cmd_Argv( i + 1 ), false );
            return;
        }
    }
    
    // fallback
    cvarSystemLocal.GetSet2( varname, Cmd_Argv( 2 ), false );
}


/*
============
idCVarSystemLocal::Cycle_f

Cycles a cvar for easy single key binding
============
*/
void idCVarSystemLocal::Cycle_f( void )
{
    S32 start, end, step, oldvalue, value;
    
    if( Cmd_Argc() < 4 || Cmd_Argc() > 5 )
    {
        Com_Printf( "usage: cycle <variable> <start> <end> [step]\n" );
        return;
    }
    
    oldvalue = value = cvarSystemLocal.VariableValue( Cmd_Argv( 1 ) );
    start = atoi( Cmd_Argv( 2 ) );
    end = atoi( Cmd_Argv( 3 ) );
    
    if( Cmd_Argc() == 5 )
    {
        step = abs( atoi( Cmd_Argv( 4 ) ) );
    }
    else
    {
        step = 1;
    }
    
    if( abs( end - start ) < step )
    {
        step = 1;
    }
    
    if( end < start )
    {
        value -= step;
        if( value < end )
        {
            value = start - ( step - ( oldvalue - end + 1 ) );
        }
    }
    else
    {
        value += step;
        if( value > end )
        {
            value = start + ( step - ( end - oldvalue + 1 ) );
        }
    }
    
    cvarSystemLocal.GetSet2( Cmd_Argv( 1 ), va( "%i", value ), false );
}

/*
============
idCVarSystemLocal::Set_f

Allows setting and defining of arbitrary cvars from console, even if they
weren't declared in C code.
============
*/
void idCVarSystemLocal::Set_f( void )
{
    S32 c, unsafe = 0;
    UTF8* value;
    
    c = Cmd_Argc();
    if( c < 3 )
    {
        Com_Printf( "usage: set <variable> <value> [unsafe]\n" );
        return;
    }
    
    // ydnar: handle unsafe vars
    if( c >= 4 && !strcmp( Cmd_Argv( c - 1 ), "unsafe" ) )
    {
        c--;
        unsafe = 1;
        if( com_crashed != NULL && com_crashed->integer )
        {
            Com_Printf( "%s is unsafe. Check com_crashed.\n", Cmd_Argv( 1 ) );
            return;
        }
    }
    
    value = strdup( Cmd_Cmd_FromNth( 2 ) ); // 3rd arg onwards, raw
    
    if( unsafe )
    {
        UTF8* end = value + strlen( value );
        
        // skip spaces
        while( --end > value )
        {
            if( *end != ' ' )
            {
                break;
            }
        }
        ++end;
        
        // skip "unsafe" (may be quoted, so just scan it)
        while( --end > value )
        {
            if( *end == ' ' )
            {
                break;
            }
        }
        ++end;
        
        // skip spaces
        while( --end > value )
        {
            if( *end != ' ' )
            {
                break;
            }
        }
        end[1] = 0; // end of string :-)
    }
    
    cvarSystemLocal.GetSet2( Cmd_Argv( 1 ), Com_UnquoteStr( value ), false );
    free( value );
}

/*
============
idCVarSystemLocal::SetU_f

As Cvar_Set, but also flags it as serverinfo
============
*/
void idCVarSystemLocal::SetU_f( void )
{
    cvar_t* v;
    
    if( Cmd_Argc() != 3 && Cmd_Argc() != 4 )
    {
        Com_Printf( "usage: setu <variable> <value> [unsafe]\n" );
        return;
    }
    Set_f();
    
    v = FindVar( Cmd_Argv( 1 ) );
    if( !v )
    {
        return;
    }
    
    v->flags |= CVAR_USERINFO;
}

/*
============
idCVarSystemLocal::SetS_f

As Cvar_Set, but also flags it as serverinfo
============
*/
void idCVarSystemLocal::SetS_f( void )
{
    cvar_t* v;
    
    if( Cmd_Argc() != 3 && Cmd_Argc() != 4 )
    {
        Com_Printf( "usage: sets <variable> <value> [unsafe]\n" );
        return;
    }
    
    Set_f();
    
    v = FindVar( Cmd_Argv( 1 ) );
    if( !v )
    {
        return;
    }
    
    v->flags |= CVAR_SERVERINFO;
}

/*
============
idCVarSystemLocal::SetA_f

As Cvar_Set, but also flags it as archived
============
*/
void idCVarSystemLocal::SetA_f( void )
{
    cvar_t* v;
    
    if( Cmd_Argc() != 3 && Cmd_Argc() != 4 )
    {
        Com_Printf( "usage: seta <variable> <value> [unsafe]\n" );
        return;
    }
    
    Set_f();
    
    v = FindVar( Cmd_Argv( 1 ) );
    if( !v )
    {
        return;
    }
    
    v->flags |= CVAR_ARCHIVE;
}

/*
============
idCVarSystemLocal::Reset_f
============
*/
void idCVarSystemLocal::Reset_f( void )
{
    if( Cmd_Argc() != 2 )
    {
        Com_Printf( "usage: reset <variable>\n" );
        return;
    }
    
    cvarSystemLocal.Reset( Cmd_Argv( 1 ) );
}

/*
============
idCVarSystemLocal::WriteVariables

Appends lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void idCVarSystemLocal::WriteVariables( fileHandle_t f )
{
    cvar_t* var;
    UTF8 buffer[1024];
    
    for( var = cvar_vars; var; var = var->next )
    {
        if( Q_stricmp( var->name, "cl_cdkey" ) == 0 )
        {
            continue;
        }
        
        if( var->flags & CVAR_ARCHIVE )
        {
            // write the latched value, even if it hasn't taken effect yet
            Com_sprintf( buffer, sizeof( buffer ), "seta %s %s%s\n", var->name, Com_QuoteStr( var->latchedString ? var->latchedString : var->string ), ( var->flags & CVAR_UNSAFE ) ? " unsafe" : "" );
            fileSystem->Printf( f, "%s", buffer );
        }
    }
}

/*
============
idCVarSystemLocal::List_f
============
*/
void idCVarSystemLocal::List_f( void )
{
    cvar_t* var;
    S32 i;
    UTF8* match;
    
    if( Cmd_Argc() > 1 )
    {
        match = Cmd_Argv( 1 );
    }
    else
    {
        match = NULL;
    }
    
    i = 0;
    
    for( var = cvar_vars; var; var = var->next, i++ )
    {
        if( match && !Com_Filter( match, var->name, false ) )
        {
            continue;
        }
        
        if( var->flags & CVAR_SERVERINFO )
        {
            Com_Printf( "S" );
        }
        else
        {
            Com_Printf( " " );
        }
        
        if( var->flags & CVAR_USERINFO )
        {
            Com_Printf( "U" );
        }
        else
        {
            Com_Printf( " " );
        }
        
        if( var->flags & CVAR_ROM )
        {
            Com_Printf( "R" );
        }
        else
        {
            Com_Printf( " " );
        }
        
        if( var->flags & CVAR_INIT )
        {
            Com_Printf( "I" );
        }
        else
        {
            Com_Printf( " " );
        }
        
        if( var->flags & CVAR_ARCHIVE )
        {
            Com_Printf( "A" );
        }
        else
        {
            Com_Printf( " " );
        }
        
        if( var->flags & CVAR_LATCH )
        {
            Com_Printf( "L" );
        }
        else
        {
            Com_Printf( " " );
        }
        
        if( var->flags & CVAR_CHEAT )
        {
            Com_Printf( "C" );
        }
        else
        {
            Com_Printf( " " );
        }
        
        Com_Printf( " %s \"%s\"\n", var->name, var->string );
    }
    
    Com_Printf( "\n%i total cvars\n", i );
    Com_Printf( "%i cvar indexes\n", cvar_numIndexes );
}

/*
============
idCVarSystemLocal::Restart_f

Resets all cvars to their hardcoded values
============
*/
void idCVarSystemLocal::Restart_f( void )
{
    cvar_t* var;
    cvar_t** prev;
    
    prev = &cvar_vars;
    while( 1 )
    {
        var = *prev;
        if( !var )
        {
            break;
        }
        
        // don't mess with rom values, or some inter-module
        // communication will get broken (com_cl_running, etc)
        if( var->flags & ( CVAR_ROM | CVAR_INIT | CVAR_NORESTART ) )
        {
            prev = &var->next;
            continue;
        }
        
        // throw out any variables the user created
        if( var->flags & CVAR_USER_CREATED )
        {
            *prev = var->next;
            
            if( var->name )
            {
                Z_Free( var->name );
            }
            
            if( var->string )
            {
                Z_Free( var->string );
            }
            
            if( var->latchedString )
            {
                Z_Free( var->latchedString );
            }
            
            if( var->resetString )
            {
                Z_Free( var->resetString );
            }
            
            // clear the var completely, since we
            // can't remove the index from the list
            ::memset( var, 0, sizeof( *var ) );
            continue;
        }
        
        cvarSystemLocal.Set( var->name, var->resetString );
        
        prev = &var->next;
    }
}

/*
=====================
idCVarSystemLocal::InfoString
=====================
*/
UTF8* idCVarSystemLocal::InfoString( S32 bit )
{
    static UTF8 info[MAX_INFO_STRING];
    cvar_t* var;
    
    info[0] = 0;
    
    for( var = cvar_vars; var; var = var->next )
    {
        if( var->flags & bit )
        {
            Info_SetValueForKey( info, var->name, var->string );
        }
    }
    
    return info;
}

/*
=====================
idCVarSystemLocal::InfoString_Big

  handles large info strings ( CS_SYSTEMINFO )
=====================
*/
UTF8* idCVarSystemLocal::InfoString_Big( S32 bit )
{
    static UTF8 info[BIG_INFO_STRING];
    cvar_t* var;
    
    info[0] = 0;
    
    for( var = cvar_vars; var; var = var->next )
    {
        if( var->flags & bit )
        {
            Info_SetValueForKey_Big( info, var->name, var->string );
        }
    }
    
    return info;
}

/*
=====================
idCVarSystemLocal::InfoStringBuffer
=====================
*/
void idCVarSystemLocal::InfoStringBuffer( S32 bit, UTF8* buff, S32 buffsize )
{
    Q_strncpyz( buff, InfoString( bit ), buffsize );
}

/*
=====================
idCVarSystemLocal::CheckRange
=====================
*/
void idCVarSystemLocal::CheckRange( cvar_t* var, F32 min, F32 max, bool integral )
{
    var->validate = true;
    var->min = min;
    var->max = max;
    var->integral = integral;
    
    // Force an initial range check
    Set( var->name, var->string );
}

/*
=====================
Cvar_Register

basically a slightly modified Cvar_Get for the interpreted modules
=====================
*/
void idCVarSystemLocal::Register( vmCvar_t* vmCvar, StringEntry varName, StringEntry defaultValue, S32 flags )
{
    cvar_t* cv;
    
    cv = Get( varName, defaultValue, flags );
    
    if( !vmCvar )
    {
        return;
    }
    
    vmCvar->handle = cv - cvar_indexes;
    vmCvar->modificationCount = -1;
    
    Update( vmCvar );
}

/*
=====================
idCVarSystemLocal::Update

updates an interpreted modules' version of a cvar
=====================
*/
void idCVarSystemLocal::Update( vmCvar_t* vmCvar )
{
    cvar_t* cv = NULL;	// bk001129
    
    assert( vmCvar );				// bk
    
    if( ( U32 )vmCvar->handle >= cvar_numIndexes )
    {
        Com_Error( ERR_DROP, "idCVarSystemLocal::Update: handle %d out of range", ( U32 )vmCvar->handle );
    }
    
    cv = cvar_indexes + vmCvar->handle;
    
    if( cv->modificationCount == vmCvar->modificationCount )
    {
        return;
    }
    
    if( !cv->string )
    {
        return;					// variable might have been cleared by a cvar_restart
    }
    
    vmCvar->modificationCount = cv->modificationCount;
    
    // bk001129 - mismatches.
    if( strlen( cv->string ) + 1 > MAX_CVAR_VALUE_STRING )
    {
        Com_Error( ERR_DROP, "idCVarSystemLocal::Update: src %s length %lu exceeds MAX_CVAR_VALUE_STRING(%lu)",
                   cv->string, ( U64 )strlen( cv->string ), ( U64 )sizeof( vmCvar->string ) );
    }
    // bk001212 - Q_strncpyz guarantees zero padding and dest[MAX_CVAR_VALUE_STRING-1]==0
    // bk001129 - paranoia. Never trust the destination string.
    // bk001129 - beware, sizeof(UTF8*) is always 4 (for cv->string).
    //            sizeof(vmCvar->string) always MAX_CVAR_VALUE_STRING
    //Q_strncpyz( vmCvar->string, cv->string, sizeof( vmCvar->string ) ); // id
    Q_strncpyz( vmCvar->string, cv->string, MAX_CVAR_VALUE_STRING );
    
    vmCvar->value = cv->value;
    vmCvar->integer = cv->integer;
}

/*
==================
idCVarSystemLocal::CompleteCvarName
==================
*/
void idCVarSystemLocal::CompleteCvarName( UTF8* args, S32 argNum )
{
    if( argNum == 2 )
    {
        // Skip "<cmd> "
        UTF8* p = Com_SkipTokens( args, 1, " " );
        
        if( p > args )
        {
            Field_CompleteCommand( p, false, true );
        }
    }
}

/*
============
idCVarSystemLocal::Init

Reads in all archived cvars
============
*/
void idCVarSystemLocal::Init( void )
{
    cvar_cheats = Get( "sv_cheats", "1", CVAR_ROM | CVAR_SYSTEMINFO );
    
    Cmd_AddCommand( "toggle", &idCVarSystemLocal::Toggle_f );
    Cmd_SetCommandCompletionFunc( "toggle", &idCVarSystemLocal::CompleteCvarName );
    Cmd_AddCommand( "cycle", &idCVarSystemLocal::Cycle_f );	// ydnar
    Cmd_SetCommandCompletionFunc( "cycle", &idCVarSystemLocal::CompleteCvarName );
    Cmd_AddCommand( "set", &idCVarSystemLocal::Set_f );
    Cmd_SetCommandCompletionFunc( "set", &idCVarSystemLocal::CompleteCvarName );
    Cmd_AddCommand( "sets", &idCVarSystemLocal::SetS_f );
    Cmd_SetCommandCompletionFunc( "sets", &idCVarSystemLocal::CompleteCvarName );
    Cmd_AddCommand( "setu", &idCVarSystemLocal::SetU_f );
    Cmd_SetCommandCompletionFunc( "setu", &idCVarSystemLocal::CompleteCvarName );
    Cmd_AddCommand( "seta", &idCVarSystemLocal::SetA_f );
    Cmd_SetCommandCompletionFunc( "seta", &idCVarSystemLocal::CompleteCvarName );
    Cmd_AddCommand( "reset", &idCVarSystemLocal::Reset_f );
    Cmd_SetCommandCompletionFunc( "reset", &cvarSystemLocal.CompleteCvarName );
    Cmd_AddCommand( "cvarlist", &cvarSystemLocal.List_f );
    Cmd_AddCommand( "cvar_restart", &cvarSystemLocal.Restart_f );
    
    // NERVE - SMF - can't rely on autoexec to do this
    Get( "devdll", "1", CVAR_ROM );
}
