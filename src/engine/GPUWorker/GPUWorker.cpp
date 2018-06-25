////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2010 id Software LLC, a ZeniMax Media company.
// Copyright(C) 2011 - 2012 Justin Marshall <justinmarshall20@gmail.com>
// Copyright(C) 2012 - 2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   GPUWorker.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

owGpuWorkerLocal		gpuWorkerLocal;
owGpuWorker*			gpuWorker = &gpuWorkerLocal;

cl_int					owGpuWorkerLocal::clError;

/*
=============
owGpuWorker::Init
=============
*/
void owGpuWorkerLocal::Init( void )
{
    Com_Printf( "----------- owGpuWorkerLocal::Init ----------\n" );
    Com_Printf( "Init OpenCL...\n" );
    Com_Printf( "...Getting Device Platform ID.\n" );
    ID_GPUWORKER_CALLAPI( clGetPlatformIDs( 1, &platform, NULL ) );
    
    // Get all the device ids
    Com_Printf( "...Getting Device ID.\n" );
    ID_GPUWORKER_CALLAPI( clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL ) );
    
    cl_context_properties properties[] =
    {
#ifdef _WIN32
        CL_GL_CONTEXT_KHR, ( cl_context_properties ) wglGetCurrentContext(),
        CL_WGL_HDC_KHR, ( cl_context_properties ) wglGetCurrentDC(),
#elif __LINUX__
        //Dushan - this should be for Linux
        CL_GL_CONTEXT_KHR, ( cl_context_properties ) glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR, ( cl_context_properties ) glXGetCurrentDisplay(),
#elif __ANDROID__
        //Dushan - also fot the Android
        CL_GL_CONTEXT_KHR, ( cl_context_properties ) eglGetCurrentDisplay(),
        CL_EGL_DISPLAY_KHR, ( cl_context_properties ) eglGetCurrentContext(),
#else
        //Dushan - I have no idea for Mac
#endif
        CL_CONTEXT_PLATFORM, ( cl_context_properties ) platform,
        0
    };
    
    // Create the OpenCL device context
    Com_Printf( "...Creating Device Context.\n" );
    context = clCreateContext( properties, 1, &device, NULL, NULL, &clError );
    
    // Create the opencl command queue.
    Com_Printf( "...Creating Device Command Queue.\n" );
    queue = clCreateCommandQueue( context, device, 0, &clError );
    
    Com_Printf( "---------------------------------------\n" );
}

/*
=============
owGpuWorkerLocal::FindProgram
=============
*/
owGPUWorkerProgram* owGpuWorkerLocal::FindProgram( StringEntry path )
{
    for( S32 i = 0; i < programPool.Num(); i++ )
    {
        if( !strcmp( programPool[i]->Name(), path ) )
        {
            return programPool[i];
        }
    }
    
    return NULL;
}

/*
=============
owGpuWorkerLocal::Shutdown
=============
*/
void owGpuWorkerLocal::Shutdown( void )
{
    Com_Printf( "----------- owGpuWorkerLocal::Shutdown ----------\n" );
    
    Com_Printf( "Clearing render program workers...\n" );
    for( S32 i = 0; i < programPool.Num(); i++ )
    {
        programPool[i]->Free();
        delete programPool[i];
    }
    
    programPool.Clear();
    
    Com_Printf( "Releasing command queue\n" );
    clReleaseCommandQueue( queue );
    
    Com_Printf( "Releasing OpenCL context\n" );
    clReleaseContext( context );
}
