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
// File name:   GPUWorker_Program.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

/*
========================
owGPUWorkerProgram::LoadWorkerProgram
========================
*/
void owGPUWorkerProgram::LoadWorkerProgram( StringEntry qpath )
{
    StringEntry buffer;
    U64 bufferSize = 0;
    cl_int deviceError = 0;
    cl_device_id deviceId;
    
    deviceId = gpuWorkerLocal.GetDeviceId();
    
    idStr fullPath = va( "%s/%s%s", GPUWORKER_FOLDER, qpath, GPUWORKER_EXT );
    
    // Set the name of the program as the program path.
    name = qpath;
    
    // Read the worker program into memory.
    Com_Printf( "Loading GPU Worker Program: %s\n", fullPath.c_str() );
    bufferSize = FS_ReadFile( fullPath.c_str(), ( void** )&buffer );
    if( bufferSize <= 0 )
    {
        Com_Error( PRINT_ALL, "Failed to load GPU worker program %s\n", fullPath.c_str() );
        return;
    }
    
    Com_Printf( "...Program Size %d\n", bufferSize );
    
    // Create the program from the file buffer.
    deviceHandle = clCreateProgramWithSource( gpuWorkerLocal.GetDeviceContext(), 1, &buffer, &bufferSize, &deviceError );
    
    // Check to see if there was a error creating the program.
    if( deviceHandle == NULL || ID_GPUWORKER_HASERROR )
    {
        Com_Printf( "Failed to create GPU worker program %s -- Error %s\n", fullPath.c_str(), clErrorString( owGpuWorkerLocal::clError ) );
        return;
    }
    
    // Build the program -- this is were it does all the compilation(syntax checking, etc), and linking.
    deviceError = clBuildProgram( ( cl_program )deviceHandle, 1, &deviceId, NULL, NULL, NULL );
    
    if( ID_GPUWORKER_HASERROR )
    {
        static UTF8* deviceErrorLog;
        U64 log_size;
        
        // Get the proper size of the device error log.
        clGetProgramBuildInfo( ( cl_program )deviceHandle, deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size );
        
        // Allocate the log buffer, and get the error log from the device.
        deviceErrorLog = new UTF8[log_size + 1];
        clGetProgramBuildInfo( ( cl_program )deviceHandle, deviceId, CL_PROGRAM_BUILD_LOG, log_size, deviceErrorLog, NULL );
        
        idStr deviceLogTemp = deviceErrorLog;
        delete[] deviceErrorLog;
        
        Com_Printf( "GPU Worker Program(%s) Compile Error %s\n", fullPath.c_str(), deviceLogTemp.c_str() );
    }
}

/*
========================
owGPUWorkerProgram::UploadMemory
Uploads a block of memory to the graphics card.
========================
*/
void owGPUWorkerProgram::UploadMemory( gpuWorkerMemoryPtr_t memhandle, void* data, S32 size )
{
    // Force blocking right now.
    if( clEnqueueWriteBuffer( gpuWorkerLocal.GetCommandQueue(), ( cl_mem )memhandle, true, 0, size, data, NULL, NULL, NULL ) <= 0 )
    {
        Com_Error( PRINT_ALL, "Uploading GPU Memory block failed.\n" );
    }
}

//Dushan
/*
========================
owGPUWorkerProgram::ReadMemory
Reads a block of memory from the graphics card.
========================
*/
void owGPUWorkerProgram::ReadMemory( gpuWorkerMemoryPtr_t memhandle, void* data, S32 size )
{
    // Force blocking right now.
    if( clEnqueueReadBuffer( gpuWorkerLocal.GetCommandQueue(), ( cl_mem )memhandle, true, 0, size, data, NULL, NULL, NULL ) <= 0 )
    {
        Com_Error( PRINT_ALL, "Downloading GPU Memory block failed.\n" );
    }
}

/*
========================
owGPUWorkerProgram::CreateKernel
========================
*/
gpuWorkerKernelHandle_t	owGPUWorkerProgram::CreateKernel( StringEntry kernelName )
{
    cl_kernel kernel;
    
    kernel = clCreateKernel( ( cl_program )deviceHandle, kernelName, &owGpuWorkerLocal::clError );
    if( kernel == NULL || ID_GPUWORKER_HASERROR )
    {
        Com_Error( PRINT_ALL, "Failed to create kernel %s\n", kernelName );
    }
    
    return ( gpuWorkerKernelHandle_t )kernel;
}


/*
========================
owGPUWorkerProgram::CreateSharedTextureHandle
Creates a shared texture handle.
========================
*/
gpuWorkerMemoryPtr_t owGPUWorkerProgram::CreateSharedTextureHandle( image_t* image )
{
    cl_mem ptr;
    
    // Create the shared gl texture2d object.
    ptr = clCreateFromGLTexture2D( gpuWorkerLocal.GetDeviceContext(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, NULL, &owGpuWorkerLocal::clError );
    
    if( ptr == NULL || ID_GPUWORKER_HASERROR )
    {
        CL_RefPrintf( PRINT_ALL, "CreateSharedTextureHandle: Failed to create shared handle from image %s\n", owGPUWorkerProgram::Name() );
        return NULL;
    }
    
    return ( gpuWorkerMemoryPtr_t )ptr;
}

/*
========================
owGPUWorkerProgram::Free
========================
*/
void owGPUWorkerProgram::Free( void )
{

}
