/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May 2007
 * Michniewski 2008
 * Tantric 2008-2010
 *
 * s9xsupport.cpp
 *
 * Snes9x support functions
 ***************************************************************************/

#include "../snes9x/snes9x.h"
#include "../snes9x/memmap.h"
#include "../snes9x/display.h"
#include "../snes9x/apu/apu.h"
#include "../snes9x/controls.h"

#include <xetypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ppc/timebase.h>
#include <time/time.h>
#include <debug.h>

#include "snes9xgx.h"
#include "video.h"
#include "audio.h"

#ifdef NETPLAY_SUPPORT
#include "../snes9x/netplay.h"
#include "../snes9x/movie.h"
#include "menu.h"
#endif

#ifdef NETPLAY_SUPPORT
uint32	netplay_joypads[8];
uint32	netplay_old_joypads[8];
#endif

#define gettime mftb

#define MAX_MESSAGE_LEN (36 * 3)

static long long prev;
static long long now;

/*** Miscellaneous Functions ***/
void S9xExit()
{
#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay)
		S9xNPDisconnect();
#endif
	ExitApp();
}

void S9xMessage(int /*type */, int /*number */, const char *message)
{
	static char buffer[MAX_MESSAGE_LEN + 1];
	snprintf(buffer, MAX_MESSAGE_LEN, "%s", message);
	S9xSetInfoString(buffer);
}

void S9xAutoSaveSRAM()
{

}

/*** Sound based functions ***/
void S9xToggleSoundChannel(int c)
{
    static int sound_switch = 255;

    if (c == 8)
        sound_switch = 255;
    else
        sound_switch ^= 1 << c;

    S9xSetSoundControl (sound_switch);
}

/****************************************************************************
 * OpenSoundDevice
 *
 * Main initialisation for Wii sound system
 ***************************************************************************/
bool8 S9xOpenSoundDevice(void)
{
	InitAudio();
	return TRUE;
}

void S9xInitNetPlay()
{
#ifdef NETPLAY_SUPPORT
	if (strlen(Settings.ServerName) == 0)
	{
		return;
	}

	if (Settings.Port < 0)
		Settings.Port = -Settings.Port;

	if (Settings.NetPlay)
	{
		NetPlay.MaxFrameSkip = 10;
		
		if(NetPlay.Connected)
			S9xNPDisconnect ();

		if (!S9xNPConnectToServer(Settings.ServerName, Settings.Port, loadedFile))
		{
			char __error[512];
			sprintf(__error,"Failed to connect to server %s on port %d.\n", Settings.ServerName, Settings.Port);
			printf(__error);
			//ErrorPrompt(__error);
			//S9xExit();
			Settings.NetPlay = 0;
		}

		printf("Connected to server %s on port %d as player #%d playing %s.\n", Settings.ServerName, Settings.Port, NetPlay.Player, loadedFile);
	}
#endif
}

/* eke-eke */
void S9xInitSync()
{
	prev = gettime();
}

/*** Synchronisation ***/

void S9xSyncSpeed ()
{
#if 0
	unsigned int timediffallowed = Settings.TurboMode ? 0 : Settings.FrameTime;

	while (!S9xSyncSound())
		usleep(10);

	uint32 skipFrms = Settings.SkipFrames;

	if (Settings.TurboMode)
		skipFrms = Settings.TurboSkipFrames;

	now = gettime();

	if (tb_diff_usec(now,prev) > timediffallowed)
	{
		/* Timer has already expired */
		if (IPPU.SkippedFrames < skipFrms)
		{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}
		else
		{
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		}
	}
	else
	{
		/*** Ahead - so hold up ***/
		while (tb_diff_usec(now,prev) < timediffallowed)
		{
			now = gettime();
			usleep(50);
		}
		IPPU.RenderThisFrame = TRUE;
		IPPU.SkippedFrames = 0;
	}

	prev = now;
#endif
	
	#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay && NetPlay.Connected)
	{
		S9xNPSendJoypadUpdate(netplay_old_joypads[0]);
		for (int J = 0; J < 8; J++)
			netplay_joypads[J] = S9xNPGetJoypad(J);

		if (!S9xNPCheckForHeartBeat())
		{
			NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay(100);

			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}
		else
		{
			NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay(200);

			if (IPPU.SkippedFrames < NetPlay.MaxFrameSkip)
			{
				IPPU.RenderThisFrame = FALSE;
				IPPU.SkippedFrames++;
			}
			else
			{
				IPPU.RenderThisFrame = TRUE;
				IPPU.SkippedFrames = 0;
			}
		}

		if (!NetPlay.PendingWait4Sync)
		{
			NetPlay.FrameCount++;
			S9xNPStepJoypadHistory();
		}

		return;
	}
#endif
	
	return;
}

/*** Video / Display related functions ***/
bool8 S9xInitUpdate()
{
	return (TRUE);
}

bool8 S9xDeinitUpdate(int Width, int Height)
{
	update_video(Width, Height);
	return (TRUE);
}

bool8 S9xContinueUpdate(int Width, int Height)
{
	return (TRUE);
}

void S9xSetPalette()
{
	return;
}

/*** Input functions ***/
void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2)
{
	return;
}

bool S9xPollButton(uint32 id, bool * pressed)
{
	return 0;
}

bool S9xPollAxis(uint32 id, int16 * value)
{
	return 0;
}

bool S9xPollPointer(uint32 id, int16 * x, int16 * y)
{
	return 0;
}

/****************************************************************************
 * Note that these are DUMMY functions, and only allow Snes9x to
 * compile. Where possible, they will return an error signal.
 ***************************************************************************/

const char *S9xChooseFilename(bool8 read_only)
{
	ExitApp();
	return NULL;
}

const char * S9xChooseMovieFilename(bool8 read_only)
{
	ExitApp();
	return NULL;
}

const char * S9xGetDirectory(enum s9x_getdirtype dirtype)
{
	ExitApp();
	return NULL;
}

const char * S9xGetFilename(const char *ex, enum s9x_getdirtype dirtype)
{
	ExitApp();
	return NULL;
}

const char * S9xGetFilenameInc(const char *e, enum s9x_getdirtype dirtype)
{
	ExitApp();
	return NULL;
}

const char * S9xBasename(const char *name)
{
	ExitApp();
	return name;
}

const char * S9xStringInput (const char * s)
{
	ExitApp();
	return s;
}

void _splitpath(char const *buf, char *drive, char *dir, char *fname, char *ext)
{
	ExitApp();
}

void _makepath(char *filename, const char *drive, const char *dir,
		const char *fname, const char *ext)
{
	ExitApp();
}

int dup(int fildes)
{
	ExitApp();
	return 1;
}

int access(const char *pathname, int mode)
{
	ExitApp();
	return 1;
}

#ifdef NETPLAY_SUPPORT
void doNetplay(){
	if (NetPlay.PendingWait4Sync && !S9xNPWaitForHeartBeatDelay(100))
	{
		// S9xProcessEvents(FALSE);
		return;
	}

	for (int J = 0; J < 8; J++)
		netplay_old_joypads[J] = MovieGetJoypad(J);

	for (int J = 0; J < 8; J++)
		MovieSetJoypad(J, netplay_joypads[J]);

	if (NetPlay.Connected)
	{
		if (NetPlay.PendingWait4Sync)
		{
			NetPlay.PendingWait4Sync = FALSE;
			NetPlay.FrameCount++;
			S9xNPStepJoypadHistory();
		}
	}
	else
	{
		ErrorPrompt("Lost connection to server.\n");
		Settings.NetPlay = 0;
	}
}
#endif