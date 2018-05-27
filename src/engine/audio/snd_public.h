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
// File name:   snd_public.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __SND_PUBLIC_H__
#define __SND_PUBLIC_H__

//
// idSoundSystem
//
class idSoundSystem
{
public:
    virtual void Init( void ) = 0;
    virtual void Shutdown( void ) = 0;
    virtual void UpdateThread( void ) = 0;
    virtual void StartSound( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfxHandle, S32 volume ) = 0;
    virtual void StartSoundEx( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfxHandle, S32 flags, S32 volume ) = 0;
    virtual void StartLocalSound( sfxHandle_t sfxHandle, S32 channelNum, S32 volume ) = 0;
    virtual void StartBackgroundTrack( StringEntry intro, StringEntry loop, S32 fadeupTime ) = 0;
    virtual void StopBackgroundTrack( void ) = 0;
    virtual void FadeStreamingSound( F32 targetvol, S32 time, S32 ssNum ) = 0;
    virtual void FadeAllSounds( F32 targetVol, S32 time, bool stopsounds ) = 0;
    virtual F32 StartStreamingSound( StringEntry intro, StringEntry loop, S32 entnum, S32 channel, S32 attenuation ) = 0;
    virtual void StopStreamingSound( S32 index ) = 0;
    virtual void StopEntStreamingSound( S32 entNum ) = 0; //----(SA)	added
    
    // cinematics and voice-over-network will send raw samples
    // 1.0 volume will be direct output of source samples
    virtual void RawSamples( S32 samples, S32 rate, S32 width, S32 channels, const U8* data, F32 lvol, F32 rvol, S32 streamingIndex ) = 0;
    
    // stop all sounds and the background track
    virtual void StopAllSounds( void ) = 0;
    
    // all continuous looping sounds must be added before calling Update
    virtual void ClearLoopingSounds( void ) = 0;
    virtual void ClearSounds( bool clearStreaming, bool clearMusic ) = 0;
    virtual void AddLoopingSound( const vec3_t origin, const vec3_t velocity, const S32 range, sfxHandle_t sfxHandle, S32 volume, S32 soundTime ) = 0;
    virtual void AddRealLoopingSound( const vec3_t origin, const vec3_t velocity, const S32 range, sfxHandle_t sfxHandle, S32 volume, S32 soundTime ) = 0;
    virtual S32 GetSoundLength( sfxHandle_t sfxHandle ) = 0;
    virtual S32 GetCurrentSoundTime( void ) = 0;
    
    // recompute the reletive volumes for all running sounds
    // reletive to the given entityNum / orientation
    virtual void Respatialize( S32 entityNum, const vec3_t origin, vec3_t axis[3], S32 inwater ) = 0;
    
    // let the sound system know where an entity currently is
    virtual void UpdateEntityPosition( S32 entityNum, const vec3_t origin ) = 0;
    
    virtual void Update( void ) = 0;
    virtual void DisableSounds( void ) = 0;
    virtual void BeginRegistration( void ) = 0;
    
    // RegisterSound will allways return a valid sample, even if it
    // has to create a placeholder.  This prevents continuous filesystem
    // checks for missing files
    virtual sfxHandle_t RegisterSound( StringEntry name, bool compressed ) = 0;
    virtual void DisplayFreeMemory( void ) = 0;
    virtual S32 GetVoiceAmplitude( S32 entityNum ) = 0;
    virtual void Reload( void ) = 0;
    virtual void StopLoopingSound( S32 entityNum ) = 0;
    virtual S32 SoundDuration( sfxHandle_t handle ) = 0;
};

extern idSoundSystem* soundSystem;

#endif // !__SND_PUBLIC_H_

