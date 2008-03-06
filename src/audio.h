//
// I AM WORKING ON THIS FILE AT THE MOMENT
// Please do not, and never, consider this the final version! :)
//
//
#include "vector3d-pre.h"

#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef DEBUG_DEP
#warning "audio.h"
#endif

#include "sdlheader.h"

#include "terrain.h"
#include "linkedlist.h"

#include <map>
#include <string>

#define AUDIO_DEFAULT_SIGNAL_STRENGTH 5.0f

namespace Audio
{
	// For the sake of simplicity
	typedef Mix_Music Music;
	typedef Mix_Chunk Sound;

	// Either you loop, or not.
	enum PlayType
	{
		AUDIO_PLAY_TYPE_UNKNOWN = -1,
		AUDIO_PLAY_TYPE_LOOP    =  0,
		AUDIO_PLAY_TYPE_ONCE    =  1
	};
	
	enum AudioType
	{
		AUDIO_UNKNOWN = -1,
		AUDIO_MUSIC   =  0,
		AUDIO_SOUND   =  1
	};

	static const int SFX_ACT_COUNT = 5;
	enum SoundNodeAction
	{
		SFX_ACT_FIRE_FNF         = 0,
		SFX_ACT_DEATH_FNF        = 1,
		SFX_ACT_MOVE_RPT         = 2,
		SFX_ACT_MOVE_DONE_FNF    = 3,
		SFX_ACT_IDLE_RPT         = 4,
		SFX_ACT_FOCUS_FNF        = 5
	};

	struct AudioList
	{
		union
		{
			Music**     ppMusic; 
			Sound**     ppSound; 
		};
		int         length;
		PlayType    playType;
		AudioType   type;    
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
		float x, y;
		float dx, dy;
		float strength;
		int times;
		int channel;

		Game::Dimension::Unit* pSpeaker;

		SoundNode(Sound* p, float newX, float newY, float newDx, float newDy, float newStrength, int numPlay) :
			pSound(p), x(newX), y(newY), dx(newDx), dy(newDy), strength(newStrength), times(numPlay),
				channel(-1)
		{}

		~SoundNode(void)
		{}
	};
	
	// Private struct containing information
	// for the worker-thread.
	struct ThreadInfo
	{
		AudioList* pList;
		int        index;
		SDL_mutex* mutex;
		bool       threadRuntime;
		bool       switchSong;
		int        newIndex;
	};
	
	typedef std::map<std::string, Audio::AudioList*>  AudioStates;
 	typedef AudioStates::iterator                     AudioStatesIterator;
	typedef Utilities::Node<Audio::SoundNode*>        SoundListNode;

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
	
	void   GetAudioList(std::string tag, AudioList*&);
	Sound* GetSound(std::string tag, int index);
	
	int  PlayList(std::string tag, int index = 0, int volume = -1);
	int  PlayList(AudioList*, int index = 0, int volume = -1, const char* tag = "");

	const char* PlayListCurrentlyPlaying(const char* set = "");
	bool        PlayListPlaying(const char* tag);
	
	int PlayOnce(std::string tag, int* channel, int index = 0, int volume = -1);
	int PlayOnce(Sound*, int* channel);
	
	int PlayOnceFromLocation(std::string, int* channel, Utilities::Vector3D* sound, const Utilities::Vector3D* camera, int index = 0, float = AUDIO_DEFAULT_SIGNAL_STRENGTH);
	int PlayOnceFromLocation(Sound*, int* channel, Utilities::Vector3D* sound, const Utilities::Vector3D* camera, float = AUDIO_DEFAULT_SIGNAL_STRENGTH);
	
	int GetMusicVolume(void);
	int GetChannelVolume(int);
	void SetMusicVolume(int);
	void SetChannelVolume(int channel, int);
	
	void StopList(void);
	void StopAll(void);
	
	bool _CreateThread(void);
	int  _ThreadMethod(void*);
	void _KillThread(void);

	SoundListNode* CreateSoundNode(Sound*, float x, float y, float dx, float dy, float strength, int numPlay);
	void       RemoveSoundNode(SoundListNode*);
	void       PlaceSoundNodes(const Utilities::Vector3D& observer);
	Uint8      CalculateVolume(const Utilities::Vector3D& vObserver, float x, float y, float strength);
	void       SetSpeakerUnit(SoundNode* pSoundNode, Game::Dimension::Unit* p);
}

#ifdef DEBUG_DEP
#warning "audio.h-end"
#endif

#endif
