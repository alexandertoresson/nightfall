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
#ifndef __ERRORS_H__
#define __ERRORS_H__

#ifdef DEBUG_DEP
#warning "errors.h"
#endif

enum
{
	SUCCESS = 0,
	ERROR_GENERAL,

	WINDOW_ERROR_INIT_FAILURE,
	WINDOW_ERROR_ALREADY_INITIALIZED, 
	WINDOW_ERROR_NOT_INITIALIZED,
	WINDOW_ERROR_NO_CONFIG_FILE,	
	WINDOW_ERROR_CANNOT_FIND_CONFIGFILE,
	WINDOW_ERROR_NO_CONFIGURATION_INTERPRETER,
	
	AUDIO_ERROR_MISSING_DIRECTORY,
	AUDIO_ERROR_INVALID_CONFIGURATION,
	AUDIO_ERROR_INVALID_AUDIO_LIST,
	AUDIO_ERROR_LIST_MISSING_TAG,
	AUDIO_ERROR_LIST_CORRECT_TAG_WRONG_PLACE,
	AUDIO_ERROR_LIST_INCORRECT_VALUE,
	AUDIO_ERROR_LIST_UNKNOWN_TYPE,
	AUDIO_THREAD_PREOCCUPIED,
	AUDIO_ERROR_LIST_INCORRECT_FORMAT,
	AUDIO_ERROR_LIST_INVALID_INDEX,
	AUDIO_ERROR_LIST_INVALID_FILE,
	
	STRUCTURED_INSTRUCTIONS_ERROR_NO_LIST,
	
	ENVIRONMENT_INVALID_CONDITIONS,
	ENVIRONMENT_SKYBOX_LOAD_FAILED,
	ENVIRONMENT_SKYBOX_REFERENCE_NOT_FOUND,
	
	MODEL_ERROR_INVALID_FORMAT,
	MODEL_ERROR_UNEXPECTED_ERROR,
	MODEL_ERROR_PARSE,
	MODEL_ERROR_FILE_NOT_FOUND,
	MODEL_ERROR_NAME_CONFLICT,
	MODEL_ERROR_INVALID_SOUND_FORMAT,

	FONT_ERROR_INIT_FAILURE,
	FONT_ERROR_FILE_LOAD,

	NETWORK_ERROR_RESOLVE_FAILED,
	NETWORK_ERROR_PACKET_ALLOC_FAILED,
	NETWORK_ERROR_SOCKET_OPEN,
	NETWORK_ERROR_SOCKET_BIND,
	NETWORK_ERROR_INIT,
	
	FILE_DOES_NOT_EXIST
};

#ifdef DEBUG_DEP
#warning "errors.h-end"
#endif

#endif
