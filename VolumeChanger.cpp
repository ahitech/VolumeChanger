#include "VolumeChanger.h"

#include <Alert.h>
#include <MediaRoster.h>
#include <String.h>

#include <string.h>


/* DEBUGGING */
FILE *out;


VolumeChanger::VolumeChanger() 
{	
	previousVolumeLevel = MUTED;	// We start with non-muted sound
}


VolumeChanger::~VolumeChanger()
{
	if (previousVolumeLevel != MUTED)
	{
		UnMute();
	}
}


/*
 * This function receives ERROR or new volume level
 * 	If input is ERROR, current volume level is returned
 * 	Otherwise, the input is set as new volume level
 * Returns:
 *	B_ERROR in case of error
 *	B_OK if everything was good
 */
status_t VolumeChanger::SetVolumeLevel (int volumeIn, float* volumeOut)
{
	// Code of this function is based on "setvolume" utility
	// by Axel Dörfler, axeld@pinc-software.de
	
	status_t status;
	media_node mixer;
	BMediaRoster *roster;
	BParameter *parameter;
	BContinuousParameter *gain = NULL;
	BParameterWeb *web;
	
	roster = BMediaRoster::Roster();
	if (NULL == roster)
		return B_ERROR;

	status = roster->GetAudioMixer(&mixer);
	if (B_OK != status)
		return B_ERROR;

	status = roster->GetParameterWebFor(mixer, &web);
	roster->ReleaseNode(mixer);
	if (status != B_OK)
	{
		web = NULL;
		return B_ERROR;
	}
	
	for (int32 index = 0; 
		 (parameter = web->ParameterAt(index)) != NULL; 
		 index++) 
	{
		if (!strcmp(parameter->Kind(), B_MASTER_GAIN)) {
			gain = dynamic_cast<BContinuousParameter *>(parameter);
			break;
		}
	}

	if (gain == NULL) {
		delete web;
		return B_ERROR;
	}

	float volume = 0.0;
	
	if (ERROR == volumeIn)		// Get current volume
	{
		bigtime_t when;
		size_t size = sizeof(volume);
		gain->GetValue(&volume, &size, &when);
		out = fopen ("/boot/home/textlog.txt", "a+");
		if (out) {
			fprintf (out, "SetVolumeLevel: Read volume %f\n", volume);
			fclose (out);
		}
		*volumeOut = volume;
	}
	else					// Set new volume
	{
		if (volumeIn > gain->MaxValue())
			volumeIn = gain->MaxValue();
		else if (volumeIn < gain->MinValue())
			volumeIn = gain->MinValue();

		if (out) {
			fprintf (out, "SetVolumeLevel: Setting volume to %d\n", volume);
			fclose (out);
		}


		gain->SetValue(&volumeIn, sizeof(volumeIn), system_time());
		*volumeOut = volumeIn;
	}
	return B_OK;
}


status_t VolumeChanger::GetVolumeLevel (int* saveTo)
{
	float previousVolume;
	status_t status;
	
	status = SetVolumeLevel (ERROR, &previousVolume);
	if (B_OK != status)
	{
		return status;
	}
	
	// Rounding to nearest integer
	*saveTo = ((int )(previousVolume + 0.5));
	return B_OK;
}


status_t VolumeChanger::Mute(void)
{
	status_t status = GetVolumeLevel(&previousVolumeLevel);
	float dummy;
	if (status == B_OK)
	{
		status = SetVolumeLevel (MUTED, &dummy);
	}
	return status;
}


status_t VolumeChanger::UnMute (void)
{
	if (previousVolumeLevel != MUTED)
	{
		int newLevel = previousVolumeLevel;
		float dummy;
		previousVolumeLevel = MUTED;
		return SetVolumeLevel (newLevel, &dummy);
	}
	return B_OK;
}


filter_result VolumeChanger::Filter(BMessage* in,
									BList* outlist)
{
	int32 key;
	int volume;
	int change = 0;
	float dummy;
	bool changeVolume = false;
	FILE *out;

	if (in->what == B_UNMAPPED_KEY_DOWN)
	{

		in->FindInt32 ("key", &key);
		
		switch (key)
		{
			case VOLUME_UP_KEY:
				change = +1;
				break;
			case VOLUME_DOWN_KEY:
				change = -1;
				break;
			case VOLUME_MUTE_KEY:
			 /*
				if (previousVolumeLevel == MUTED)
					Mute();
				else
					UnMute();
			 */
				// Intentional fall-through
			 
			default:
				return B_DISPATCH_MESSAGE;
				break;
		};
		
		if (change != 0)
		{
			if (GetVolumeLevel (&volume) != B_OK)
			return B_DISPATCH_MESSAGE;
			
			out = fopen ("/boot/home/textlog.txt", "a+");
			if (out) {
				fprintf (out, "Read volume %d\n", volume);
				fprintf (out, "Setting volume to %d\n", volume + change);
				fclose (out);
			}
			
			
			SetVolumeLevel (volume + change, &dummy);
			return B_SKIP_MESSAGE;
		}
	}
	
	return B_DISPATCH_MESSAGE;
}
	


/*

filter_result VolumeChanger::Filter(BMessage* in,
									BList* outlist)
{
	filter_result res = B_DISPATCH_MESSAGE;
	if (in->what != B_UNMAPPED_KEY_DOWN)
	{
		return res;
	}
	
	int32 key;
	in->FindInt32 ("key", &key);
	int volume;
	int change;
	float dummy;
	switch (key)
	{
		case VOLUME_UP_KEY:
			change = 1;
			break;
		case VOLUME_DOWN_KEY:
			change = -1;
			break;
		case VOLUME_MUTE_KEY:
			if (previousVolumeLevel == MUTED)
				Mute();
			else
				UnMute();
			
			// Intentional fall-through
		default:
			return res;
			break;
	};
	
	if (previousVolumeLevel != MUTED)
	{
		SetVolumeLevel (previousVolumeLevel + change, &dummy);
		previousVolumeLevel = MUTED;
	} 
	else 
	{
		if (GetVolumeLevel (&volume) != B_OK)
			return res;
		SetVolumeLevel (volume + change, &dummy);
	}
				
	return res;
}


*/


status_t VolumeChanger::InitCheck()
{
	return B_OK;
}



BInputServerFilter* instantiate_input_filter()
{
	return (new(std::nothrow) VolumeChanger());
}
