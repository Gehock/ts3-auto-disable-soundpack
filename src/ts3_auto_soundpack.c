#ifdef _WIN32
#define WINVER 0x0500
//#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"


static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 22

//#define COMMAND_BUFSIZE 128
//#define INFODATA_BUFSIZE 128
//#define SERVERINFO_BUFSIZE 256
//#define CHANNELINFO_BUFSIZE 512
//#define RETURNCODE_BUFSIZE 128

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if (WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif


/*************************************************************************************************************************/

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
#ifdef _WIN32
	/* TeamSpeak expects UTF-8 encoded characters. Following demonstrates a possibility how to convert UTF-16 wchar_t into UTF-8. */
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if (!result) {
		const wchar_t* name = L"Auto disable soundpack";
		if (wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
			result = "Auto disable soundpack";  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return "Auto disable soundpack";
#endif
}

/* Plugin version */
const char* ts3plugin_version() {
	return "0.3.3";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
	return "Gehock";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
	return "Disables the soundpack automatically when in TaskForceRadio channel.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

/*
* Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
* If the function returns 1 on failure, the plugin will be unloaded again.
*/
int ts3plugin_init() {

	/* Your plugin init code here */
	printf_s("PLUGIN: init auto soundpack\n");

	return 0;  /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
			   /* -2 is a very special case and should only be used if a plugin displays a dialog (e.g. overlay) asking the user to disable
			   * the plugin again, avoiding the show another dialog by the client telling the user the plugin failed to load.
			   * For normal case, if a plugin really failed to load because of an error, the correct return value is 1. */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
	/* Your plugin cleanup code here */
	printf_s("PLUGIN: shutdown auto soundpack\n");

	/*
	* Note:
	* If your plugin implements a settings dialog, it must be closed and deleted here, else the
	* TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	*/

}

/*
* Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
* the user manually disabled it in the plugin dialog.
* This function is optional. If missing, no autoload is assumed.
*/
int ts3plugin_requestAutoload() {
	return 1;  /* 1 = request autoloaded, 0 = do not request autoload */
}


void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
	char *chTo;
	char *chFrom;
	if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, newChannelID, CHANNEL_NAME, &chTo) != ERROR_ok) {
		printf_s("Error getting channel name move\n");
		return;
	}
	if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, oldChannelID, CHANNEL_NAME, &chFrom) != ERROR_ok) {
		printf_s("Error getting channel name move\n");
		return;
	}
	if (strcmp(chTo, "TaskForceRadio") == 0) {
		printf_s("Moved to Task Force\n");
		printf_s("Moved, pressed\n");
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		ip.ki.wVk = VK_CONTROL;
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));

		ip.ki.wVk = VK_F8;
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));

		// Release the "V" key
		ip.ki.wVk = VK_F8;
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));

		// Release the "Ctrl" key
		ip.ki.wVk = VK_CONTROL;
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
	} else if (strcmp(chFrom, "TaskForceRadio") == 0) {
		printf_s("Moved from Task Force\n");
		printf_s("Moved, pressed\n");
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		ip.ki.wVk = VK_CONTROL;
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));

		ip.ki.wVk = VK_F9;
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));

		// Release the "V" key
		ip.ki.wVk = VK_F9;
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));

		// Release the "Ctrl" key
		ip.ki.wVk = VK_CONTROL;
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
	}

	ts3Functions.freeMemory(chTo);
}
