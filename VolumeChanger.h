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
	

#define		VOLUME_UP_KEY		(0xC00E9)
#define		VOLUME_DOWN_KEY		(0xC00EA)
#define		VOLUME_MUTE_KEY		(0xC00E2)



class VolumeChanger :
 public BInputServerFilter
{
	public:
		VolumeChanger();
		virtual ~VolumeChanger();
		virtual status_t InitCheck();
		virtual filter_result Filter(BMessage* message, BList* outlist);
	
	private:
		int previousVolumeLevel;
		
		FILE* logFile;
		status_t Mute(void);
		status_t UnMute (void);
		status_t SetVolumeLevel(int In, float* Out);
		status_t GetVolumeLevel(int *);
};

#endif
