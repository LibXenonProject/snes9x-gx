/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * Modified by Ced2911, 2011
 *
 * input.cpp
 * Wii/GameCube controller management
 ***************************************************************************/
#include "../snes9x/snes9x.h"
#include "../snes9x/memmap.h"
#include "../snes9x/controls.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <input/input.h>
#include <usb/usbmain.h>
#include <xetypes.h>
//#include "menu.h"
#include "video.h"
#include "input.h"
#include "gui/gui.h"

#include "snes9xgx.h"
#include "button_mapping.h"



extern "C" void enableCapture();

#define MAX_INPUTS 4

int rumbleRequest[4] = {0, 0, 0, 0};
GuiTrigger userInput[4];
static int rumbleCount[4] = {0, 0, 0, 0};


static WPADData wpad_xenon[MAX_INPUTS];

static struct controller_data_s ctrl[MAX_INPUTS];
static struct controller_data_s old_ctrl[MAX_INPUTS];

#define ASSIGN_BUTTON_TRUE( keycode, snescmd ) \
	  S9xMapButton( keycode, cmd = S9xGetCommandT(snescmd), true)

#define ASSIGN_BUTTON_FALSE( keycode, snescmd ) \
	  S9xMapButton( keycode, cmd = S9xGetCommandT(snescmd), false)

#define	STICK_DEAD_ZONE (13107)
#define HANDLE_STICK_DEAD_ZONE(x) ((((x)>-STICK_DEAD_ZONE) && (x)<STICK_DEAD_ZONE)?0:(x-x/abs(x)*STICK_DEAD_ZONE))

static void XenonInputInit() {
        memset(ctrl, 0, MAX_INPUTS * sizeof (controller_data_s));
        memset(old_ctrl, 0, MAX_INPUTS * sizeof (controller_data_s));

        memset(wpad_xenon, 0, MAX_INPUTS * sizeof (WPADData));
}

#define PUSHED(x) ((ctrl[ictrl].x)&&(old_ctrl[ictrl].x==0))
#define RELEASED(x) ((ctrl[ictrl].x==0)&&(old_ctrl[ictrl].x==1))
#define HELD(x) ((ctrl[ictrl].x==1))

static uint16_t WPAD_ButtonsDown(int ictrl) {
        uint16_t btn = 0;

        if (PUSHED(a)) {
                btn |= WPAD_CLASSIC_BUTTON_A;
        }

        if (PUSHED(b)) {
                btn |= WPAD_CLASSIC_BUTTON_B;
        }

        if (PUSHED(x)) {
                btn |= WPAD_CLASSIC_BUTTON_X;
        }

        if (PUSHED(y)) {
                btn |= WPAD_CLASSIC_BUTTON_Y;
        }

        if (PUSHED(up)) {
                btn |= WPAD_CLASSIC_BUTTON_UP;
        }

        if (PUSHED(down)) {
                btn |= WPAD_CLASSIC_BUTTON_DOWN;
        }

        if (PUSHED(left)) {
                btn |= WPAD_CLASSIC_BUTTON_LEFT;
        }

        if (PUSHED(right)) {
                btn |= WPAD_CLASSIC_BUTTON_RIGHT;
        }

        //    if (PUSHED(start)) {
        //        btn |= WPAD_CLASSIC_BUTTON_START;
        //    }
        //
        //    if (PUSHED(select)) {
        //        btn |= WPAD_CLASSIC_BUTTON_BACK;
        //    }
        //
        //    if (PUSHED(logo)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LOGO;
        //        enableCapture();
        //    }
        //
        //    if (PUSHED(rb)) {
        //        btn |= WPAD_CLASSIC_BUTTON_RB;
        //    }
        //
        //    if (PUSHED(lb)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LB;
        //    }
        //
        //    if (PUSHED(s1_z)) {
        //        btn |= WPAD_CLASSIC_BUTTON_RSTICK;
        //    }
        //
        //    if (PUSHED(s2_z)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LSTICK;
        //    }
        return btn;
}

static uint16_t WPAD_ButtonsUp(int ictrl) {
        uint16_t btn = 0;

        if (RELEASED(a)) {
                btn |= WPAD_CLASSIC_BUTTON_A;
        }

        if (RELEASED(b)) {
                btn |= WPAD_CLASSIC_BUTTON_B;
        }

        if (RELEASED(x)) {
                btn |= WPAD_CLASSIC_BUTTON_X;
        }

        if (RELEASED(y)) {
                btn |= WPAD_CLASSIC_BUTTON_Y;
        }

        if (RELEASED(up)) {
                btn |= WPAD_CLASSIC_BUTTON_UP;
        }

        if (RELEASED(down)) {
                btn |= WPAD_CLASSIC_BUTTON_DOWN;
        }

        if (RELEASED(left)) {
                btn |= WPAD_CLASSIC_BUTTON_LEFT;
        }

        if (RELEASED(right)) {
                btn |= WPAD_CLASSIC_BUTTON_RIGHT;
        }

        //    if (RELEASED(start)) {
        //        btn |= WPAD_CLASSIC_BUTTON_START;
        //    }
        //
        //    if (RELEASED(select)) {
        //        btn |= WPAD_CLASSIC_BUTTON_BACK;
        //    }
        //
        //    if (RELEASED(logo)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LOGO;
        //    }
        //
        //    if (RELEASED(rb)) {
        //        btn |= WPAD_CLASSIC_BUTTON_RB;
        //    }
        //
        //    if (RELEASED(lb)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LB;
        //    }
        //
        //    if (RELEASED(s1_z)) {
        //        btn |= WPAD_CLASSIC_BUTTON_RSTICK;
        //    }
        //
        //    if (RELEASED(s2_z)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LSTICK;
        //    }

        return btn;
}

static uint16_t WPAD_ButtonsHeld(int ictrl) {
        uint16_t btn = 0;

        if (HELD(a)) {
                btn |= WPAD_CLASSIC_BUTTON_A;
        }

        if (HELD(b)) {
                btn |= WPAD_CLASSIC_BUTTON_B;
        }

        if (HELD(x)) {
                btn |= WPAD_CLASSIC_BUTTON_X;
        }

        if (HELD(y)) {
                btn |= WPAD_CLASSIC_BUTTON_Y;
        }

        if (HELD(up)) {
                btn |= WPAD_CLASSIC_BUTTON_UP;
        }

        if (HELD(down)) {
                btn |= WPAD_CLASSIC_BUTTON_DOWN;
        }

        if (HELD(left)) {
                btn |= WPAD_CLASSIC_BUTTON_LEFT;
        }

        if (HELD(right)) {
                btn |= WPAD_CLASSIC_BUTTON_RIGHT;
        }
        //
        //    if (HELD(start)) {
        //        btn |= WPAD_CLASSIC_BUTTON_START;
        //    }
        //
        //    if (HELD(select)) {
        //        btn |= WPAD_CLASSIC_BUTTON_BACK;
        //    }
        //
        //    if (HELD(logo)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LOGO;
        //    }
        //
        //    if (HELD(rb)) {
        //        btn |= WPAD_CLASSIC_BUTTON_RB;
        //    }
        //
        //    if (HELD(lb)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LB;
        //    }
        //
        //    if (HELD(s1_z)) {
        //        btn |= WPAD_CLASSIC_BUTTON_RSTICK;
        //    }
        //
        //    if (HELD(s2_z)) {
        //        btn |= WPAD_CLASSIC_BUTTON_LSTICK;
        //    }

        return btn;
}

static uint16_t PAD_ButtonsDown(int ictrl) {
        uint16_t btn = 0;

        if (PUSHED(a)) {
                btn |= PAD_BUTTON_A;
        }

        if (PUSHED(b)) {
                btn |= PAD_BUTTON_B;
        }

        if (PUSHED(x)) {
                btn |= PAD_BUTTON_X;
        }

        if (PUSHED(y)) {
                btn |= PAD_BUTTON_Y;
        }

        if (PUSHED(up)) {
                btn |= PAD_BUTTON_UP;
        }

        if (PUSHED(down)) {
                btn |= PAD_BUTTON_DOWN;
        }

        if (PUSHED(left)) {
                btn |= PAD_BUTTON_LEFT;
        }

        if (PUSHED(right)) {
                btn |= PAD_BUTTON_RIGHT;
        }

        if (PUSHED(start)) {
                btn |= PAD_BUTTON_START;
        }

        if (PUSHED(select)) {
                btn |= PAD_BUTTON_BACK;
        }

        if (PUSHED(logo)) {
                btn |= PAD_BUTTON_LOGO;
        }

        if (PUSHED(rb)) {
                btn |= PAD_BUTTON_RB;
        }

        if (PUSHED(lb)) {
                btn |= PAD_BUTTON_LB;
        }

        if (PUSHED(s1_z)) {
                btn |= PAD_BUTTON_RSTICK;
        }

        if (PUSHED(s2_z)) {
                btn |= PAD_BUTTON_LSTICK;
        }
        return btn;
}

static uint16_t PAD_ButtonsUp(int ictrl) {
        uint16_t btn = 0;

        if (RELEASED(a)) {
                btn |= PAD_BUTTON_A;
        }

        if (RELEASED(b)) {
                btn |= PAD_BUTTON_B;
        }

        if (RELEASED(x)) {
                btn |= PAD_BUTTON_X;
        }

        if (RELEASED(y)) {
                btn |= PAD_BUTTON_Y;
        }

        if (RELEASED(up)) {
                btn |= PAD_BUTTON_UP;
        }

        if (RELEASED(down)) {
                btn |= PAD_BUTTON_DOWN;
        }

        if (RELEASED(left)) {
                btn |= PAD_BUTTON_LEFT;
        }

        if (RELEASED(right)) {
                btn |= PAD_BUTTON_RIGHT;
        }

        if (RELEASED(start)) {
                btn |= PAD_BUTTON_START;
        }

        if (RELEASED(select)) {
                btn |= PAD_BUTTON_BACK;
        }

        if (RELEASED(logo)) {
                btn |= PAD_BUTTON_LOGO;
        }

        if (RELEASED(rb)) {
                btn |= PAD_BUTTON_RB;
        }

        if (RELEASED(lb)) {
                btn |= PAD_BUTTON_LB;
        }

        if (RELEASED(s1_z)) {
                btn |= PAD_BUTTON_RSTICK;
        }

        if (RELEASED(s2_z)) {
                btn |= PAD_BUTTON_LSTICK;
        }

        return btn;
}

static uint16_t PAD_ButtonsHeld(int ictrl) {
        uint16_t btn = 0;

        if (HELD(a)) {
                btn |= PAD_BUTTON_A;
        }

        if (HELD(b)) {
                btn |= PAD_BUTTON_B;
        }

        if (HELD(x)) {
                btn |= PAD_BUTTON_X;
        }

        if (HELD(y)) {
                btn |= PAD_BUTTON_Y;
        }

        if (HELD(up)) {
                btn |= PAD_BUTTON_UP;
        }

        if (HELD(down)) {
                btn |= PAD_BUTTON_DOWN;
        }

        if (HELD(left)) {
                btn |= PAD_BUTTON_LEFT;
        }

        if (HELD(right)) {
                btn |= PAD_BUTTON_RIGHT;
        }

        if (HELD(start)) {
                btn |= PAD_BUTTON_START;
        }

        if (HELD(select)) {
                btn |= PAD_BUTTON_BACK;
        }

        if (HELD(logo)) {
                btn |= PAD_BUTTON_LOGO;
        }

        if (HELD(rb)) {
                btn |= PAD_BUTTON_RB;
        }

        if (HELD(lb)) {
                btn |= PAD_BUTTON_LB;
        }

        if (HELD(s1_z)) {
                btn |= PAD_BUTTON_RSTICK;
        }

        if (HELD(s2_z)) {
                btn |= PAD_BUTTON_LSTICK;
        }

        return btn;
}

s8 PAD_StickX(int i) {
        return HANDLE_STICK_DEAD_ZONE(ctrl[i].s1_x) >> 8;
}

s8 PAD_StickY(int i) {
        return HANDLE_STICK_DEAD_ZONE(ctrl[i].s1_y) >> 8;
}

s8 PAD_SubStickX(int i) {
        return HANDLE_STICK_DEAD_ZONE(ctrl[i].s2_y) >> 8;
}

s8 PAD_SubStickY(int i) {
        return HANDLE_STICK_DEAD_ZONE(ctrl[i].s2_y) >> 8;
}

u8 PAD_TriggerL(int i) {
        return ctrl[i].lt;
}

u8 PAD_TriggerR(int i) {
        return ctrl[i].rt;
}

static void XenonInputUpdate() {
        usb_do_poll();
        for (int i = 0; i < MAX_INPUTS; i++) {
                old_ctrl[i] = ctrl[i];
                get_controller_data(&ctrl[i], i);
        }
        // update wpad
        for (int i = 0; i < MAX_INPUTS; i++) {
                wpad_xenon[i].btns_d = WPAD_ButtonsDown(i);
                wpad_xenon[i].btns_u = WPAD_ButtonsUp(i);
                wpad_xenon[i].btns_h = WPAD_ButtonsHeld(i);

                //float irx = (float)((float)PAD_StickX(i)/128.f);
                //float iry = (float)(-(float)PAD_StickY(i)/128.f)-0.5f;
                //        float iry = 0.5f-((float)PAD_StickY(i)/128.f);
                float iry = 0.5f + ((float) -PAD_StickY(i) / 128.f);
                float irx = 0.5f + ((float) PAD_StickX(i) / 128.f);

                irx *= screenwidth;
                iry *= screenheight;

                wpad_xenon[i].ir.x = irx;
                wpad_xenon[i].ir.y = iry;

                wpad_xenon[i].ir.valid = 0;
        }
}

/****************************************************************************
 * UpdatePads
 *
 * Scans pad and wpad
 ***************************************************************************/
void UpdatePads() {
        XenonInputUpdate();

        for (int i = 3; i >= 0; i--) {
                userInput[i].pad.btns_d = PAD_ButtonsDown(i);
                userInput[i].pad.btns_u = PAD_ButtonsUp(i);
                userInput[i].pad.btns_h = PAD_ButtonsHeld(i);
                //        userInput[i].pad.stickX = PAD_StickX(i);
                //        userInput[i].pad.stickY = PAD_StickY(i);
                userInput[i].pad.substickX = PAD_SubStickX(i);
                userInput[i].pad.substickY = PAD_SubStickY(i);
                userInput[i].pad.triggerL = PAD_TriggerL(i);
                userInput[i].pad.triggerR = PAD_TriggerR(i);
        }
}

/****************************************************************************
 * SetupPads
 *
 * Sets up userInput triggers for use
 ***************************************************************************/
void SetupPads() {
        XenonInputInit();


        for (int i = 0; i < 4; i++) {
                userInput[i].chan = i;
                userInput[i].wpad = &wpad_xenon[i];
                userInput[i].wpad->exp.type = EXP_CLASSIC;
        }
}

/****************************************************************************
 * ShutoffRumble
 ***************************************************************************/

void ShutoffRumble() {

}

/****************************************************************************
 * DoRumble
 ***************************************************************************/

void DoRumble(int i) {

}

/****************************************************************************
 * Set the default mapping
 ***************************************************************************/
void SetDefaultButtonMap() {
        int maxcode = 0x10;
        s9xcommand_t cmd;

        /*** Joypad 1 ***/
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 A");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 B");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 X");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 Y");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 L");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 R");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 Start");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 Select");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 Up");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 Down");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 Left");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad1 Right");

        maxcode = 0x20;
        /*** Joypad 2 ***/
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 A");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 B");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 X");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 Y");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 L");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 R");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 Start");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 Select");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 Up");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 Down");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 Left");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad2 Right");

        maxcode = 0x30;
        /*** Joypad 3 ***/
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 A");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 B");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 X");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 Y");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 L");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 R");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 Start");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 Select");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 Up");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 Down");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 Left");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad3 Right");

        maxcode = 0x40;
        /*** Joypad 4 ***/
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 A");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 B");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 X");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 Y");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 L");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 R");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 Start");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 Select");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 Up");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 Down");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 Left");
        ASSIGN_BUTTON_FALSE(maxcode++, "Joypad4 Right");

        maxcode = 0x50;
        /*** Superscope ***/
        ASSIGN_BUTTON_FALSE(maxcode++, "Superscope Fire");
        ASSIGN_BUTTON_FALSE(maxcode++, "Superscope AimOffscreen");
        ASSIGN_BUTTON_FALSE(maxcode++, "Superscope Cursor");
        ASSIGN_BUTTON_FALSE(maxcode++, "Superscope ToggleTurbo");
        ASSIGN_BUTTON_FALSE(maxcode++, "Superscope ToggleTurbo");
        ASSIGN_BUTTON_FALSE(maxcode++, "Superscope Pause");

        maxcode = 0x60;
        /*** Mouse ***/
        ASSIGN_BUTTON_FALSE(maxcode++, "Mouse1 L");
        ASSIGN_BUTTON_FALSE(maxcode++, "Mouse1 R");
        ASSIGN_BUTTON_FALSE(maxcode++, "Mouse2 L");
        ASSIGN_BUTTON_FALSE(maxcode++, "Mouse2 R");

        maxcode = 0x70;
        /*** Justifier ***/
        ASSIGN_BUTTON_FALSE(maxcode++, "Justifier1 Trigger");
        ASSIGN_BUTTON_FALSE(maxcode++, "Justifier1 AimOffscreen");
        ASSIGN_BUTTON_FALSE(maxcode++, "Justifier1 Start");
        ASSIGN_BUTTON_FALSE(maxcode++, "Justifier2 Trigger");
        ASSIGN_BUTTON_FALSE(maxcode++, "Justifier2 AimOffscreen");
        ASSIGN_BUTTON_FALSE(maxcode++, "Justifier2 Start");

        maxcode = 0x80;
        S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Superscope"), false);
        S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Mouse1"), false);
        S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Mouse2"), false);
        S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Justifier1"), false);
        S9xMapPointer(maxcode++, S9xGetCommandT("Pointer Justifier2"), false);

        maxcode = 0x90;
        //ASSIGN_BUTTON_FALSE (maxcode++, "Screenshot");

        SetControllers();
}



static int scopeTurbo = 0; // tracks whether superscope turbo is on or off
u32 btnmap[4][4][12]; // button mapping

void ResetControls(int consoleCtrl, int wiiCtrl) {
        int i;
        /*** Gamecube controller Padmap ***/
#if 0
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_GCPAD)) {
                i = 0;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_X;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_Y;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_TRIGGER_L;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_TRIGGER_R;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_START;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_TRIGGER_Z;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_UP;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_DOWN;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_LEFT;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_RIGHT;
        }
#else
         if (consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_GCPAD)) {
                i = 0;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_X;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_Y;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_TRIGGER_L;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_TRIGGER_R;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_START;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_BACK;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_UP;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_DOWN;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_LEFT;
                btnmap[CTRL_PAD][CTRLR_GCPAD][i++] = PAD_BUTTON_RIGHT;
        }
#endif
        /*** Wiimote Padmap ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_WIIMOTE)) {
                i = 0;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_B;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_2;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_1;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_A;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = 0x0000;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = 0x0000;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_PLUS;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_MINUS;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_RIGHT;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_LEFT;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_UP;
                btnmap[CTRL_PAD][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_DOWN;
        }

        /*** Classic Controller Padmap ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_CLASSIC)) {
                i = 0;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_A;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_B;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_X;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_Y;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_FULL_L;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_FULL_R;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_PLUS;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_MINUS;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_UP;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_DOWN;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_LEFT;
                btnmap[CTRL_PAD][CTRLR_CLASSIC][i++] = WPAD_CLASSIC_BUTTON_RIGHT;
        }

        /*** Nunchuk + wiimote Padmap ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_PAD && wiiCtrl == CTRLR_NUNCHUK)) {
                i = 0;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_A;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_B;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_NUNCHUK_BUTTON_C;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_NUNCHUK_BUTTON_Z;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_2;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_1;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_PLUS;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_MINUS;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_UP;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_DOWN;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_LEFT;
                btnmap[CTRL_PAD][CTRLR_NUNCHUK][i++] = WPAD_BUTTON_RIGHT;
        }

        /*** Superscope : GC controller button mapping ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_SCOPE && wiiCtrl == CTRLR_GCPAD)) {
                i = 0;
                btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
                btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
                btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_TRIGGER_Z;
                btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_Y;
                btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_X;
                btnmap[CTRL_SCOPE][CTRLR_GCPAD][i++] = PAD_BUTTON_START;
        }

        /*** Superscope : wiimote button mapping ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_SCOPE && wiiCtrl == CTRLR_WIIMOTE)) {
                i = 0;
                btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_B;
                btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_A;
                btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_MINUS;
                btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_UP;
                btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_DOWN;
                btnmap[CTRL_SCOPE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_PLUS;
        }

        /*** Mouse : GC controller button mapping ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_MOUSE && wiiCtrl == CTRLR_GCPAD)) {
                i = 0;
                btnmap[CTRL_MOUSE][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
                btnmap[CTRL_MOUSE][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
        }

        /*** Mouse : wiimote button mapping ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_MOUSE && wiiCtrl == CTRLR_WIIMOTE)) {
                i = 0;
                btnmap[CTRL_MOUSE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_A;
                btnmap[CTRL_MOUSE][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_B;
        }

        /*** Justifier : GC controller button mapping ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_JUST && wiiCtrl == CTRLR_GCPAD)) {
                i = 0;
                btnmap[CTRL_JUST][CTRLR_GCPAD][i++] = PAD_BUTTON_B;
                btnmap[CTRL_JUST][CTRLR_GCPAD][i++] = PAD_BUTTON_A;
                btnmap[CTRL_JUST][CTRLR_GCPAD][i++] = PAD_BUTTON_START;
        }

        /*** Justifier : wiimote button mapping ***/
        if (consoleCtrl == -1 || (consoleCtrl == CTRL_JUST && wiiCtrl == CTRLR_WIIMOTE)) {
                i = 0;
                btnmap[CTRL_JUST][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_B;
                btnmap[CTRL_JUST][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_A;
                btnmap[CTRL_JUST][CTRLR_WIIMOTE][i++] = WPAD_BUTTON_PLUS;
        }
}

/****************************************************************************
 * decodepad
 *
 * Reads the changes (buttons pressed, etc) from a controller and reports
 * these changes to Snes9x
 ***************************************************************************/
static void decodepad(int chan) {
        int i, offset;
        double angle;
        static const double THRES = 0.38268343236508984; // cos(67.5)

        s8 pad_x = userInput[chan].pad.stickX;
        s8 pad_y = userInput[chan].pad.stickY;
        u32 jp = userInput[chan].pad.btns_h;



        /***
        Gamecube Joystick input
         ***/
        // Is XY inside the "zone"?
        if (pad_x * pad_x + pad_y * pad_y > PADCAL * PADCAL) {
                angle = atan2(pad_y, pad_x);

                if (cos(angle) > THRES)
                        jp |= PAD_BUTTON_RIGHT;
                else if (cos(angle) < -THRES)
                        jp |= PAD_BUTTON_LEFT;
                if (sin(angle) > THRES)
                        jp |= PAD_BUTTON_UP;
                else if (sin(angle) < -THRES)
                        jp |= PAD_BUTTON_DOWN;
        }

        // Count as pressed if down far enough (~50% down)
        if (userInput[chan].pad.triggerL > 0x80)
                jp |= PAD_TRIGGER_L;
        if (userInput[chan].pad.triggerR > 0x80)
                jp |= PAD_TRIGGER_R;


        /*** Fix offset to pad ***/
        offset = ((chan + 1) << 4);

        /*** Report pressed buttons (gamepads) ***/
        for (i = 0; i < MAXJP; i++) {
                if ((jp & btnmap[CTRL_PAD][CTRLR_GCPAD][i]))
                        S9xReportButton(offset + i, true);
                else
                        S9xReportButton(offset + i, false);
        }

}

bool MenuRequested() {
        for (int i = 0; i < 4; i++) {
                if (
                        (userInput[i].pad.substickX < -70) ||
                        (userInput[i].pad.btns_h & PAD_TRIGGER_L &&
                        userInput[i].pad.btns_h & PAD_TRIGGER_R &&
                        userInput[i].pad.btns_h & PAD_BUTTON_X &&
                        userInput[i].pad.btns_h & PAD_BUTTON_Y
                        ) ||                        
                        userInput[i].pad.btns_h & PAD_BUTTON_LOGO
                        ) {
                        return true;
                }
        }
        return false;
}

/****************************************************************************
 * ReportButtons
 *
 * Called on each rendered frame
 * Our way of putting controller input into Snes9x
 ***************************************************************************/
void ReportButtons() {
        int i, j;

        UpdatePads();

        Settings.TurboMode = (
                userInput[0].pad.substickX > 70 ||
                userInput[0].WPAD_StickX(1) > 70
                ); // RIGHT on c-stick and on classic controller right joystick

        /* Check for menu:
         * CStick left
         * OR "L+R+X+Y" (eg. Homebrew/Adapted SNES controllers)
         * OR "Home" on the wiimote or classic controller
         * OR Left on classic right analog stick
         */
        if (MenuRequested())
                ScreenshotRequested = 1; // go to the menu

        j = (Settings.MultiPlayer5Master == true ? 4 : 2);

        for (i = 0; i < j; i++)
                decodepad(i);
}

void SetControllers() {
        if (Settings.MultiPlayer5Master == true) {
                S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
                S9xSetController(1, CTL_MP5, 1, 2, 3, -1);
        } else if (Settings.SuperScopeMaster == true) {
                S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
                S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
        } else if (Settings.MouseMaster == true) {
                S9xSetController(0, CTL_MOUSE, 0, 0, 0, 0);
                S9xSetController(1, CTL_JOYPAD, 1, 0, 0, 0);
        } else if (Settings.JustifierMaster == true) {
                S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
                S9xSetController(1, CTL_JUSTIFIER, 1, 0, 0, 0);
        } else {
                // Plugin 2 Joypads by default
                S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
                S9xSetController(1, CTL_JOYPAD, 1, 0, 0, 0);
        }
}