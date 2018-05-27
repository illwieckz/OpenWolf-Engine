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
// File name:   l_memory.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __L_MEMORY_H__
#define __L_MEMORY_H__

//#define MEMDEBUG

#ifdef MEMDEBUG
#define GetMemory(size)				GetMemoryDebug(size, #size, __FILE__, __LINE__);
#define GetClearedMemory(size)		GetClearedMemoryDebug(size, #size, __FILE__, __LINE__);
//allocate a memory block of the given size
void* GetMemoryDebug( U64 size, UTF8* label, UTF8* file, S32 line );
//allocate a memory block of the given size and clear it
void* GetClearedMemoryDebug( U64 size, UTF8* label, UTF8* file, S32 line );
//
#define GetHunkMemory(size)			GetHunkMemoryDebug(size, #size, __FILE__, __LINE__);
#define GetClearedHunkMemory(size)	GetClearedHunkMemoryDebug(size, #size, __FILE__, __LINE__);
//allocate a memory block of the given size
void* GetHunkMemoryDebug( U64 size, UTF8* label, UTF8* file, S32 line );
//allocate a memory block of the given size and clear it
void* GetClearedHunkMemoryDebug( U64 size, UTF8* label, UTF8* file, S32 line );
#else
//allocate a memory block of the given size
void* GetMemory( U64 size );
//allocate a memory block of the given size and clear it
void* GetClearedMemory( U64 size );
//
#ifdef BSPC
#define GetHunkMemory GetMemory
#define GetClearedHunkMemory GetClearedMemory
#else
//allocate a memory block of the given size
void* GetHunkMemory( U64 size );
//allocate a memory block of the given size and clear it
void* GetClearedHunkMemory( U64 size );
#endif
#endif

//free the given memory block
void FreeMemory( void* ptr );
//returns the amount available memory
S32 AvailableMemory( void );
//prints the total used memory size
void PrintUsedMemorySize( void );
//print all memory blocks with label
void PrintMemoryLabels( void );
//returns the size of the memory block in bytes
S32 MemoryByteSize( void* ptr );
//free all allocated memory
void DumpMemory( void );

#endif //!__L_MEMORY_H__