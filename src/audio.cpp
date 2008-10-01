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
#include "audio.h"

#include "errors.h"
#include "vector3d.h"
#include "console.h"
#include "dimension.h"
#include "unitsquares.h"
#include "unit.h"
#include "configuration.h"
#include "utilities.h"
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;

namespace Audio
{
	SDL_Thread*  pAudioThread;  // Thread monitoring the playlist.
	AudioStates audioStates;  // Hashtable that holds all sound and music. The music is accessed through std::string keys.
	ThreadInfo*  pAudioThreadInfo = NULL;
	bool         soundIsEnabled = false;
	std::list<SoundNode> nodesList;

	int Kill()
	{
		if (soundIsEnabled)
		{
			StopAll();

			_KillThread();

			//		while (threadRuntime == 1)
			//			SDL_Delay(10);

			int instances = Mix_QuerySpec(NULL, NULL, NULL);
			for (int i = 0; i < instances; i++)
				Mix_CloseAudio();

			DeallocList();

			return SUCCESS;
		}

		return SUCCESS;
	}

	AudioList::~AudioList()
	{
		std::cout << "Releasing audio list \"" << this << '"' << std::endl;

		for (unsigned i = 0; i < ppMusic.size(); i++)
		{
			Mix_FreeMusic(ppMusic[i]); // Free the music-stuff
			std::cout << ppMusic[i] << " freed" << std::endl;
		}

		for (unsigned i = 0; i < ppSound.size(); i++)
		{
			std::cout << ppSound[i] << " freed" << std::endl;
			Mix_FreeChunk(ppSound[i]); // Free the binary-chunk
		}

	}

	void DeallocList()
	{
		audioStates.clear();
	}

	int Init(std::string configFile)
	{
		//
		// audio.txt must ALWAYS lie in the very root of the application.
		//
		Utilities::ConfigurationFile musicConfig(configFile);

		int error = 0;
		if ((error = musicConfig.Parse()) != SUCCESS)
			return error;

		// GetValue gets the configuration value (quiote obvious, not?)
		std::string musicDirectory = musicConfig.GetValue("music directory");
		std::string soundDirectory = musicConfig.GetValue("sound directory");

		// Make sure there's given a valid value
		if (musicDirectory.length() == 0)
		{
			console << Console::err << "Invalid directory input for music." << Console::nl;
			return AUDIO_ERROR_INVALID_CONFIGURATION;
		}

		if (soundDirectory.length() == 0)
		{
			console << Console::err << "Invalid directory input for music." << Console::nl;
			return AUDIO_ERROR_INVALID_CONFIGURATION;
		}

		// Enforces integer values. If a non-numeric symbol is found, the variable affected
		// will be set to zero. . .
		int rate       = Utilities::StringToInt(musicConfig.GetValue("audio rate"));
		int setup      = Utilities::StringToInt(musicConfig.GetValue("audio setup"));
		int channels   = Utilities::StringToInt(musicConfig.GetValue("audio channels"));
		int chunksize  = Utilities::StringToInt(musicConfig.GetValue("audio chunksize"));

		// . . . thus the following checks
		if (rate <= 0)
		{
			console << Console::err <<  "Invalid bitrate" << Console::nl;
			return AUDIO_ERROR_INVALID_CONFIGURATION;
		}

		if (channels <= 0 ||
			channels >= 1000 /* some number to prevent fools from crashing the application */)
		{
			console << Console::err << "Invalid amount of channels" << Console::nl;
			return AUDIO_ERROR_INVALID_CONFIGURATION;
		}

		if (setup <= 0 || setup > 2)
		{
			console << Console::err << "Invalid setup. 1 = mono, 2 = stereo" << Console::nl;
			return AUDIO_ERROR_INVALID_CONFIGURATION;
		}

		if (chunksize <= 0)
		{
			console << Console::err << "Invalid chunksize" << Console::nl;
			return AUDIO_ERROR_INVALID_CONFIGURATION;
		}

		// Get the audio format
		int format = (Uint16) Utilities::StringToInt(musicConfig.GetValue("audio format"));
		std::string formatType;
		if (format == 0 || format < 0 || format > 8)
		{
			format = MIX_DEFAULT_FORMAT;
			formatType = "Default";	
		}
		else
		{
			// Now these are the pre-defined integers.
			switch (format)
			{
			case 1:
				format = AUDIO_U8;
				formatType = "Unsigned 8-bit";
				break;
			case 2:
				format = AUDIO_S8;
				formatType = "Signed 8-bit";
				break;
			case 3:
				format = AUDIO_U16LSB;
				formatType = "Unsigned 16-bit little-endian";
				break;
			case 4:
				format = AUDIO_S16LSB;
				formatType = "Signed 16-bit little-endian";
				break;
			case 5:
				format = AUDIO_U16MSB;
				formatType = "Unsigned 16-bit big-endian";
				break;
			case 6:
				format = AUDIO_S16MSB;
				formatType = "Signed 16-bit big-endian";
				break;
			case 7:
				format = AUDIO_U16SYS;
				formatType = "Unsigned 16-bit system byteorder";
				break;
			default:
				format = AUDIO_S16SYS;
				formatType = "Signed 16-bit system byteorder";
			}
		}

		if (Mix_OpenAudio(rate, (Uint16) format, setup, chunksize) < 0)
		{
			console << Console::err << "Audio Error: " << Mix_GetError() << Console::nl;
			return ERROR_GENERAL;
		}

		Mix_AllocateChannels(channels);

		// Output the results
		console << "AUDIO PROPERTIES "                    << Console::nl
			<< "Description -------- Value"           << Console::nl
			<< "Rate: . . . . . . . . " << rate       << Console::nl
			<< "Setup:  . . . . . . . " << (setup == 1 ? "mono" : "stereo") << Console::nl
			<< "Channels: . . . . . . " << channels   << Console::nl
			<< "Chunksize:  . . . . . " << chunksize  << Console::nl
			<< "Format: . . . . . . . " << formatType << Console::nl
			<< "Current music volume: " << GetMusicVolume() << Console::nl
			<< "Current mix volume: . " << GetChannelVolume(-1) << Console::nl;		

		// Create audio list?
		std::string createAudioList = musicConfig.GetValue("create audio list");

		if (createAudioList == "yes")
		{
			// Holds where the audio list is located
			std::string audioListFile = musicConfig.GetValue("audio list");

			console << "Loading audio from audio list " << audioListFile << "..." << Console::nl;

			// Make sure no zero-string
			if (audioListFile.length() == 0)
			{
				console << Console::err << "Audio list not defined" << Console::nl;
				return AUDIO_ERROR_INVALID_AUDIO_LIST;
			}


			// Get to work! This thing parses the structured
			// instructions file. Once it's done, it's possible to iterate
			// through each instruction and its values.
			Utilities::StructuredInstructionsFile audioList(audioListFile);

			if ((error = audioList.Parse()) != SUCCESS)
			{
				return error;
			}

			audioList.PrepareIterator();

			const Utilities::StructuredInstructionsItem* pInstruction;
			std::queue<std::string> buffer;
			gc_ptr<AudioList>       pList = NULL;
			std::string             name;
			int                     currentIndex = 0;

			while((pInstruction = audioList.NextItem()) != NULL)
			{
				//
				// Finalizes everything. Clears the temporary placeholders
				// and loads what is needed. Simply prepares everything for
				// take-off.
				//
				if (pInstruction->instruction == "save")
				{
					if (pList)
					{
						// Make sure the list is set to either music or sound fx.
						if (pList->type == AUDIO_UNKNOWN)
						{
							return AUDIO_ERROR_LIST_INCORRECT_FORMAT;
						}
						else if (pList->type == AUDIO_MUSIC && buffer.size() > 0)
						{
							std::queue<Music*> finalBuffer;
							std::string file;
							while (buffer.size() > 0)
							{
								file = (std::string) musicDirectory + buffer.front();
								console << "Loading '" << file << "' as music. Result: ";

								Music* pNoice = Mix_LoadMUS(file.c_str());
								if (pNoice == NULL)
									console << "Failure. Error: " << Mix_GetError() << Console::nl;
								else
								{
									console << "Success!" << Console::nl;
									finalBuffer.push(pNoice);
								}
								buffer.pop();
							}

							if (finalBuffer.size() > 0)
							{
								while (finalBuffer.size())
								{
									pList->ppMusic.push_back(finalBuffer.front());
									finalBuffer.pop();
								}
								audioStates[name] = pList;
							}
							else
							{
								return ERROR_GENERAL;
							}
						}
						else if (buffer.size() > 0)
						{
							std::queue<Sound*> finalBuffer;
							std::string file;
							while (buffer.size() > 0)
							{
								file = (std::string) musicDirectory + buffer.front();
								console << "Loading '" << file << "' as sound-FX. Result: ";

								Sound* pNoice = Mix_LoadWAV(file.c_str());
								if (pNoice == NULL)
									console << "Failure. Error: " << Mix_GetError() << Console::nl;
								else
								{
									console << "Success!" << Console::nl;
									finalBuffer.push(pNoice);
								}
								buffer.pop();
							}

							while (finalBuffer.size())
							{
								pList->ppSound.push_back(finalBuffer.front());
								finalBuffer.pop();
							}
							audioStates[name] = pList;
						}

						pList = NULL;

						name = "";
					}
					else
						console << Console::err << "Warning: null-pointer pList at save-param." << Console::nl;
				}
				//
				// Defines which list we wish to work with.
				//
				else if (pInstruction->instruction == "list")
				{					
					if (pList)
					{
						console << Console::err << "Warning: audio list not saved" << Console::nl;
					}

					pList = new AudioList;

					if (pInstruction->value.size() == 0)
					{
						return AUDIO_ERROR_LIST_MISSING_TAG;
					}

					name = pInstruction->value;
					currentIndex = 0;
				}
				//
				// Do we wish to repeat in infinity (~63000 times) or perhaps
				// play just once, fire n' forget!?
				//
				else if (pInstruction->instruction == "type")
				{
					if (!pList)
					{
						return AUDIO_ERROR_LIST_CORRECT_TAG_WRONG_PLACE;
					}

					pList->playType = pInstruction->value == "repeat" ? AUDIO_PLAY_TYPE_LOOP :
						AUDIO_PLAY_TYPE_ONCE;
				}
				//
				// What kind of audio-type are we dealing with?
				// Soundfx or music?
				//
				else if (pInstruction->instruction == "audio")
				{
					if (!pList)
					{
						return AUDIO_ERROR_LIST_CORRECT_TAG_WRONG_PLACE;
					}

					std::string compare;
					Utilities::StringTrim(pInstruction->value, compare);

					pList->type = compare == "music" ? AUDIO_MUSIC : compare == "sound" ? AUDIO_SOUND : AUDIO_UNKNOWN; 
				}
				//
				// Adds music or sound fx to defined list
				//
				else if (pInstruction->instruction == "add")
				{
					if (!pList)
					{
						return AUDIO_ERROR_LIST_CORRECT_TAG_WRONG_PLACE;
					}

					console << "Adding " << pInstruction->value << Console::nl;
					buffer.push(pInstruction->value);
				}
			}
		}

		_CreateThread();

		return SUCCESS;
	}

	void OnChannelFinish(void (*fptr)(int channel))
	{
		Mix_ChannelFinished(fptr);
	}

	void OnMusicFinish(void (*fptr)(void))
	{
		Mix_HookMusicFinished(fptr);
	}

	gc_ptr<AudioList> GetAudioList(std::string tag)
	{
		// Returns the audio list. Hashtable. 
		AudioStatesIterator it = audioStates.find(tag);

		if (it != audioStates.end())
		{
			return (*it).second;
		}
		return NULL;
	}

	Sound* GetSound(std::string tag, unsigned index)
	{
		if (soundIsEnabled)
		{
			gc_ptr<AudioList> pList = GetAudioList(tag);

			if (!pList)
				return NULL;

			if (index > pList->ppSound.size())
				return NULL;

			return pList->ppSound[index];
		}
		else
		{
			return NULL;
		}
	}

	int PlayList(std::string tag, unsigned index, int volume)
	{
		if (soundIsEnabled)
		{
			// Get the audio-list with its songs.
			// No validation, no nothing. Just get it, and pass
			// it over to it's identical twin.
			gc_ptr<AudioList> pList = GetAudioList(tag);

			return PlayList(pList, index, volume, tag.c_str());
		}
		else
		{
			return SUCCESS;
		}
	}

	int PlayList(gc_ptr<AudioList> pList, unsigned index, int volume, std::string tag)
	{
		if (soundIsEnabled)
		{
			if (!pList)
				return AUDIO_ERROR_LIST_MISSING_TAG;

			if (pList->type != AUDIO_MUSIC)
				return AUDIO_ERROR_LIST_INCORRECT_FORMAT;

			if (pAudioThread == NULL)
			{
				console << Console::err << "Failed to play list " << tag << " because of missing audio-thread." << Console::nl;
				return ERROR_GENERAL;
			}

			if (index >= pList->ppMusic.size())
				index = pList->ppMusic.size() - 1;

			SDL_LockMutex(pAudioThreadInfo->mutex);
			if (pAudioThreadInfo->pList)
			{
				pAudioThreadInfo->newIndex = index;
				pAudioThreadInfo->switchSong = true;
			}
			else
			{
				pAudioThreadInfo->index = index;
				pAudioThreadInfo->newIndex = -1;
			}
			pAudioThreadInfo->pList         = pList;
			pAudioThreadInfo->threadRuntime = true;
			SDL_UnlockMutex(pAudioThreadInfo->mutex);

			if (volume >= 0)
				SetMusicVolume(volume);

			if (tag.length())
				PlayListCurrentlyPlaying(tag);
		}

		return SUCCESS;
	}

	std::string PlayListCurrentlyPlaying(std::string set)
	{
		static std::string currentPlaylist;
		if (!set.length())
			return currentPlaylist;

		currentPlaylist = set;
		return set;
	}

	bool PlayListPlaying(std::string tag)
	{
		if (PlayListCurrentlyPlaying() == tag)
			return false;

		return true;
	}

	int PlayOnce(string tag, int* channel, unsigned index, int volume)
	{
		if (soundIsEnabled)
		{
			gc_ptr<AudioList> pList = GetAudioList(tag);

			if (!pList)
				return AUDIO_ERROR_LIST_MISSING_TAG;

			if (pList->type != AUDIO_SOUND)
				return AUDIO_ERROR_LIST_INCORRECT_FORMAT;

			if (index >= pList->ppSound.size())
				return AUDIO_ERROR_LIST_INVALID_INDEX;

			Sound* pNoice = pList->ppSound[index];
			if (pNoice == NULL)
				return ERROR_GENERAL;

			if (volume >= 0 && volume < 256)
				pNoice->volume = (Uint8) volume;

			return PlayOnce(pNoice, channel);
		}
		else
		{
			return SUCCESS;
		}
	}

	int PlayOnce(Sound* pNoice, int* channel)
	{
		if (soundIsEnabled)
		{
			if (pNoice == NULL)
				return ERROR_GENERAL;

			int response = Mix_PlayChannel(-1, pNoice, 0);

			*channel = response;

			return response == -1 ? ERROR_GENERAL : SUCCESS;
		}
		else
		{
			return SUCCESS;
		}
	}	

	int PlayOnceFromLocation(string tag, int* channel, const Utilities::Vector3D& pSound, const Utilities::Vector3D& pCamera, unsigned index, float strength)
	{
		if (soundIsEnabled)
		{
			//
			// Note: passes arguments to its identical twin defined below.
			//

			gc_ptr<AudioList> pList = GetAudioList(tag);

			if (!pList)
				return AUDIO_ERROR_LIST_MISSING_TAG;

			if (pList->type != AUDIO_SOUND)
				return AUDIO_ERROR_LIST_INCORRECT_FORMAT;

			if (index >= pList->ppSound.size())
				return AUDIO_ERROR_LIST_INVALID_INDEX;

			return PlayOnceFromLocation(pList->ppSound[index], channel, pSound, pCamera);
		}
		else
		{
			return SUCCESS;
		}
	}

	int PlayOnceFromLocation(Sound* pSound, int* channel, const Utilities::Vector3D& vSound, const Utilities::Vector3D& vCamera, float strength)
	{
		if (soundIsEnabled)
		{
			if (pSound == NULL)
				return ERROR_GENERAL;

			pSound->volume = CalculateVolume(vCamera, vSound, strength);

			return PlayOnce(pSound, channel);
		}
		else
		{
			return SUCCESS;
		}
	}

	int GetMusicVolume(void)
	{
		if (soundIsEnabled)
		{
			return Mix_VolumeMusic(-1);
		}
		else
		{
			return 100;
		}
	}

	int GetChannelVolume(int channel)
	{
		if (soundIsEnabled)
		{
			return Mix_Volume(channel, -1);
		}
		else
		{
			return 100;
		}
	}

	void SetMusicVolume(int volume)
	{
		if (soundIsEnabled)
		{
			if (volume < 0)
				volume = 0;

			else if (volume > MIX_MAX_VOLUME)
				volume = MIX_MAX_VOLUME;

			Mix_VolumeMusic(volume);
		}
	}

	void SetChannelVolume(int channel, int volume)
	{
		if (soundIsEnabled)
		{
			if (volume < 0)
				volume = 0;

			else if (volume > MIX_MAX_VOLUME)
				volume = MIX_MAX_VOLUME;

			Mix_Volume(channel, volume);
		}
	}

	void _StopAllChannels(void)
	{
		Mix_FadeOutMusic(1000);  
		Mix_HaltChannel(-1);  // Kill all SoundFX
	}

	void _SetThreadToSleep(void)
	{
		SDL_LockMutex(pAudioThreadInfo->mutex);
		pAudioThreadInfo->index = 0;
		pAudioThreadInfo->pList = NULL;
		SDL_UnlockMutex(pAudioThreadInfo->mutex);
	}

	void StopList(void) // << deprecated
	{
		StopAll();
	}

	void StopAll(void)
	{
		if (soundIsEnabled)
		{
			_StopAllChannels();
			_SetThreadToSleep();
		}
	}

	bool _CreateThread(void)
	{
		assert (pAudioThread == NULL);

		pAudioThreadInfo = new ThreadInfo;
		pAudioThreadInfo->index         = 0;
		pAudioThreadInfo->newIndex      = -1;
		pAudioThreadInfo->mutex         = SDL_CreateMutex();
		pAudioThreadInfo->pList         = NULL;
		pAudioThreadInfo->threadRuntime = true;
		pAudioThreadInfo->switchSong    = false;

		pAudioThread = SDL_CreateThread(_ThreadMethod, static_cast<void*>(pAudioThreadInfo));

		if (pAudioThread == NULL)
		{
			console << Console::err << "Failed to create audio-thread. Error: " << SDL_GetError() << Console::nl;
			return false;
		}

		soundIsEnabled = true;

		return true;
	}

	int _ThreadMethod(void* arg)
	{
		// Extract the ThreadInfo.
		ThreadInfo* pInfo = static_cast<ThreadInfo*>(arg);

		while (true)
		{
			SDL_LockMutex(pInfo->mutex);
			Music* pTune = NULL;

			if (pInfo->pList)
			{
				if (pInfo->index >= pInfo->pList->ppMusic.size())
				{
					if (pInfo->pList->playType == AUDIO_PLAY_TYPE_LOOP)
					{
						pInfo->index = 0;
						pTune = pInfo->pList->ppMusic[0];
					}
				}
				else
					pTune = pInfo->pList->ppMusic[pInfo->index];
			}

			if (pTune == NULL)
			{
				pInfo->pList = NULL;
				pInfo->index = 0;
				pInfo->switchSong = false;
				pInfo->newIndex = 0;

				SDL_UnlockMutex(pInfo->mutex);

				while (!pInfo->pList && pInfo->threadRuntime == true)
					SDL_Delay(1000);

				if (pInfo->threadRuntime == false)
				{
					_StopAllChannels();
					break;
				}

				continue;
			}
			SDL_UnlockMutex(pInfo->mutex);

			console << "Audio: List play advance (" << pInfo->index << ")" << Console::nl;

			// Fade in! We don't want rough transitions here!
			Mix_FadeInMusic(pTune, 0, 1000);

			// The inhumane loop in which the thread does most of its
			// work. No, really, it's not that bad. Check whether the
			// song is still playing. If so, just wait one second, and
			// check again.
			while (Mix_PlayingMusic() && pInfo->threadRuntime == true && pInfo->switchSong == false)
				SDL_Delay(1000);

			if (pInfo->switchSong == true)
			{
				Mix_FadeOutMusic(1000);
				SDL_Delay(1500);

				SDL_LockMutex(pInfo->mutex);
				pInfo->switchSong = false;

				if (pInfo->newIndex > -1)
				{
					pInfo->index = pInfo->newIndex - 1;
					pInfo->newIndex = -1;
				}
				SDL_UnlockMutex(pInfo->mutex);
			}
			else if (pInfo->threadRuntime == false)
			{
				_StopAllChannels();
				break;
			}

			// Advance the index... and check whether we've reached the very end
			// of the list.
			SDL_LockMutex(pInfo->mutex);
			pInfo->index++;
			SDL_UnlockMutex(pInfo->mutex);

			SDL_Delay(4000);
		}
		return 0;
	}

	// 
	// Annihilate everything! Kill kill kill!
	// No, not really. Just ask the thread to
	// stop working. Await the very moment the
	// thread drops the tools, and goes asleep.
	//
	// Why prefixed with _? Well, we, meaning I,
	// does not wish to see this function being
	// called from an external module. Now, please
	// use StopAll or StopList instead.
	//
	// DO NOT CALL FROM EXTERNAL MODULES	
	void _KillThread(void)
	{
		if (pAudioThread == NULL)
			return;

		pAudioThreadInfo->threadRuntime = 0;

		SDL_WaitThread(pAudioThread, NULL);

		SDL_DestroyMutex(pAudioThreadInfo->mutex);
		delete pAudioThreadInfo;

		pAudioThreadInfo = NULL;
		pAudioThread = NULL;
	}

	SoundListNode CreateSoundNode(Sound* pSound, const Utilities::Vector3D& position, const Utilities::Vector3D& velocity, float strength, int numPlay)
	{
		if (pSound == NULL)
			return std::list<SoundNode>::iterator();

		nodesList.push_back(SoundNode(pSound, position, velocity, strength, numPlay));
		return --nodesList.end();
	}

	void RemoveSoundNode(SoundListNode pNode)
	{
		if ((*pNode).channel > -1)
		{
			if (Mix_Playing((*pNode).channel))
				Mix_FadeOutChannel((*pNode).channel, 500);
		}

		nodesList.erase(pNode);
	}

	void PlaceSoundNodes(const Utilities::Vector3D& vObserver)
	{
		if (soundIsEnabled)
		{
			for (std::list<SoundNode>::iterator it = nodesList.begin(); it != nodesList.end(); )
			{
				if (!it->pSpeaker)
				{
					it->position = Game::Dimension::GetTerrainCoord(it->pSpeaker->pos.x, it->pSpeaker->pos.y);

					if (it->pSpeaker->owner != Game::Dimension::currentPlayerView)
					{
						if (!Game::Dimension::UnitIsVisible(it->pSpeaker, Game::Dimension::currentPlayerView))
						{
							if (it->channel > -1)
							{
								if (Mix_Playing(it->channel))
									Mix_HaltChannel(it->channel);
								it->channel = -1;
							}
							
							it++;
							continue;
						}
					}
				}

				Uint8 volume = CalculateVolume(vObserver, it->position, it->strength);

				if (it->channel == -1)
				{
					if (volume > 0)
					{
						it->channel = Mix_PlayChannel(-1, it->pSound, it->times);
						Mix_Volume(it->channel, volume);
					}
				}
				else
				{
					if (!Mix_Playing(it->channel))
					{
						std::list<SoundNode>::iterator last = it;
						it++;
						nodesList.erase(last);
					}
					else
					{
						if (Mix_Volume(it->channel, -1) != volume)
							Mix_Volume(it->channel, volume);
					}
				}
				
			}
		}
	}

	Uint8 CalculateVolume(const Utilities::Vector3D& vCamera, const Utilities::Vector3D& vPosition, float strength)
	{
		float distance = vCamera.distance(vPosition);
		if (distance >= strength)
			return 0;

		float k = MIX_MAX_VOLUME / strength;
		Uint8 val = (Uint8)((strength - distance) * k);
		return val;
	}

	void SetSpeakerUnit(SoundListNode& pSoundListNode, const gc_ptr<Game::Dimension::Unit>& p)
	{
		if (soundIsEnabled)
		{
			(*pSoundListNode).pSpeaker = p;
		}
	}
}
