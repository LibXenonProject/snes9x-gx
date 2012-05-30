/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May 2007-July 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * snes9xgx.cpp
 *
 * This file controls overall program flow. Most things start and end here!
 ***************************************************************************/

#include <xetypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include "w_input.h"
#include <libfat/fat.h>
#include <debug.h>
#include <sys/iosupport.h>
#include <diskio/ata.h>
#include <usb/usbmain.h>
#include <xenon_soc/xenon_power.h>
#include <debug.h>

#include "snes9xgx.h"
//#include "networkop.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "sram.h"
#include "freeze.h"
#include "preferences.h"
#include "button_mapping.h"
#include "fileop.h"
#include "filebrowser.h"
#include "input.h"
#include "FreeTypeGX.h"

#include "../snes9x/snes9x.h"
#include "../snes9x/memmap.h"
#include "../snes9x/apu/apu.h"
#include "../snes9x/controls.h"

int ScreenshotRequested = 0;
int ConfigRequested = 0;
int ShutdownRequested = 0;
int ResetRequested = 0;
int ExitRequested = 0;
char appPath[1024] = { 0 };
char loadedFile[1024] = { 0 };
static int currentMode;

extern "C" {
extern void __exception_setreload(int t);
}

extern void S9xInitSync();
extern uint32 prevRenderedFrameCount;

/****************************************************************************
 * Shutdown / Reboot / Exit
 ***************************************************************************/

void ExitCleanup()
{
	ShutdownAudio();
	StopGX();

	HaltDeviceThread();
	UnmountAllFAT();
}

void ExitApp()
{
	SavePrefs(SILENT);

	if (SNESROMSize > 0 && !ConfigRequested && GCSettings.AutoSave == 1)
		SaveSRAMAuto(SILENT);

	ExitCleanup();

	//if(ShutdownRequested)
                exit(0);
}

int main(int argc, char *argv[])
{
	xenon_make_it_faster(XENON_SPEED_FULL);
	usb_init();
	usb_do_poll();
	xenon_ata_init();

	InitDeviceThread();
	//InitGCVideo(); // Initialise video
	InitVideo();
	ResetVideo_Menu (); // change to menu video mode
	SetupPads();
	MountAllFAT(); // Initialize libFAT for SD and USB


	DefaultSettings (); // Set defaults
	S9xUnmapAllControls ();
	SetDefaultButtonMap ();

	// Allocate SNES Memory
	if (!Memory.Init ())
		ExitApp();

	// Allocate APU
	if (!S9xInitAPU ())
		ExitApp();

	S9xSetRenderPixelFormat (RGB565); // Set Pixel Renderer to match 565
	S9xInitSound (64, 0); // Initialise Sound System

	// Initialise Graphics
	//setGFX ();
	if (!S9xGraphicsInit ())
		ExitApp();
	
	//AllocGfxMem();
	S9xInitSync(); // initialize frame sync
	InitFreeType((u8*)font_ttf, font_ttf_size); // Initialize font system
	
	savebuffer = (unsigned char *)malloc(SAVEBUFFERSIZE);
	browserList = (BROWSERENTRY *)malloc(sizeof(BROWSERENTRY)*MAX_BROWSER_SIZE);
	
	InitGUIThreads();

	while (1) // main loop
	{
		// go back to checking if devices were inserted/removed
		// since we're entering the menu
		ResumeDeviceThread();

		SwitchAudioMode(1);

		if(SNESROMSize == 0)
			MainMenu(MENU_GAMESELECTION);
		else
			MainMenu(MENU_GAME);
#ifdef HW_RVL
		SelectFilterMethod();
#endif
		
		ConfigRequested = 0;
		ScreenshotRequested = 0;
		SwitchAudioMode(0);
		
		Settings.MultiPlayer5Master = (GCSettings.Controller == CTRL_PAD4 ? true : false);
		Settings.SuperScopeMaster = (GCSettings.Controller == CTRL_SCOPE ? true : false);
		Settings.MouseMaster = (GCSettings.Controller == CTRL_MOUSE ? true : false);
		Settings.JustifierMaster = (GCSettings.Controller == CTRL_JUST ? true : false);
		SetControllers ();

		// stop checking if devices were removed/inserted
		// since we're starting emulation again
		HaltDeviceThread();
		
		AudioStart ();

		//CheckVideo = 2;	// force video update
		prevRenderedFrameCount = IPPU.RenderedFramesCount;
		currentMode = GCSettings.render;

		while(1) // emulation loop
		{
			S9xMainLoop ();
			ReportButtons ();

			if(ResetRequested)
			{
				S9xSoftReset (); // reset game
				ResetRequested = 0;
			}
			if (ConfigRequested)
			{
				ConfigRequested = 0;
				ResetVideo_Menu();
				break;
			}
		} // emulation loop
	} // main loop
}
