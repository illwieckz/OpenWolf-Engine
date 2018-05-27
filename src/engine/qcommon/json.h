////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2016 James Canete
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
// File name:   json.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __JSON_H__
#define __JSON_H__

enum
{
    JSONTYPE_STRING, // string
    JSONTYPE_OBJECT, // object
    JSONTYPE_ARRAY,  // array
    JSONTYPE_VALUE,  // number, true, false, or null
    JSONTYPE_ERROR   // out of data
};

// --------------------------------------------------------------------------
//   Array Functions
// --------------------------------------------------------------------------

// Get pointer to first value in array
// When given pointer to an array, returns pointer to the first
// returns NULL if array is empty or not an array.
StringEntry JSON_ArrayGetFirstValue( StringEntry json, StringEntry jsonEnd );

// Get pointer to next value in array
// When given pointer to a value, returns pointer to the next value
// returns NULL when no next value.
StringEntry JSON_ArrayGetNextValue( StringEntry json, StringEntry jsonEnd );

// Get pointers to values in an array
// returns 0 if not an array, array is empty, or out of data
// returns number of values in the array and copies into index if successful
U32 JSON_ArrayGetIndex( StringEntry json, StringEntry jsonEnd, StringEntry* indexes, U32 numIndexes );

// Get pointer to indexed value from array
// returns NULL if not an array, no index, or out of data
StringEntry JSON_ArrayGetValue( StringEntry json, StringEntry jsonEnd, U32 index );

// --------------------------------------------------------------------------
//   Object Functions
// --------------------------------------------------------------------------

// Get pointer to named value from object
// returns NULL if not an object, name not found, or out of data
StringEntry JSON_ObjectGetNamedValue( StringEntry json, StringEntry jsonEnd, StringEntry name );

// --------------------------------------------------------------------------
//   Value Functions
// --------------------------------------------------------------------------

// Get type of value
// returns JSONTYPE_ERROR if out of data
U32 JSON_ValueGetType( StringEntry json, StringEntry jsonEnd );

// Get value as string
// returns 0 if out of data
// returns length and copies into string if successful, including terminating nul.
// string values are stripped of enclosing quotes but not escaped
U32 JSON_ValueGetString( StringEntry json, StringEntry jsonEnd, UTF8* outString, U32 stringLen );

// Get value as appropriate type
// returns 0 if value is false, value is null, or out of data
// returns 1 if value is true
// returns value otherwise
F64 JSON_ValueGetDouble( StringEntry json, StringEntry jsonEnd );
F32 JSON_ValueGetFloat( StringEntry json, StringEntry jsonEnd );
S32 JSON_ValueGetInt( StringEntry json, StringEntry jsonEnd );

#endif

#ifdef JSON_IMPLEMENTATION
#include <stdio.h>

// --------------------------------------------------------------------------
//   Internal Functions
// --------------------------------------------------------------------------

static StringEntry JSON_SkipSeparators( StringEntry json, StringEntry jsonEnd );
static StringEntry JSON_SkipString( StringEntry json, StringEntry jsonEnd );
static StringEntry JSON_SkipStruct( StringEntry json, StringEntry jsonEnd );
static StringEntry JSON_SkipValue( StringEntry json, StringEntry jsonEnd );
static StringEntry JSON_SkipValueAndSeparators( StringEntry json, StringEntry jsonEnd );

#define IS_SEPARATOR(x)    ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == '\r' || (x) == ',' || (x) == ':')
#define IS_STRUCT_OPEN(x)  ((x) == '{' || (x) == '[')
#define IS_STRUCT_CLOSE(x) ((x) == '}' || (x) == ']')

static StringEntry JSON_SkipSeparators( StringEntry json, StringEntry jsonEnd )
{
    while( json < jsonEnd && IS_SEPARATOR( *json ) )
        json++;
        
    return json;
}

static StringEntry JSON_SkipString( StringEntry json, StringEntry jsonEnd )
{
    for( json++; json < jsonEnd && *json != '"'; json++ )
        if( *json == '\\' )
            json++;
            
    return ( json + 1 > jsonEnd ) ? jsonEnd : json + 1;
}

static StringEntry JSON_SkipStruct( StringEntry json, StringEntry jsonEnd )
{
    json = JSON_SkipSeparators( json + 1, jsonEnd );
    while( json < jsonEnd && !IS_STRUCT_CLOSE( *json ) )
        json = JSON_SkipValueAndSeparators( json, jsonEnd );
        
    return ( json + 1 > jsonEnd ) ? jsonEnd : json + 1;
}

static StringEntry JSON_SkipValue( StringEntry json, StringEntry jsonEnd )
{
    if( json >= jsonEnd )
        return jsonEnd;
    else if( *json == '"' )
        json = JSON_SkipString( json, jsonEnd );
    else if( IS_STRUCT_OPEN( *json ) )
        json = JSON_SkipStruct( json, jsonEnd );
    else
    {
        while( json < jsonEnd && !IS_SEPARATOR( *json ) && !IS_STRUCT_CLOSE( *json ) )
            json++;
    }
    
    return json;
}

static StringEntry JSON_SkipValueAndSeparators( StringEntry json, StringEntry jsonEnd )
{
    json = JSON_SkipValue( json, jsonEnd );
    return JSON_SkipSeparators( json, jsonEnd );
}

// returns 0 if value requires more parsing, 1 if no more data/false/null, 2 if true
static U32 JSON_NoParse( StringEntry json, StringEntry jsonEnd )
{
    if( !json || json >= jsonEnd || *json == 'f' || *json == 'n' )
        return 1;
        
    if( *json == 't' )
        return 2;
        
    return 0;
}

// --------------------------------------------------------------------------
//   Array Functions
// --------------------------------------------------------------------------

StringEntry JSON_ArrayGetFirstValue( StringEntry json, StringEntry jsonEnd )
{
    if( !json || json >= jsonEnd || !IS_STRUCT_OPEN( *json ) )
        return NULL;
        
    json = JSON_SkipSeparators( json + 1, jsonEnd );
    
    return ( json >= jsonEnd || IS_STRUCT_CLOSE( *json ) ) ? NULL : json;
}

StringEntry JSON_ArrayGetNextValue( StringEntry json, StringEntry jsonEnd )
{
    if( !json || json >= jsonEnd || IS_STRUCT_CLOSE( *json ) )
        return NULL;
        
    json = JSON_SkipValueAndSeparators( json, jsonEnd );
    
    return ( json >= jsonEnd || IS_STRUCT_CLOSE( *json ) ) ? NULL : json;
}

U32 JSON_ArrayGetIndex( StringEntry json, StringEntry jsonEnd, StringEntry* indexes, U32 numIndexes )
{
    U32 length = 0;
    
    for( json = JSON_ArrayGetFirstValue( json, jsonEnd ); json; json = JSON_ArrayGetNextValue( json, jsonEnd ) )
    {
        if( indexes && numIndexes )
        {
            *indexes++ = json;
            numIndexes--;
        }
        length++;
    }
    
    return length;
}

StringEntry JSON_ArrayGetValue( StringEntry json, StringEntry jsonEnd, U32 index )
{
    for( json = JSON_ArrayGetFirstValue( json, jsonEnd ); json && index; json = JSON_ArrayGetNextValue( json, jsonEnd ) )
        index--;
        
    return json;
}

// --------------------------------------------------------------------------
//   Object Functions
// --------------------------------------------------------------------------

StringEntry JSON_ObjectGetNamedValue( StringEntry json, StringEntry jsonEnd, StringEntry name )
{
    U32 nameLen = strlen( name );
    
    for( json = JSON_ArrayGetFirstValue( json, jsonEnd ); json; json = JSON_ArrayGetNextValue( json, jsonEnd ) )
    {
        if( *json == '"' )
        {
            StringEntry thisNameStart, *thisNameEnd;
            
            thisNameStart = json + 1;
            json = JSON_SkipString( json, jsonEnd );
            thisNameEnd = json - 1;
            json = JSON_SkipSeparators( json, jsonEnd );
            
            if( ( U32 )( thisNameEnd - thisNameStart ) == nameLen )
                if( strncmp( thisNameStart, name, nameLen ) == 0 )
                    return json;
        }
    }
    
    return NULL;
}

// --------------------------------------------------------------------------
//   Value Functions
// --------------------------------------------------------------------------

U32 JSON_ValueGetType( StringEntry json, StringEntry jsonEnd )
{
    if( !json || json >= jsonEnd )
        return JSONTYPE_ERROR;
    else if( *json == '"' )
        return JSONTYPE_STRING;
    else if( *json == '{' )
        return JSONTYPE_OBJECT;
    else if( *json == '[' )
        return JSONTYPE_ARRAY;
        
    return JSONTYPE_VALUE;
}

U32 JSON_ValueGetString( StringEntry json, StringEntry jsonEnd, UTF8* outString, U32 stringLen )
{
    StringEntry stringEnd, *stringStart;
    
    if( !json )
    {
        *outString = '\0';
        return 0;
    }
    
    stringStart = json;
    stringEnd = JSON_SkipValue( stringStart, jsonEnd );
    if( stringEnd >= jsonEnd )
    {
        *outString = '\0';
        return 0;
    }
    
    // skip enclosing quotes if they exist
    if( *stringStart == '"' )
        stringStart++;
        
    if( *( stringEnd - 1 ) == '"' )
        stringEnd--;
        
    stringLen--;
    if( stringLen > stringEnd - stringStart )
        stringLen = stringEnd - stringStart;
        
    json = stringStart;
    while( stringLen-- )
        *outString++ = *json++;
    *outString = '\0';
    
    return stringEnd - stringStart;
}

F64 JSON_ValueGetDouble( StringEntry json, StringEntry jsonEnd )
{
    UTF8 cValue[256];
    F64 dValue = 0.0;
    U32 np = JSON_NoParse( json, jsonEnd );
    
    if( np )
        return ( F64 )( np - 1 );
        
    if( !JSON_ValueGetString( json, jsonEnd, cValue, 256 ) )
        return 0.0;
        
    sscanf( cValue, "%lf", &dValue );
    
    return dValue;
}

F32 JSON_ValueGetFloat( StringEntry json, StringEntry jsonEnd )
{
    UTF8 cValue[256];
    F32 fValue = 0.0f;
    U32 np = JSON_NoParse( json, jsonEnd );
    
    if( np )
        return ( F32 )( np - 1 );
        
    if( !JSON_ValueGetString( json, jsonEnd, cValue, 256 ) )
        return 0.0f;
        
    sscanf( cValue, "%f", &fValue );
    
    return fValue;
}

S32 JSON_ValueGetInt( StringEntry json, StringEntry jsonEnd )
{
    UTF8 cValue[256];
    S32 iValue = 0;
    U32 np = JSON_NoParse( json, jsonEnd );
    
    if( np )
        return np - 1;
        
    if( !JSON_ValueGetString( json, jsonEnd, cValue, 256 ) )
        return 0;
        
    sscanf( cValue, "%d", &iValue );
    
    return iValue;
}

#undef IS_SEPARATOR
#undef IS_STRUCT_OPEN
#undef IS_STRUCT_CLOSE

#endif //!__JSON_H__
