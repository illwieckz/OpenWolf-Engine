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
// File name:   htable.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <qcommon/htable.h>

/*=============================================*
 * Type definitions                            *
 *=============================================*/

/*
 * "List heads" - used to store various lists
 */

struct listhead_t
{
    struct listhead_t* 	previous;
    struct listhead_t* 	next;
};


/*
 * Resets a list head
 */
#define RESET_LIST(LPTR) \
	( (LPTR)->previous = (LPTR)->next = (LPTR) )


/*
 * Table entry - used to store actual items
 */

struct tentry_t
{
    /* Entry in one of the table's sub-lists */
    struct listhead_t	loc_list;
    
    /* Entry in the table's main list */
    struct listhead_t	full_list;
    
    /* Cached hash value */
    U32		hash;
};


/*
 * Function pointers
 */

typedef U32( * getkey_t )( StringEntry key );
typedef UTF8* ( * keyfromentry_t )( struct tentry_t* entry , U64 key_offset );
typedef S32( * comparekey_t )( StringEntry k1 , StringEntry k2 );


/*
 * Main hash table structure
 */
struct hashtable_s
{
    /* Actual size of the table */
    U64			size;
    
    /* Table flags */
    U32		flags;
    
    /* Item size */
    U64			item_size;
    
    /* Key offset in an item */
    U64			key_offset;
    
    /* Length of key (0 for pointer) */
    U64			key_length;
    
    /* Functions */
    getkey_t		GetKey;
    keyfromentry_t		KeyFromEntry;
    comparekey_t		CompareKey;
    
    /* List of all items */
    struct listhead_t	all_items;
};


/*
 * Macro that finds the first list head after a table's main structure
 */
#define TABLE_START(TABLE) \
	( (struct listhead_t *)( ((UTF8*)(TABLE)) + sizeof( struct hashtable_s ) ) )


/*=============================================*
 * Internal functions prototypes               *
 *=============================================*/

/* Checks if a size is a prime number */
static bool _HT_IsPrime( U64 n );

/* Finds the next higher prime number */
static U64 _HT_NextPrime( U64 n );

/* Computes a string's hash key (case insensitive) */
static U32 _HT_GetCIKey( StringEntry key );

/* Computes a string's hash key (case sensitive) */
static U32 _HT_GetKey( StringEntry key );

/* Returns a table entry's key (in-table items, fixed size key) */
static UTF8* _HT_KeyFromEntryII( struct tentry_t* entry , U64 key_offset );

/* Returns a table entry's key (in-table items, pointer key) */
static UTF8* _HT_KeyFromEntryIP( struct tentry_t* entry , U64 key_offset );

/* Returns a table entry's key (external items, fixed size key) */
static UTF8* _HT_KeyFromEntryPI( struct tentry_t* entry , U64 key_offset );

/* Returns a table entry's key (external items, pointer key) */
static UTF8* _HT_KeyFromEntryPP( struct tentry_t* entry , U64 key_offset );

/* Allocate and initialise a table entry. */
static struct tentry_t* _HT_CreateEntry( hashtable_t table , U32 hash , struct listhead_t* list_entry , StringEntry key );

/* Insert a table entry into a table's global list */
static void _HT_InsertInGlobalList( hashtable_t table , struct tentry_t* t_entry , StringEntry key );


/*=============================================*
 * Hash table public functions                 *
 *=============================================*/


hashtable_t HT_Create(
    U64		size ,
    U32	flags ,
    U64		item_size ,
    U64		key_offset ,
    U64		key_length
)
{
    hashtable_t		table;
    U64			real_size;
    struct listhead_t* 	t_item;
    
    // Allocate table
    real_size = _HT_NextPrime( size );
    table = ( hashtable_t )Z_Malloc( sizeof( struct hashtable_s ) + real_size * sizeof( struct listhead_t ) );
    assert( table );
    
    // Initialise main table fields
    table->size = real_size;
    table->flags = flags;
    table->item_size = item_size;
    table->key_offset = key_offset;
    table->key_length = key_length;
    RESET_LIST( &table->all_items );
    
    // Set functions
    table->GetKey = ( flags & HT_FLAG_CASE ) ? _HT_GetKey : _HT_GetCIKey;
    table->CompareKey = ( flags & HT_FLAG_CASE ) ? strcmp : Q_strcasecmp;
    if( ( flags & HT_FLAG_INTABLE ) == 0 )
    {
        table->KeyFromEntry = key_length ? _HT_KeyFromEntryPI : _HT_KeyFromEntryPP;
    }
    else
    {
        table->KeyFromEntry = key_length ? _HT_KeyFromEntryII : _HT_KeyFromEntryIP;
    }
    
    // Initialise table entries
    t_item = TABLE_START( table );
    while( real_size > 0 )
    {
        RESET_LIST( t_item );
        t_item ++, real_size --;
    }
    
    return table;
}


void HT_Destroy(
    hashtable_t	table
)
{
    bool		del_key;
    struct listhead_t* 	list_head;
    struct listhead_t* 	list_entry;
    struct tentry_t* 	t_entry;
    
    del_key = ( bool )( table->key_length == 0 && ( table->flags & ( HT_FLAG_INTABLE | HT_FLAG_FREE ) ) != 0 );
    list_head = &( table->all_items );
    list_entry = list_head->next;
    while( list_entry != list_head )
    {
        t_entry = ( struct tentry_t* )( ( ( UTF8* ) list_entry ) - HT_OffsetOfField( struct tentry_t , full_list ) );
        list_entry = list_entry->next;
        
        if( del_key )
            Z_Free( table->KeyFromEntry( t_entry , table->key_offset ) );
        if( ( table->flags & ( HT_FLAG_INTABLE | HT_FLAG_FREE ) ) == HT_FLAG_FREE )
        {
            void** data = ( void** )( ( ( UTF8* ) t_entry ) + sizeof( struct tentry_t ) );
            Z_Free( *data );
        }
        Z_Free( t_entry );
    }
    Z_Free( table );
}


void* HT_GetItem(
    hashtable_t	table ,
    StringEntry 	key ,
    bool* 	created
)
{
    U32		hash;
    struct listhead_t* 	list_head;
    struct listhead_t* 	list_entry;
    struct tentry_t* 	t_entry;
    void* 			data;
    
    assert( table->key_length == 0 || table->key_length >= strlen( key ) );
    
    // Try finding the item
    hash = table->GetKey( key );
    list_head = ( TABLE_START( table ) + ( hash % table->size ) );
    list_entry = list_head->next;
    while( list_entry != list_head )
    {
        t_entry = ( struct tentry_t* ) list_entry;
        
        if( t_entry->hash > hash )
            break;
            
        if( t_entry->hash == hash )
        {
            UTF8* item_key = table->KeyFromEntry( t_entry , table->key_offset );
            if( ! table->CompareKey( key , item_key ) )
            {
                data = ( void* )( ( ( UTF8* )t_entry ) + sizeof( struct tentry_t ) );
                if( created != NULL )
                    *created = false;
                return ( table->flags & HT_FLAG_INTABLE ) ? data : ( *( void** )data );
            }
        }
        
        list_entry = list_entry->next;
    }
    
    // Check if we can create the entry
    if( created == NULL )
        return NULL;
        
    // Create entry
    *created = true;
    t_entry = _HT_CreateEntry( table , hash , list_entry , key );
    
    // Initialise data
    data = ( void* )( ( ( UTF8* )t_entry ) + sizeof( struct tentry_t ) );
    if( ( table->flags & HT_FLAG_INTABLE ) == 0 )
    {
        *( void** ) data = Z_Malloc( table->item_size );
        data = *( void** ) data;
    }
    memset( data , 0 , table->item_size );
    
    // Copy key
    if( table->key_length == 0 )
    {
        UTF8** key_ptr = ( UTF8** )( ( ( UTF8* ) data ) + table->key_offset );
        *key_ptr = ( UTF8* )Z_Malloc( strlen( key ) + 1 );
        strcpy( *key_ptr , key );
    }
    else
    {
        UTF8* key_ptr = ( ( UTF8* ) data ) + table->key_offset;
        strcpy( key_ptr , key );
    }
    
    return data;
}


void* HT_PutItem(
    hashtable_t	table ,
    void* 		item ,
    bool	allow_replacement
)
{
    void* 			ret_val = NULL;
    void* 			prev_entry = NULL;
    StringEntry 		insert_key;
    U32		hash;
    struct listhead_t* 	list_head;
    struct listhead_t* 	list_entry;
    struct tentry_t* 	t_entry;
    
    // Extract item key
    if( table->key_length )
    {
        insert_key = ( ( StringEntry ) item ) + table->key_offset;
    }
    else
    {
        insert_key = *( StringEntry* )( ( ( UTF8* ) item ) + table->key_offset );
    }
    
    // Try finding an item with that key, or the new item's location
    hash = table->GetKey( insert_key );
    list_head = ( TABLE_START( table ) + ( hash % table->size ) );
    list_entry = list_head->next;
    while( list_entry != list_head )
    {
        t_entry = ( struct tentry_t* ) list_entry;
        
        if( t_entry->hash > hash )
            break;
            
        if( t_entry->hash == hash )
        {
            StringEntry item_key = table->KeyFromEntry( t_entry , table->key_offset );
            S32 cres = table->CompareKey( insert_key , item_key );
            if( ! cres )
            {
                prev_entry = ( ( ( UTF8* )t_entry ) + sizeof( struct tentry_t ) );
                ret_val = ( table->flags & HT_FLAG_INTABLE ) ? prev_entry : ( *( void** )prev_entry );
                if( ! allow_replacement )
                    return ret_val;
                break;
            }
            else if( cres > 0 )
            {
                break;
            }
        }
        
        list_entry = list_entry->next;
    }
    
    if( ret_val != NULL )
    {
        // Delete previous item's key if it was a pointer and either
        // items are in-table or should be freed automatically
        if( table->key_length == 0 && ( table->flags & ( HT_FLAG_INTABLE | HT_FLAG_FREE ) ) != 0 )
            Z_Free( table->KeyFromEntry( t_entry , table->key_offset ) );
            
        if( ( table->flags & HT_FLAG_INTABLE ) != 0 )
        {
            // Copy item data
            memcpy( prev_entry , item , table->item_size );
            ret_val = NULL;
        }
        else
        {
            if( ( table->flags & HT_FLAG_FREE ) != 0 )
            {
                // Free previous item
                Z_Free( ret_val );
                ret_val = NULL;
            }
            *( void** ) prev_entry = item;
        }
    }
    else
    {
        void* data;
        
        t_entry = _HT_CreateEntry( table , hash , list_entry , insert_key );
        data = ( void* )( ( ( UTF8* )t_entry ) + sizeof( struct tentry_t ) );
        if( ( table->flags & HT_FLAG_INTABLE ) != 0 )
        {
            memcpy( data , item , table->item_size );
        }
        else
        {
            *( void** ) data = item;
        }
    }
    
    return ret_val;
}


bool HT_DeleteItem(
    hashtable_t	table ,
    StringEntry 	key ,
    void** 		found
)
{
    U32		hash;
    struct listhead_t* 	list_head;
    struct listhead_t* 	list_entry;
    struct tentry_t* 	t_entry;
    void* 			data = NULL;
    
    // Try finding the item
    hash = table->GetKey( key );
    list_head = ( TABLE_START( table ) + ( hash % table->size ) );
    list_entry = list_head->next;
    while( list_entry != list_head )
    {
        t_entry = ( struct tentry_t* ) list_entry;
        
        if( t_entry->hash > hash )
            break;
            
        if( t_entry->hash == hash )
        {
            UTF8* item_key = table->KeyFromEntry( t_entry , table->key_offset );
            if( ! table->CompareKey( key , item_key ) )
            {
                data = ( void* )( ( ( UTF8* )t_entry ) + sizeof( struct tentry_t ) );
                data = ( table->flags & HT_FLAG_INTABLE ) ? data : ( *( void** )data );
                break;
            }
        }
        
        list_entry = list_entry->next;
    }
    
    // Did we find it?
    if( data == NULL )
    {
        if( found != NULL )
            *found = NULL;
        return false;
    }
    
    // Detach it from its lists
    t_entry->loc_list.previous->next = t_entry->loc_list.next;
    t_entry->loc_list.next->previous = t_entry->loc_list.previous;
    t_entry->full_list.previous->next = t_entry->full_list.next;
    t_entry->full_list.next->previous = t_entry->full_list.previous;
    
    // Delete key
    if( table->key_length == 0 && ( table->flags & ( HT_FLAG_INTABLE | HT_FLAG_FREE ) ) != 0 )
        Z_Free( table->KeyFromEntry( t_entry , table->key_offset ) );
        
    // Delete item
    if( ( table->flags & ( HT_FLAG_INTABLE | HT_FLAG_FREE ) ) == HT_FLAG_FREE )
        Z_Free( data );
        
    // Delete entry
    Z_Free( t_entry );
    
    // Set found pointer
    if( found != NULL )
    {
        if( ( table->flags & ( HT_FLAG_INTABLE | HT_FLAG_FREE ) ) != 0 )
            data = NULL;
        *found = data;
    }
    
    return true;
}


void HT_Apply(
    hashtable_t	table ,
    ht_apply_funct	function ,
    void* 		data
)
{
    struct listhead_t* 	list_head;
    struct listhead_t* 	list_entry;
    
    list_head = &( table->all_items );
    list_entry = list_head->next;
    while( list_entry != list_head )
    {
        void* item;
        item = ( ( UTF8* ) list_entry ) - HT_OffsetOfField( struct tentry_t , full_list ) + sizeof( struct tentry_t );
        list_entry = list_entry->next;
        
        if( ( table->flags & HT_FLAG_INTABLE ) == 0 )
            item = *( void** ) item;
        if( ! function( item , data ) )
            return;
    }
}


/*=============================================*
 * Functions related to prime numbers          *
 *=============================================*/

static bool _HT_IsPrime( U64 n )
{
    U64 temp;
    U64 nsq;
    U64 inc;
    
    if( n == 0 )
        return false;
        
    nsq = ceil( sqrt( ( F64 )n ) );
    for( inc = 1 , temp = 2 ; temp <= nsq ; temp += inc )
    {
        if( n % temp == 0 )
            return false;
        if( temp == 3 )
            inc = 2;
    }
    return true;
}


static U64 _HT_NextPrime( U64 n )
{
    U64 value = n;
    while( ! _HT_IsPrime( value ) )
        value ++;
    return value;
}



/*=============================================*
 * Key computation functions                   *
 *=============================================*/


static U32 _HT_GetCIKey( StringEntry key )
{
    StringEntry current = key;
    U32 hash = 111119;
    
    while( *current )
    {
        hash += ( U8 )tolower( *current );
        hash += ( hash << 10 );
        hash ^= ( hash >> 6 );
        current ++;
    }
    
    hash += ( hash << 3 );
    hash ^= ( hash >> 11 );
    hash += ( hash << 15 );
    
    return hash;
}


static U32 _HT_GetKey( StringEntry key )
{
    StringEntry current = key;
    U32 hash = 111119;
    
    while( *current )
    {
        hash += ( U8 ) * current;
        hash += ( hash << 10 );
        hash ^= ( hash >> 6 );
        current ++;
    }
    
    hash += ( hash << 3 );
    hash ^= ( hash >> 11 );
    hash += ( hash << 15 );
    
    return hash;
}



/*=============================================*
 * Key retrieval                               *
 *=============================================*/


static UTF8* _HT_KeyFromEntryII( struct tentry_t* entry , U64 key_offset )
{
    void* item_addr;
    item_addr = ( void* )( ( ( UTF8* )entry ) + sizeof( struct tentry_t ) );
    return ( UTF8* )( ( ( UTF8* )item_addr ) + key_offset );
}

static UTF8* _HT_KeyFromEntryIP( struct tentry_t* entry , U64 key_offset )
{
    void* item_addr;
    item_addr = ( void* )( ( ( UTF8* )entry ) + sizeof( struct tentry_t ) );
    return *( UTF8** )( ( ( UTF8* )item_addr ) + key_offset );
}

static UTF8* _HT_KeyFromEntryPI( struct tentry_t* entry , U64 key_offset )
{
    void* item_addr;
    item_addr = *( void** )( ( ( UTF8* )entry ) + sizeof( struct tentry_t ) );
    return ( UTF8* )( ( ( UTF8* )item_addr ) + key_offset );
}

static UTF8* _HT_KeyFromEntryPP( struct tentry_t* entry , U64 key_offset )
{
    void* item_addr;
    item_addr = *( void** )( ( ( UTF8* )entry ) + sizeof( struct tentry_t ) );
    return *( UTF8** )( ( ( UTF8* )item_addr ) + key_offset );
}



/*=============================================*
 * Other internal functions                    *
 *=============================================*/


static struct tentry_t* _HT_CreateEntry(
    hashtable_t		table ,
    U32		hash ,
    struct listhead_t* 	list_entry ,
    StringEntry 		key )
{
    // Allocate new entry
    struct tentry_t* t_entry;
    U64	entry_size = sizeof( struct tentry_t );
    entry_size += ( table->flags& HT_FLAG_INTABLE ) ? table->item_size : sizeof( void* );
    t_entry = ( tentry_t* )Z_Malloc( entry_size );
    t_entry->hash = hash;
    
    // Add entry to local list
    t_entry->loc_list.previous = list_entry->previous;
    t_entry->loc_list.next = list_entry;
    list_entry->previous = t_entry->loc_list.previous->next = &( t_entry->loc_list );
    
    _HT_InsertInGlobalList( table , t_entry , key );
    
    return t_entry;
}


static void _HT_InsertInGlobalList( hashtable_t table , struct tentry_t* t_entry , StringEntry key )
{
    if( ( table->flags & HT_FLAG_SORTED ) == 0 )
    {
        // Append to global list
        t_entry->full_list.previous = table->all_items.previous;
        t_entry->full_list.next = &( table->all_items );
        table->all_items.previous = t_entry->full_list.previous->next = &( t_entry->full_list );
    }
    else
    {
        // Global list must be kept sorted, find insert location
        struct listhead_t* list_entry = table->all_items.next;
        while( list_entry != &( table->all_items ) )
        {
            struct tentry_t* ai_entry;
            StringEntry ai_key;
            S32 cres;
            
            ai_entry = ( struct tentry_t* )( ( ( UTF8* ) list_entry ) - HT_OffsetOfField( struct tentry_t , full_list ) );
            ai_key = table->KeyFromEntry( ai_entry , table->key_offset );
            cres = table->CompareKey( ai_key , key );
            
            assert( cres != 0 );
            if( cres > 0 )
                break;
            list_entry = list_entry->next;
        }
        t_entry->full_list.previous = list_entry->previous;
        t_entry->full_list.next = list_entry;
        list_entry->previous = t_entry->full_list.previous->next = &( t_entry->full_list );
    }
}
