/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * Tantric 2008-2010
 *
 * networkop.cpp
 *
 * Network and SMB support routines
 ****************************************************************************/

#include <network/network.h>
#include <malloc.h>
#include <smbfs.h>
#include <mxml.h>

#include "snes9xgx.h"
#include "menu.h"
#include "fileop.h"
#include "filebrowser.h"

static bool networkInit = false;
static bool networkShareInit = false;
char wiiIP[16] = {0};

bool InitializeNetwork(bool silent) {
#ifndef NETPLAY_SUPPORT
	if (networkInit)
		return true;

	int retry = 1;

	while (retry) {
		ShowAction("Initializing network...");

		network_init();
		networkInit = 1;

		CancelAction();

		if (networkInit || silent)
			break;

		retry = ErrorPromptRetry("Unable to initialize network!");

		if (networkInit)
			return true;
	}
#endif
	return true;
}

void CloseShare() {
	//	if(networkShareInit)
	//		smbClose("smb");
	networkShareInit = false;
	isMounted[DEVICE_SMB] = false;
}

/****************************************************************************
 * Mount SMB Share
 ****************************************************************************/

bool
ConnectShare(bool silent) {
	if (!InitializeNetwork(silent))
		return false;

	if (networkShareInit)
		return true;

	int retry = 1;
	int chkS = (strlen(GCSettings.smbshare) > 0) ? 0 : 1;
	int chkI = (strlen(GCSettings.smbip) > 0) ? 0 : 1;

	// check that all parameters have been set
	if (chkS + chkI > 0) {
		if (!silent) {
			char msg[50];
			char msg2[100];
			if (chkS + chkI > 1) // more than one thing is wrong
				sprintf(msg, "Check settings.xml.");
			else if (chkS)
				sprintf(msg, "Share name is blank.");
			else if (chkI)
				sprintf(msg, "Share IP is blank.");

			sprintf(msg2, "Invalid network settings - %s", msg);
			ErrorPrompt(msg2);
		}
		return false;
	}

	while (retry) {
		if (!silent)
			ShowAction("Connecting to network share...");

		if (smbInit("smb", GCSettings.smbip, 445, GCSettings.smbshare, GCSettings.smbuser, GCSettings.smbpwd))
			networkShareInit = true;

		if (networkShareInit || silent)
			break;

		retry = ErrorPromptRetry("Failed to connect to network share.");
	}

	if (!silent)
		CancelAction();

	return networkShareInit;
}
