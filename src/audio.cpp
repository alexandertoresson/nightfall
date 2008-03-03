#include "audio.h"

#include "errors.h"
#include "vector3d.h"
#include "console.h"
#include "dimension.h"
#include "unit.h"
#include <vector>
#include <map>
#include <queue>
#include <iostream>
#include <cmath>

namespace Audio
{
	SDL_Thread*  pAudioThread;  // Thread monitoring the playlist.
	AudioStates* pAudioStates;  // Hashtable that holds all sound and music. The music is accessed through std::string keys.
	ThreadInfo*  pAudioThreadInfo = NULL;
	bool         soundIsEnabled = false;
	Utilities::LinkedList<SoundNode*>* pNodesList;

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
			delete pNodesList;

			return SUCCESS;
		}

		return SUCCESS;
	}

	void _DeallocSoundNodes(Utilities::Node<SoundNode*>* pFront)
	{
		Utilities::Node<SoundNode*>* pCurNode = pFront;
		do 
		{
			delete pCurNode->value;
			pCurNode->value = NULL;

			pCurNode = pCurNode->pNext;
		} while(pCurNode != NULL);
	}

	void DeallocList()
	{
		if (pAudioStates == NULL)
			return;

		if (pAudioThread != NULL)
			_KillThread();

		if (pAudioStates->size() > 0)
		{
			AudioStatesIterator iter;
			for (iter = pAudioStates->begin(); iter != pAudioStates->end(); iter++)
			{
				std::cout << "Releasing audio list \"" << (*iter).first << '"' << std::endl;
				AudioList* list = (*iter).second;
				if (list->ppMusic != NULL)
				{
					for (int i = 0; i < list->length; i++)
					{
						if (list->ppMusic[i] != NULL)
						{
							Mix_FreeMusic(list->ppMusic[i]); // Free the music-stuff
							list->ppMusic[i] = NULL;
							std::cout << list->ppMusic[i] << " freed" << std::endl;
						}
					}

					delete [] list->ppMusic;
					list->ppMusic = NULL;
				}
				else if (list->ppSound != NULL)
				{
					for (int i = 0; i < list->length; i++)
					{
						if (list->ppSound[i] != NULL)
						{
							std::cout << list->ppSound[i] << " freed" << std::endl;
							Mix_FreeChunk(list->ppSound[i]); // Free the binary-chunk
							list->ppSound[i] = NULL;
						}
					}

					delete [] list->ppSound;
					list->ppSound = NULL;
				}

				std::cout << "Deleting " << (*iter).first << std::endl;
				delete list;
				list = NULL;
			}
			pAudioStates->clear();
		}

		delete pAudioStates;
		pAudioStates = NULL;
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
		const char* musicDirectory = musicConfig.GetValue("music directory");
		const char* soundDirectory = musicConfig.GetValue("sound directory");

		// Make sure there's given a valid value (and not a null-terminated string!)
		if (musicDirectory[0] == 0)
		{
			console << Console::err << "Invalid directory input for music." << Console::nl;
			return AUDIO_ERROR_INVALID_CONFIGURATION;
		}

		if (soundDirectory[0] == 0)
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
		const char* formatType;
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
		const char* createAudioList = musicConfig.GetValue("create audio list");

		// Create a new audio handler. 
		pAudioStates = new AudioStates;	

		if (createAudioList[0] != 0 && !strcmp(createAudioList, "yes"))
		{
			// Holds where the audio list is located
			const char* audioListFile = musicConfig.GetValue("audio list");

			console << "Loading audio from audio list " << audioListFile << "..." << Console::nl;

			// Make sure no zero-string
			if (audioListFile[0] == 0)
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
			AudioList*              pList = NULL;
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
					if (pList != NULL)
					{
						// Make sure the list is set to either music or sound fx.
						if (pList->type == AUDIO_UNKNOWN)
						{
							delete pList;
							DeallocList();
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
								pList->length  = finalBuffer.size();
								pList->ppMusic = new Music*[pList->length];
								for (int i = 0; i < pList->length; i++)
								{
									pList->ppMusic[i] = finalBuffer.front();
									finalBuffer.pop();
								}
								(*pAudioStates)[name] = pList;
							}
							else
							{
								delete pList;
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

							if (finalBuffer.size() > 0)
							{
								pList->ppSound = new Sound*[finalBuffer.size()];
								pList->length  = finalBuffer.size();
								for (int i = 0; i < pList->length; i++)
								{
									pList->ppSound[i] = finalBuffer.front();
									finalBuffer.pop();
								}
								(*pAudioStates)[name] = pList;
							}
							else
								delete pList;
						}

						pList = NULL;

						if (name.length() > 0)
							name.clear();
					}
					else
						console << Console::err << "Warning: null-pointer pList at save-param." << Console::nl;
				}
				//
				// Defines which list we wish to work with.
				//
				else if (pInstruction->instruction == "list")
				{					
					if (pList != NULL)
					{
						console << Console::err << "Warning: pointer set where it supposed to be deallocated! Doing an immediate kill - MEMORY LEAK?!" << Console::nl;
						delete pList;
					}

					pList = new AudioList;
					pList->ppSound  = NULL;
					pList->ppMusic  = NULL;
					pList->length   = 0;
					pList->playType = AUDIO_PLAY_TYPE_UNKNOWN;
					pList->type     = AUDIO_UNKNOWN;					

					if (pInstruction->value.size() == 0)
					{
						delete pList;
						DeallocList();
						return AUDIO_ERROR_LIST_MISSING_TAG;
					}

					name         = pInstruction->value;
					currentIndex = 0;
				}
				//
				// Do we wish to repeat in infinity (~63000 times) or perhaps
				// play just once, fire n' forget!?
				//
				else if (pInstruction->instruction == "type")
				{
					if (pList == NULL)
					{
						DeallocList();
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
					if (pList == NULL)
					{
						DeallocList();
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
					if (pList == NULL)
					{
						DeallocList();
						return AUDIO_ERROR_LIST_CORRECT_TAG_WRONG_PLACE;
					}

					if (pInstruction->value.length() > 128)
					{
						delete pList;
						DeallocList();
						return AUDIO_ERROR_LIST_INVALID_FILE;
					}

					console << "Adding " << pInstruction->value << Console::nl;
					buffer.push(pInstruction->value);
				}
			}
		}

		_CreateThread();

		pNodesList = new Utilities::LinkedList<SoundNode*>(true);
		pNodesList->SetDeallocFunc(_DeallocSoundNodes);

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

	void GetAudioList(std::string tag, AudioList*& pList)
	{
		// Returns the audio list. Hashtable. 
		AudioStatesIterator it = (*pAudioStates).find(tag);

		if (it != (*pAudioStates).end())
		{
			pList = (*it).second;
			return;
		}

		if (pList != NULL)
			pList = NULL;
	}

	Sound* GetSound(std::string tag, int index)
	{
		if (soundIsEnabled)
		{
			AudioList* pList = NULL;
			GetAudioList(tag, pList);

			if (pList == NULL)
				return NULL;

			if (pList->ppSound == NULL)
				return NULL;

			if (index < 0 || index > pList->length)
				return NULL;

			return pList->ppSound[index];
		}
		else
		{
			return NULL;
		}
	}

	int PlayList(std::string tag, int index, int volume)
	{
		if (soundIsEnabled)
		{
			// Get the audio-list with its songs.
			// No validation, no nothing. Just get it, and pass
			// it over to it's identical twin.
			AudioList* pList = NULL;
			GetAudioList(tag, pList);

			return PlayList(pList, index, volume, tag.c_str());
		}
		else
		{
			return SUCCESS;
		}
	}

	int PlayList(AudioList* pList, int index /* = 0 */, int volume /* = -1 */, const char* tag /* = "" */)
	{
		if (soundIsEnabled)
		{
			if (pList == NULL)
				return AUDIO_ERROR_LIST_MISSING_TAG;

			if (pList->type != AUDIO_MUSIC)
				return AUDIO_ERROR_LIST_INCORRECT_FORMAT;

			if (pAudioThread == NULL)
			{
				console << Console::err << "Failed to play list " << tag << " because of missing audio-thread." << Console::nl;
				return ERROR_GENERAL;
			}

			if (index >= pList->length)
				index = pList->length - 1;

			SDL_LockMutex(pAudioThreadInfo->mutex);
			if (pAudioThreadInfo->pList != NULL)
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

			if (tag != 0)
				PlayListCurrentlyPlaying(tag);
		}

		return SUCCESS;
	}

	const char* PlayListCurrentlyPlaying(const char* set /* = "" */)
	{
		static std::string currentPlaylist;
		if (set == 0)
			return currentPlaylist.c_str();

		currentPlaylist = set;
		return set;
	}

	bool PlayListPlaying(const char* tag)
	{
		if (strcmp(PlayListCurrentlyPlaying(), tag) == 0)
			return false;

		return true;
	}

	int PlayOnce(string tag, int* channel, int index, int volume)
	{
		if (soundIsEnabled)
		{
			AudioList* pList = NULL;
			GetAudioList(tag, pList);

			if (pList == NULL)
				return AUDIO_ERROR_LIST_MISSING_TAG;

			if (pList->type != AUDIO_SOUND)
				return AUDIO_ERROR_LIST_INCORRECT_FORMAT;

			if (index >= pList->length || index < 0)
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

			if (channel != NULL)
				*channel = response;		

			return response == -1 ? ERROR_GENERAL : SUCCESS;
		}
		else
		{
			return SUCCESS;
		}
	}	

	int PlayOnceFromLocation(string tag, int* channel, Utilities::Vector3D* pSound, const Utilities::Vector3D* pCamera, int index, float strength)
	{
		if (soundIsEnabled)
		{
			//
			// Note: passes arguments to its identical twin defined below.
			//

			AudioList* pList = NULL;
			GetAudioList(tag, pList);

			if (pList == NULL)
				return AUDIO_ERROR_LIST_MISSING_TAG;

			if (pList->type != AUDIO_SOUND)
				return AUDIO_ERROR_LIST_INCORRECT_FORMAT;

			if (index < 0 || index >= pList->length)
				return AUDIO_ERROR_LIST_INVALID_INDEX;

			return PlayOnceFromLocation(pList->ppSound[index], channel, pSound, pCamera);
		}
		else
		{
			return SUCCESS;
		}
	}

	int PlayOnceFromLocation(Sound* pSound, int* channel, Utilities::Vector3D* vSound, const Utilities::Vector3D* vCamera, float strength)
	{
		if (soundIsEnabled)
		{
			if (pSound == NULL)
				return ERROR_GENERAL;

			pSound->volume = CalculateVolume(*vCamera, vSound->x, vSound->y, strength);

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

			if (pInfo->pList != NULL)
			{
				if (pInfo->index >= pInfo->pList->length)
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

				while (pInfo->pList == NULL && pInfo->threadRuntime == true)
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

	SoundListNode* CreateSoundNode(Sound* pSound, float x, float y, float dx, float dy, float strength, int numPlay)
	{
		if (pSound == NULL)
			return NULL;

		SoundNode* pNode = new SoundNode(pSound, x, y, dx, dy, strength, numPlay);
		if (!pNode)
			return NULL;
			
		pNode->pSpeaker = NULL;

		return (SoundListNode*)pNodesList->Add(pNode);
	
		return NULL;
	}

	void RemoveSoundNode(SoundListNode* pNode)
	{
		if (pNode == NULL)
			return;

		if (pNode->value->channel > -1)
		{
			if (Mix_Playing(pNode->value->channel))
				Mix_FadeOutChannel(pNode->value->channel, 500);
		}

		pNodesList->Remove(pNode, true);
		pNode = NULL;
	}

	void PlaceSoundNodes(const Utilities::Vector3D& vObserver)
	{
		if (soundIsEnabled)
		{
			if (!pNodesList->Length())
				return;

			Utilities::Node<SoundNode*>* pCurNode = pNodesList->pHead;
			Utilities::Node<SoundNode*>* pNextNode = NULL;
			SoundNode* pCurObj = NULL;
			while (pCurNode != NULL)
			{
				pCurObj   = pCurNode->value;
				pNextNode = pCurNode->pNext;
				
				if (pCurObj->pSpeaker != NULL)
				{
					pCurObj->x = pCurObj->pSpeaker->pos.x;
					pCurObj->y = pCurObj->pSpeaker->pos.y;

					if (pCurObj->pSpeaker->owner != Game::Dimension::currentPlayerView)
					{
						if (!Game::Dimension::UnitIsVisible(pCurObj->pSpeaker, Game::Dimension::currentPlayerView))
						{
							if (pCurObj->channel > -1)
							{
								if (Mix_Playing(pCurObj->channel))
									Mix_HaltChannel(pCurObj->channel);
								pCurObj->channel = -1;
							}
							
							pCurNode = pNextNode;
							continue;
						}
					}
				}

				Uint8 volume = CalculateVolume(vObserver, pCurObj->x, pCurObj->y, pCurObj->strength);

				if (pCurObj->channel == -1)
				{
					if (volume > 0)
					{
						pCurObj->channel = Mix_PlayChannel(-1, pCurObj->pSound, pCurObj->times);
						Mix_Volume(pCurObj->channel, volume);
					}
				}
				else
				{
					if (!Mix_Playing(pCurObj->channel))
					{
						pNodesList->Remove(pCurNode, true);
					}
					else
					{
						if (Mix_Volume(pCurObj->channel, -1) != volume)
							Mix_Volume(pCurObj->channel, volume);
					}
				}
				
				pCurNode = pNextNode;
			}
		}
	}

	Uint8 CalculateVolume(const Utilities::Vector3D& vCamera, float x, float y, float strength)
	{
		float distance = vCamera.distance(Game::Dimension::GetTerrainCoord(x, y));
		if (distance >= strength)
			return 0;

		float k = MIX_MAX_VOLUME / strength;
		Uint8 val = (Uint8)((strength - distance) * k);
//		if (val < 0)       // if you want to test for the case where the volume is < 0, 
//			val = 0;   // this is not the place for it as val is already an unsigned integer.
		return val;
	}

	void SetSpeakerUnit(SoundNode* pSoundNode, Game::Dimension::Unit* p)
	{
		if (soundIsEnabled)
		{
			assert(pSoundNode != NULL);

			pSoundNode->pSpeaker = p;
		}
	}
}
