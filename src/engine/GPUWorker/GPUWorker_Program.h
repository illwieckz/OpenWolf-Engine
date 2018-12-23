////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2010 id Software LLC, a ZeniMax Media company.
// Copyright(C) 2011 - 2012 Justin Marshall <justinmarshall20@gmail.com>
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
// File name:   GPUWorkerProgram.h
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __GPUWORKER_PROGGRAM_H__
#define __GPUWORKER_PROGGRAM_H__

#ifndef __R_LOCAL_H__
#include <GPURenderer/r_local.h>
#endif
#ifndef __UTIL_STR_H__
#include <OWLib/util_str.h>
#endif
#ifndef __CL_PLATFORM_H
#include <cl/cl_platform.h>
#endif

#define GPUWORKER_FOLDER "renderWorkerProgs"
#define GPUWORKER_EXT ".opencl"

// ----------------------------------
typedef void* gpuWorkerProgramHandle_t;
typedef void* gpuWorkerMemoryPtr_t;
typedef void* gpuWorkerKernelHandle_t;
// ----------------------------------

//
// owGPUWorkerProgram
//
class owGPUWorkerProgram
{
public:
    // Loads in the worker program.
    virtual void					LoadWorkerProgram( StringEntry path );
    // Frees the render program handles.
    virtual void					Free( void );
    // Returns the name of the current worker program.
    StringEntry 					Name()
    {
        return name.c_str();
    };
    // strings in case of the error.
#ifndef DEDICATED
    StringEntry                     clErrorString( cl_int err );
#endif
protected:
    gpuWorkerProgramHandle_t		deviceHandle;		// Handle to the gpu program device handle.
    
    // Uploads memory to the GPU.
    virtual void					UploadMemory( gpuWorkerMemoryPtr_t memhandle, void* data, S32 size );
    
    // Read memory from the GPU.
    virtual void                    ReadMemory( gpuWorkerMemoryPtr_t memhandle, void* data, S32 size );
    
    // Creates a kernel handle(a kernel a function were going to execute at some point).
    virtual gpuWorkerKernelHandle_t	CreateKernel( StringEntry kernelName );
    virtual gpuWorkerMemoryPtr_t    CreateSharedTextureHandle( image_t* image );
    
private:
    idStr							name;
};

#endif // !__GPUWORKER_PROGGRAM_H__
