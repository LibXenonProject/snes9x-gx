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

void handle_sound(void *) {
        //S9xFinalizeSamples();
        S9xFinalizeSamples();

        /* this all isn't that great... */
        //	int sample_rate = 48000*2;
        //	int samples_per_frame = Settings.PAL ? sample_rate / 50 : sample_rate / 60;

        int samples_guard = 2048;

        if (xenon_sound_get_unplayed() < samples_guard) {
                unsigned char buffer[65536];

                int req_samples = 512;

                S9xMixSamples(buffer, req_samples);

                int i;
                for (i = 0; i < req_samples * 2; i += 4)
                        *(int*) (buffer + i) = __builtin_bswap32(*(int*) (buffer + i));
                xenon_sound_submit(buffer, req_samples * 2);
        }
}

/****************************************************************************
 * Audio Threading
 ***************************************************************************/
static void * AudioThread(void *arg) {
        return NULL;
}

/****************************************************************************
 * MixSamples
 * This continually calls S9xMixSamples On each DMA Completion
 ***************************************************************************/
static void XenonMixSamples() {
        S9xFinalizeSamples();
}

/****************************************************************************
 * InitAudio
 ***************************************************************************/
void InitAudio() {
        xenon_sound_init();
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
