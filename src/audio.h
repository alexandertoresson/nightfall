/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "vector3d-pre.h"

#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef DEBUG_DEP
#warning "audio.h"
#endif

#include "audio-pre.h"

#include "sdlheader.h"

#include "terrain.h"

#include <map>
#include <string>

#define AUDIO_DEFAULT_SIGNAL_STRENGTH 5.0f

namespace Audio
{
	// For the sake of simplicity
	typedef Mix_Music Music;
	typedef Mix_Chunk Sound;

	struct AudioList : gc_null_shader<AudioList>
	{
		std::vector<Music*>     ppMusic; 
		std::vector<Sound*>     ppSound; 
		PlayType    playType;
		AudioType   type;
		
		AudioList() : playType(AUDIO_PLAY_TYPE_UNKNOWN), type(AUDIO_UNKNOWN)
		{
			
		}

		~AudioList();
	};

	struct AudioFXInfo
	{
		union
		{
			Sound* pSound;
			Music* pMusic;
		};
		float strength;
		int channel;

		AudioFXInfo() : strength(AUDIO_DEFAULT_SIGNAL_STRENGTH), channel(-1) {}
	};

	class SoundNode
	{
	public:
		Sound* pSound;
		Utilities::Vector3D position;
		Utilities::Vector3D velocity;
		float strength;
		int times;
		int channel;

		gc_ptr<Game::Dimension::Unit> pSpeaker;

		SoundNode(Sound* p, const Utilities::Vector3D& position, const Utilities::Vector3D& velocity, float newStrength, int numPlay) :
			pSound(p), position(position), velocity(velocity), strength(newStrength), times(numPlay),
			channel(-1)
		{}

		~SoundNode(void)
		{}
	};
	
	// Private struct containing information
	// for the worker-thread.
	struct ThreadInfo
	{
		gc_ptr<AudioList> pList;
		unsigned   index;
		SDL_mutex* mutex;
		bool       threadRuntime;
		bool       switchSong;
		int        newIndex;
	};
	
	typedef std::map<std::string, gc_root_ptr<Audio::AudioList>::type > AudioStates;
 	typedef AudioStates::iterator                            AudioStatesIterator;
	typedef std::list<Audio::SoundNode>::iterator            SoundListNode;

	// Prepare the audio-engine. Does -a lot-. 
	// Starts up the beast, allocates channels,
	// and loads, if asked, audio and sound fx from
	// audio lists.
	int Init(std::string configFile);
	
	// Gracefully kills the audio-engine.
	int Kill(void);
	void DeallocList(void);
	
	void OnChannelFinish(void (*fptr)(int));
	void OnMusicFinish(void (*fptr)(void));
	
	gc_ptr<AudioList> GetAudioList(std::string tag);
	Sound* GetSound(std::string tag, unsigned index);
	
	int  PlayList(std::string tag, unsigned index = 0, int volume = -1);
	int  PlayList(gc_ptr<AudioList>, unsigned index = 0, int volume = -1, std::string tag = "");

	std::string PlayListCurrentlyPlaying(std::string set = "");
	bool        PlayListPlaying(std::string tag);
	
	int PlayOnce(std::string tag, int* channel, unsigned index = 0, int volume = -1);
	int PlayOnce(Sound*, int* channel);
	
	int PlayOnceFromLocation(std::string, int* channel, const Utilities::Vector3D& sound, const Utilities::Vector3D& camera, unsigned index = 0, float = AUDIO_DEFAULT_SIGNAL_STRENGTH);
	int PlayOnceFromLocation(Sound*, int* channel, const Utilities::Vector3D& sound, const Utilities::Vector3D& camera, float = AUDIO_DEFAULT_SIGNAL_STRENGTH);
	
	int GetMusicVolume(void);
	int GetChannelVolume(int);
	void SetMusicVolume(int);
	void SetChannelVolume(int channel, int);
	
	void StopList(void);
	void StopAll(void);
	
	bool _CreateThread(void);
	int  _ThreadMethod(void*);
	void _KillThread(void);

	SoundListNode CreateSoundNode(Sound*, const Utilities::Vector3D& position, const Utilities::Vector3D& velocity, float strength, int numPlay);
	void       RemoveSoundNode(SoundListNode);
	void       PlaceSoundNodes(const Utilities::Vector3D& observer);
	Uint8      CalculateVolume(const Utilities::Vector3D& vCamera, const Utilities::Vector3D& vPosition, float strength);
	void       SetSpeakerUnit(SoundListNode& pSoundListNode, const gc_ptr<Game::Dimension::Unit>& p);
}

#ifdef DEBUG_DEP
#warning "audio.h-end"
#endif

#endif
