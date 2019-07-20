#include "VolumeChanger.h"

#include <Alert.h>
#include <File.h>
#include <MediaRoster.h>
#include <String.h>

#include <math.h>
#include <string.h>


VolumeChanger::VolumeChanger() 
{	
	this->settings = new BMessage (SETTINGS_MESSAGE_CONSTANT);
	GetSettings();
}


VolumeChanger::~VolumeChanger()
{
	SaveSettings();
}


/*
 * This function fills the "settings" BMessage with settings of the program.
 * It first attempts to read the settings from file. In case anything goes unsuccessfully,
 * it fills the settings with defaults.
 * If this file does not exist, or any error is encountered, a default settings set is created.
 */
status_t VolumeChanger::GetSettings ()
{
	status_t	toReturn = B_OK;
	toReturn = ReadSettingsFromFile();		// Read info from the file
				// Return value from ReadSettingsFromFile() is not checked.
	toReturn = CheckSettings();				// Create a default settings set
	
	return (toReturn);		// Returning the result of reading the settings
}


/*
 * This function tries to open the file with settings at the preset path.
 * If successfully, it unflattens contents of the settings file into a BMessage.
 * It is assumed that the BMessage for settings is allocated. 
 *		It will be erased, but should be allocated!
 */
status_t VolumeChanger::ReadSettingsFromFile()
{
	status_t toReturn = B_OK;
	BFile *readFrom = new BFile(PATH_TO_SETTINGS_FILE, B_READ_ONLY);
	if ((toReturn = readFrom->InitCheck()) == B_OK)
	{
		toReturn = this->settings->Unflatten(readFrom);
		readFrom->Unset();								// Close the file
	}
	delete readFrom;
	return toReturn;
}


/*
 * This function flattens contents of the settings message into the settings file
 * at the predefined path.
 */
status_t VolumeChanger::SaveSettings ()
{
	status_t	toReturn = B_OK;
	BFile *writeTo = new BFile(PATH_TO_SETTINGS_FILE, 	// Try opening the file
							   B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if ((toReturn = writeTo->InitCheck()) == B_OK)
	{
		toReturn = this->settings->Flatten (writeTo);	// Perform actual write
		writeTo->Unset();								// Flush and close the file
		delete writeTo;
	}
	return (toReturn);
}


/*
 * After the settings are read, no matter if with error or not, they should be verified
 * and any missing settings should be added.
 * This function does exactly that.
 * It is assumed that the "settings" BMessage is initialized.
 */
status_t	VolumeChanger::CheckSettings (void)
{
	status_t 	toReturn = B_OK;
	type_code 	type = B_ANY_TYPE;
	int32		countFound = 0;

	// Sanity check
	if (! settings)
	{
		return (B_NO_INIT);
	}
	
	if (B_OK != settings->GetInfo(VOLUME_UP_KEY_NAME, &type, &countFound))
	{
		settings->AddInt32(VOLUME_UP_KEY_NAME, VOLUME_UP_KEY); 	
	}
	if (B_OK != settings->GetInfo(VOLUME_DOWN_KEY_NAME, &type, &countFound))
	{
		settings->AddInt32(VOLUME_DOWN_KEY_NAME, VOLUME_DOWN_KEY);
	}
	if (B_OK != settings->GetInfo(VOLUME_MUTE_KEY_NAME, &type, &countFound))
	{
		settings->AddInt32(VOLUME_MUTE_KEY_NAME, VOLUME_MUTE_KEY);
	}
	if (B_OK != settings->GetInfo(PREVIOUS_VOLUME_LEVEL_NAME, &type, &countFound))
	{
		settings->AddInt32(PREVIOUS_VOLUME_LEVEL_NAME, MUTED);
	}
	return B_OK;	
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
		*volumeOut = round(volume);
	}
	else					// Set new volume
	{
		if (volumeIn > (int)gain->MaxValue())
		{
			volumeIn = (int)gain->MaxValue();
		}
		else if (volumeIn < (int)gain->MinValue())
		{
			volumeIn = (int)gain->MinValue();
		}

		volume = volumeIn;
		gain->SetValue(&volume, sizeof(volume), system_time());
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
	*saveTo = ((int )previousVolume);
	return B_OK;
}


int32 VolumeChanger::Mute(void)
{
	int32 currentVolumeLevel;
	status_t status = GetVolumeLevel(&currentVolumeLevel);
	float dummy;
	if (status == B_OK)
	{
		status = SetVolumeLevel (MUTED, &dummy);
	}
	if (status == B_OK)
	{
		settings->ReplaceInt32(PREVIOUS_VOLUME_LEVEL_NAME, currentVolumeLevel);
		status = SaveSettings();		// Write changes to disk
	}
	return currentVolumeLevel;
}


status_t VolumeChanger::UnMute (void)
{
	int32 previousVolumeLevel;
	float dummy;
	settings->FindInt32(PREVIOUS_VOLUME_LEVEL_NAME, &previousVolumeLevel);
	SetVolumeLevel (previousVolumeLevel, &dummy);
	settings->ReplaceInt32(PREVIOUS_VOLUME_LEVEL_NAME, MUTED);
	return SaveSettings();		// Write changes to disk
}


filter_result VolumeChanger::Filter(BMessage* in,
									BList* outlist)
{
	int32 key;
	int volume;
	int change = 0;
	float dummy;
	bool changeVolume = false;
	
	int32 volumeUpKey;
	int32 volumeDownKey;
	int32 volumeMuteKey;
	int32 previousVolume;
	
	settings->FindInt32(VOLUME_UP_KEY_NAME, &volumeUpKey);
	settings->FindInt32(VOLUME_DOWN_KEY_NAME, &volumeDownKey);
	settings->FindInt32(VOLUME_MUTE_KEY_NAME, &volumeMuteKey);
	settings->FindInt32(PREVIOUS_VOLUME_LEVEL_NAME, &previousVolume);

	if (in->what == B_UNMAPPED_KEY_DOWN)
	{

		in->FindInt32 ("key", &key);
		
		if (key == volumeUpKey)
		{
			change = +1;
		}
		else if (key == volumeDownKey)
		{
			change = -1;
		}
		else if (key == volumeMuteKey)
		{
			if (previousVolume == MUTED) {
				previousVolume = Mute();
			} else {
				UnMute();
				previousVolume = MUTED;
			}
		}
		if (change != 0)
		{
			if (GetVolumeLevel (&volume) != B_OK)
			{
				return B_DISPATCH_MESSAGE;
			}
			
			SetVolumeLevel (volume + change, &dummy);
		}
	}
	
	return B_DISPATCH_MESSAGE;
}
	


status_t VolumeChanger::InitCheck()
{
	return B_OK;
}



BInputServerFilter* instantiate_input_filter()
{
	return (new(std::nothrow) VolumeChanger());
}
