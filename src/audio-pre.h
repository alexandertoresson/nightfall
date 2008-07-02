#ifndef __AUDIO_H_PRE__
#define __AUDIO_H_PRE__

#ifdef DEBUG_DEP
#warning "audio.h-pre"
#endif

namespace Audio
{
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

	struct AudioList;
	struct AudioFXInfo;
	class SoundNode;
}

#endif
