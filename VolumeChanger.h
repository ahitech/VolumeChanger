#ifndef VOLUMECHANGER_H
#define VOLUMECHANGER_H

#include <Errors.h>
#include <InputServerFilter.h>
#include <Message.h>
#include <ParameterWeb.h>
#include <SupportDefs.h>

#include <stdlib.h>
#include <stdio.h>


extern "C" _EXPORT BInputServerFilter* instantiate_input_filter();

/* Both MUTED and ERROR are based on MIN_INT (-2147483648),
 * and are therefore well in range for "float"
 */
#ifndef MIN_INT
#define		MIN_INT 			(-2147483648)
#endif
#define		MUTED	(MIN_INT + 1)
#ifdef	ERROR
#undef	ERROR
#endif
#define		ERROR	MIN_INT
	

#define		SETTINGS_MESSAGE_CONSTANT	'sett'

#define		VOLUME_UP_KEY		(0xC00E9)
#define		VOLUME_DOWN_KEY		(0xC00EA)
#define		VOLUME_MUTE_KEY		(0xC00E2)

#define		PATH_TO_SETTINGS_FILE	\
							"/boot/home/config/settings/VolumeChanger.txt"


#define		VOLUME_UP_KEY_NAME			"Volume Up Key"
#define		VOLUME_DOWN_KEY_NAME		"Volume Down Key"
#define		VOLUME_MUTE_KEY_NAME		"Volume Mute Key"
#define		PREVIOUS_VOLUME_LEVEL_NAME	"Previous Volume Level"



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
		int32 Mute(void);
		status_t UnMute (void);
		status_t SetVolumeLevel(int In, float* Out);
		status_t GetVolumeLevel(int *);
};

#endif
