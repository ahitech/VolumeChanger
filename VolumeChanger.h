#ifndef VOLUMECHANGER_H
#define VOLUMECHANGER_H

#include "MixerControl.h"

#include <Errors.h>
#include <InputServerFilter.h>
#include <Message.h>
#include <ParameterWeb.h>
#include <SupportDefs.h>

#include <stdlib.h>
#include <stdio.h>


extern "C" _EXPORT BInputServerFilter* instantiate_input_filter();



const uint32	kFindButton	=	'Tfnd';
#define		SETTINGS_MESSAGE_CONSTANT	'sett'
#define		MESSAGE_TIMEOUT		(1000)	// 1000 microseconds

/*
 * Key codes
 */
#define		VOLUME_UP_KEY		(0xC00E9)
#define		VOLUME_DOWN_KEY		(0xC00EA)
#define		VOLUME_MUTE_KEY		(0xC00E2)
#define		SEARCH_KEY			(0xC0221)
#define		WINDOWS_KEY			(0x00066)
#define		CTRL_KEY			(0x0005C)
#define		ALT_KEY				(0x0005D)
#define		SEARCH_KEY_2		(0x00070070)


#define		PATH_TO_SETTINGS_FILE	\
							"/boot/home/config/settings/VolumeChanger.txt"

/*
 * Strings for searching of the key codes
 */
#define		VOLUME_UP_KEY_NAME		"Volume Up Key"
#define		VOLUME_DOWN_KEY_NAME	"Volume Down Key"
#define		VOLUME_MUTE_KEY_NAME	"Volume Mute Key"
#define		SEARCH_KEY_NAME			"Search Key"
#define		SEARCH_KEY_2_NAME		"Search Key 2"
#define		WINDOWS_KEY_NAME		"Windows Key"
#define		CTRL_KEY_NAME			"Control Key"
#define		ALT_KEY_NAME			"Alt Key"


const uint32 STATE_NEUTRAL 		= 0x0000;
const uint32 STATE_WIN_HELD		= 0x0001;
const uint32 STATE_CTRL_HELD	= 0x0002;
const uint32 STATE_ALT_HELD		= 0x0004;


class MixerControl;


class VolumeChanger :
 public BInputServerFilter
{
	public:
		VolumeChanger();
		virtual ~VolumeChanger();
		virtual status_t InitCheck();
		virtual filter_result Filter(BMessage* message, BList* outlist);
	
	private:		
		// Settings section
		BMessage* settings;
		status_t ReadSettingsFromFile(void);
		status_t GetSettings();
		status_t SaveSettings();
		status_t CheckSettings(void);
		
		FILE* logFile;
		void Mute(void);
		status_t SetVolumeLevel(int In);
		status_t GetVolumeLevel(int *);
		void ChangeVolumeBy(int in);
		void OpenSearch(void);
		
		MixerControl* mixerControl;
};

#endif
