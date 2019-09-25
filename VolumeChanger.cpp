#include "VolumeChanger.h"

#include <Alert.h>
#include <File.h>
#include <MediaRoster.h>
#include <Messenger.h>
#include <String.h>

#include <math.h>
#include <string.h>


VolumeChanger::VolumeChanger() 
{	
	MixerControl* mixerControl = new MixerControl(VOLUME_USE_MIXER);
	mixerControl->Connect(VOLUME_USE_MIXER);
	this->settings = new BMessage (SETTINGS_MESSAGE_CONSTANT);
	GetSettings();
}


VolumeChanger::~VolumeChanger()
{
	if (mixerControl)
	{
		delete mixerControl;
	}
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
	if (B_OK != settings->GetInfo(SEARCH_KEY_NAME, &type, &countFound))
	{
		settings->AddInt32(SEARCH_KEY_NAME, SEARCH_KEY);
	}
	if (B_OK != settings->GetInfo(WINDOWS_KEY_NAME, &type, &countFound))
	{
		settings->AddInt32(WINDOWS_KEY_NAME, WINDOWS_KEY);
	}
	if (B_OK != settings->GetInfo(CTRL_KEY_NAME, &type, &countFound))
	{
		settings->AddInt32(CTRL_KEY_NAME, CTRL_KEY);
	}
	if (B_OK != settings->GetInfo(ALT_KEY_NAME, &type, &countFound))
	{
		settings->AddInt32(ALT_KEY_NAME, SEARCH_KEY);
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
status_t VolumeChanger::SetVolumeLevel (int volumeIn)
{
	mixerControl->SetVolume(volumeIn);
	return B_OK;
}


status_t VolumeChanger::GetVolumeLevel (int* saveTo)
{
	*saveTo = (int )(mixerControl->Volume());
	return B_OK;
}


void VolumeChanger::ChangeVolumeBy (int change)
{
	int i = 1000;
	while (!mixerControl && (i-- > 0)) {
		this->mixerControl = new MixerControl (VOLUME_USE_MIXER);
		if (this->mixerControl)
			this->mixerControl->Connect(VOLUME_USE_MIXER);
	}
	if (mixerControl)
	{
		mixerControl->ChangeVolumeBy (change);
	} else {
		FILE* out = fopen ("/boot/home/log.txt", "wa");
		fprintf (out, "CHANGE VOLUME - Could not allocate the mixer Control!\n");
		fclose (out);
	}
}



void VolumeChanger::Mute(void)
{
	int i = 1000;
	while (!mixerControl && (i-- > 0)) {
		this->mixerControl = new MixerControl (VOLUME_USE_MIXER);
		if (this->mixerControl)
			this->mixerControl->Connect(VOLUME_USE_MIXER);
	}
	if (mixerControl) {
		bool muted = this->mixerControl->Mute();
		mixerControl->SetMute(!muted);
	} else {
		FILE* out = fopen ("/boot/home/log.txt", "wa");
		fprintf (out, "MUTE OR UNMUTE - Could not allocate the mixer Control!\n");
		fclose (out);
	}
	SaveSettings();
}



void VolumeChanger::OpenSearch(void)
{
	/* This function opens the same window as Deskbar -> Find... menu option.
	 * Which means sending a predefined BMessage to the Deskbar.
	 * I assume the Deskbar is running and don't do anything if it's not.
	 */
	BMessenger* deskbar = new BMessenger ("application/x-vnd.Be-TSKB", -1);
	
	if (deskbar) {
		FILE* out = fopen ("/boot/home/log.txt", "wa");
		fprintf (out, "OPEN SEARCH - Trying to send message to the Deskbar!\n");
		fclose (out);
		deskbar->SendMessage (kFindButton);
		delete (deskbar);
	}
	else
	{
		FILE* out = fopen ("/boot/home/log.txt", "wa");
		fprintf (out, "OPEN SEARCH - Could not locate the Deskbar!\n");
		fclose (out);
	}
}



filter_result VolumeChanger::Filter(BMessage* in,
									BList* outlist)
{
	int32 key;
	int volume;
	int change = 0;
	float dummy;
	bool changeVolume = false;
	
	static uint32 currentState = STATE_NEUTRAL;
	
	int32 volumeUpKey;
	int32 volumeDownKey;
	int32 volumeMuteKey;
	int32 searchKey;
	int32 windowsKey;
	int32 ctrlKey;
	int32 altKey;
	
	settings->FindInt32(VOLUME_UP_KEY_NAME, &volumeUpKey);
	settings->FindInt32(VOLUME_DOWN_KEY_NAME, &volumeDownKey);
	settings->FindInt32(VOLUME_MUTE_KEY_NAME, &volumeMuteKey);
	settings->FindInt32(SEARCH_KEY_NAME, &searchKey);
	settings->FindInt32(WINDOWS_KEY_NAME, &windowsKey);
	settings->FindInt32(CTRL_KEY_NAME, &ctrlKey);
	settings->FindInt32(ALT_KEY_NAME, &altKey);

	if (in->what == B_UNMAPPED_KEY_DOWN)
	{

		in->FindInt32 ("key", &key);
		
		if (key == volumeUpKey)
		{
			ChangeVolumeBy(+1);
		}
		else if (key == volumeDownKey)
		{
			ChangeVolumeBy(-1);
		}
		else if (key == volumeMuteKey)
		{
			Mute();
		}
		else if (key == searchKey)
		{
			OpenSearch();
		}
//		else if (key == windowsKey)
//		{
//			currentState &= STATE_WIN_HELD;
//		}
	}
	else if (in->what == B_UNMAPPED_KEY_UP)
	{
		in->FindInt32 ("key", &key);
		
//		if (key == windowsKey)
//		{
//			currentState ^= STATE_WIN_HELD;		// I assume here that Win was held before
//		}
	}
	else if (in->what == B_KEY_DOWN)
	{
//		uint32 modifiers;
		const char* bytes = NULL;
		if (in->FindString("bytes", &bytes) != B_OK) ;
//		in->FindInt32("modifiers", &modifiers);
		
		if (key == B_RIGHT_ARROW && (modifiers() & B_OPTION_KEY) != 0)
		{
			BAlert* alert = new BAlert ("Shortcut pressed",
										"You've pressed Win+Up!",
										"Yep!");
			alert->Go();
				
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
