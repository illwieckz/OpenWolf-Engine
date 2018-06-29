////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2011 - 2012 Justin Marshall <justinmarshall20@gmail.com>
// Copyright(C) 2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   GPUWorker_CLCache.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

/*
=============================
owCacheGPUWorker::LoadWorkerProgram
=============================
*/
void owCacheGPUWorker::LoadWorkerProgram( StringEntry path )
{
    S32 owCacheSize = 0;
    
    // Load the worker program.
    owGPUWorkerProgram::LoadWorkerProgram( path );
    
    owCacheSize = OW_MAXCHARTS * ( 64 * 64 );
    
    // Allocate the space needed for the cache on the gpu.
    CL_RefPrintf( PRINT_ALL, "HelloWorld GPU Cache Size %d bytes...\n", owCacheSize );
    
    owCreateBuffer = clCreateBuffer( gpuWorkerLocal.GetDeviceContext(), CL_MEM_READ_WRITE, owCacheSize, NULL, &owGpuWorkerLocal::clError );
    
    if( ID_GPUWORKER_HASERROR )
    {
        Com_Error( ERR_DROP, "Failed to allocate HelloWorld GPU cache...\n" );
    }
    
    uploadTileKernel = CreateKernel( "HelloWorld" );
}

/*
=============================
owCacheGPUWorker::CreateSharedTexturePage
=============================
*/
void owCacheGPUWorker::CreateSharedTexturePage( image_t* image )
{
    clPageAddrPool.Append( CreateSharedTextureHandle( image ) );
}

/*
=============================
owCacheGPUWorker::UploadCache
=============================
*/
void owCacheGPUWorker::UploadCache( void* buffer, S32 size )
{
    UploadMemory( UploadCLCache, buffer, size );
}
