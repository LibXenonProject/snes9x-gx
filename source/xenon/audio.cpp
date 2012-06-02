/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * Tantric 2008-2010
 *
 * audio.cpp
 *
 * Audio driver
 * Audio is fixed to 32Khz/16bit/Stereo
 ***************************************************************************/
#include <xetypes.h>
#include <xenon_sound/sound.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../snes9x/snes9x.h"
#include "../snes9x/memmap.h"
#include "../snes9x/cpuexec.h"
#include "../snes9x/ppu.h"
#include "../snes9x/apu/apu.h"
#include "../snes9x/display.h"
#include "../snes9x/gfx.h"
#include "../snes9x/spc7110.h"
#include "../snes9x/controls.h"
#include "snes9xgx.h"

#include <xenon_soc/xenon_power.h>
#include <ppc/atomic.h>

static uint8_t xenon_thread_stack[6 * 0x10000];
static uint32_t __attribute__((aligned(128))) audio_lock = 0;
static uint32_t have_sound = 0;

uint8_t buffer[65536];
int samples_guard = 2048;
int req_samples = 512;

static void audio_thread() {
	while (exitThreads == 0) {
		lock(&audio_lock);
		if (have_sound == 1) {

			//S9xFinalizeSamples();
			S9xFinalizeSamples();

			/* this all isn't that great... */
			//	int sample_rate = 48000*2;
			//	int samples_per_frame = Settings.PAL ? sample_rate / 50 : sample_rate / 60;

			if (xenon_sound_get_unplayed() < samples_guard) {
				S9xMixSamples(buffer, req_samples);
				
				for (int i = 0; i < req_samples * 2; i += 4)
					*(int*) (buffer + i) = __builtin_bswap32(*(int*) (buffer + i));
				
				xenon_sound_submit(buffer, req_samples * 2);
			}
		}
		have_sound = 0;
		unlock(&audio_lock);
		
		usleep(20);
	}
}

void handle_sound(void *) {
//	lock(&audio_lock);
//	have_sound = 1;
//	unlock(&audio_lock);
	
	//S9xFinalizeSamples();
	S9xFinalizeSamples();

	/* this all isn't that great... */
	//	int sample_rate = 48000*2;
	//	int samples_per_frame = Settings.PAL ? sample_rate / 50 : sample_rate / 60;

	if (xenon_sound_get_unplayed() < samples_guard) {
		S9xMixSamples(buffer, req_samples);

		for (int i = 0; i < req_samples * 2; i += 4)
			*(int*) (buffer + i) = __builtin_bswap32(*(int*) (buffer + i));

		xenon_sound_submit(buffer, req_samples * 2);
	}
}

/****************************************************************************
 * InitAudio
 ***************************************************************************/
void InitAudio() {
	xenon_sound_init();
	//xenon_run_thread_task(4, xenon_thread_stack + (4 * 0x10000) - 0x100, audio_thread);
	S9xSetSamplesAvailableCallback(handle_sound, NULL);
}

/****************************************************************************
 * SwitchAudioMode
 *
 * Switches between menu sound and emulator sound
 ***************************************************************************/
void SwitchAudioMode(int mode) {
}

/****************************************************************************
 * ShutdownAudio
 *
 * Shuts down audio subsystem. Useful to avoid unpleasant sounds if a
 * crash occurs during shutdown.
 ***************************************************************************/
void ShutdownAudio() {
}

/****************************************************************************
 * AudioStart
 *
 * Called to kick off the Audio Queue
 ***************************************************************************/
void AudioStart() {
}
