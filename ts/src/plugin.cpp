/*
 * TeamSpeak 3 demo plugin
 *
 * Copyright (c) 2008-2013 TeamSpeak Systems GmbH
 */

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include "public_errors.h"
#include "public_errors_rare.h"
#include "public_definitions.h"
#include "public_rare_definitions.h"
#include "ts3_functions.h"
#include "plugin.h"

#define M_PI       3.14159265358979323846

#define PIPE_NAME L"\\\\.\\pipe\\task_force_radio_pipe"
#define PIPE_NAME L"\\\\.\\pipe\\task_force_radio_pipe_debug"
#define PLUGIN_NAME "task_force_radio"

struct CLIENT_DATA
{
	anyID clientId;
	bool tangentPressed;
	TS3_VECTOR clientPosition;
	uint64 positionTime;
};

typedef std::map<std::string, CLIENT_DATA> STRING_TO_CLIENT_DATA_MAP;
struct SERVER_RADIO_DATA 
{
	anyID myID;
	std::string myNickname;
	bool tangentPressed;
	TS3_VECTOR myPosition;
	STRING_TO_CLIENT_DATA_MAP nicknameToClientData;

	SERVER_RADIO_DATA()
	{
		myID = -1;
		tangentPressed = false;
	}
};
typedef std::map<uint64, SERVER_RADIO_DATA> SERVER_ID_TO_SERVER_DATA;

HANDLE thread = INVALID_HANDLE_VALUE;
bool exitThread = FALSE;
bool pipeConnected = false;
bool vadEnabled = false;
static char* pluginID = NULL;

CRITICAL_SECTION serverDataCriticalSection;
SERVER_ID_TO_SERVER_DATA serverIdToData;

static struct TS3Functions ts3Functions;

HANDLE openPipe() 
{
	HANDLE pipe = CreateFile(
			PIPE_NAME,
			GENERIC_READ, // only need read access
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	return pipe;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) 
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) 
	{
		elems.push_back(item);
	}
	return elems;
}

// taken from https://github.com/MadStyleCow/A2TS_Rebuild/blob/master/src/ts3plugin.cpp#L1367
BOOL hlp_checkVad()
{
	char* vad; // Is "true" or "false"
	if(ts3Functions.getPreProcessorConfigValue(ts3Functions.getCurrentServerConnectionHandlerID(), "vad", &vad) == ERROR_ok)
	{
		if(strcmp(vad,"true") == 0)
		{
			ts3Functions.freeMemory(vad);
			return TRUE;
		}
		else
		{
			ts3Functions.freeMemory(vad);
			return FALSE;
		}
	}
	else
	{
		printf("PLUGIN: Failed to get VAD value.\n");
		return FALSE;
	}
}

void hlp_enableVad()
{
	if(ts3Functions.setPreProcessorConfigValue(ts3Functions.getCurrentServerConnectionHandlerID(), "vad", "true") == ERROR_ok)
	{
		printf("PLUGIN: VAD succesfully enabled.\n");
	}
	else
	{
		printf("PLUGIN: Failure enabling VAD.\n");
	}
}

void hlp_disableVad()
{
	if(ts3Functions.setPreProcessorConfigValue(ts3Functions.getCurrentServerConnectionHandlerID(), "vad", "false") == ERROR_ok)
	{
		printf("PLUGIN: VAD succesfully disabled.\n");
	}
	else
	{
		printf("PLUGIN: Failure disabling VAD.\n");
	}
}


void updateDebugInfo() {		
	if(ts3Functions.setClientSelfVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), CLIENT_META_DATA, pipeConnected ? "Task Force Radio Connected to Arma 3 :)" : "Task Force Radio NOT Connected to Arma 3 :(") != ERROR_ok) {
		printf("Can't set own META_DATA");
	}	
}

std::vector<std::string> split(const std::string &s, char delim) 
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

bool hasClientData(uint64 serverConnectionHandlerID, anyID clientID)
{
	bool result = false;
	EnterCriticalSection(&serverDataCriticalSection);
	for (STRING_TO_CLIENT_DATA_MAP::iterator it = serverIdToData[serverConnectionHandlerID].nicknameToClientData.begin(); 
		it != serverIdToData[serverConnectionHandlerID].nicknameToClientData.end(); it++)
	{
		if (it->second.clientId == clientID)
		{
			result = true;
			break;
		}
	}
	LeaveCriticalSection(&serverDataCriticalSection);
	return result;
}

CLIENT_DATA getClientData(uint64 serverConnectionHandlerID, anyID clientID)
{
	CLIENT_DATA data = CLIENT_DATA();
	EnterCriticalSection(&serverDataCriticalSection);
	for (STRING_TO_CLIENT_DATA_MAP::iterator it = serverIdToData[serverConnectionHandlerID].nicknameToClientData.begin(); 
		it != serverIdToData[serverConnectionHandlerID].nicknameToClientData.end(); it++)
	{
		if (it->second.clientId == clientID)
		{
			data = it->second;			
			break;
		}
	}
	LeaveCriticalSection(&serverDataCriticalSection);
	return data;
}

std::vector<anyID> getChannelClients(uint64 serverConnectionHandlerID, uint64 channelId)
{
	std::vector<anyID> result;
	anyID* clients = NULL;
	int i = 0;		
	anyID* clientsCopy = clients;
	if (ts3Functions.getChannelClientList(serverConnectionHandlerID, channelId, &clients) == ERROR_ok) 
	{
		int i = 0;		
		anyID* clientsCopy = clients;		
		while (clients[i]) 		
		{			
			result.push_back(clients[i]);			
			i++;
		}		
		ts3Functions.freeMemory(clients);
	}
	return result;
}

anyID getMyId(uint64 serverConnectionHandlerID)
{
	anyID myID;
	if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok)
	{
		printf("DEBUG: Failure getting client ID. Error code %d\n");		
	}
	return myID;
}

uint64 getCurrentChannel(uint64 serverConnectionHandlerID)
{
	uint64 channelId;
	if (ts3Functions.getChannelOfClient(serverConnectionHandlerID, getMyId(serverConnectionHandlerID), &channelId) != ERROR_ok) 
	{
		printf("PLUGIN: Can't get current channel");		
	}
	return channelId;
}

void centerAll(uint64 serverConnectionId) 
{
	std::vector<anyID> clientsIds = getChannelClients(serverConnectionId, getCurrentChannel(serverConnectionId));
	anyID myId = getMyId(serverConnectionId);
	for (auto it = clientsIds.begin(); it != clientsIds.end(); it++)
	{
		TS3_VECTOR zero;
		zero.x = zero.y = zero.z = 0.0f;
		if (*it == getMyId(serverConnectionId))
		{			
			if (ts3Functions.systemset3DListenerAttributes(serverConnectionId, &zero, NULL, NULL) != ERROR_ok)
			{
				printf("can't center listener");
			}
		}
		else 
		{
			if (ts3Functions.channelset3DAttributes(serverConnectionId, *it, &zero) != ERROR_ok)
			{
				printf("can't center client");
			}
		}
	}
}

std::string getMyNickname(uint64 serverConnectionHandlerID)
{
	char* bufferForNickname;
	if(ts3Functions.getClientVariableAsString(serverConnectionHandlerID, getMyId(serverConnectionHandlerID), CLIENT_NICKNAME, &bufferForNickname) != ERROR_ok) {
		printf("Error getting client nickname\n");
		return "";
	}
	std::string result = std::string(bufferForNickname);
	ts3Functions.freeMemory(bufferForNickname);
	return result;
}


void processGameCommand(std::string command)
{
	std::vector<std::string> tokens = split(command, '@'); // may not be used in nickname
	if (tokens.size() == 6 && tokens[0] == "POS") 
	{
		std::string nickname = tokens[1];
		float x = std::stof(tokens[2]);
		float y = std::stof(tokens[3]);
		float z = std::stof(tokens[4]);
		float viewAngle = std::stof(tokens[5]);

		TS3_VECTOR position;
		position.x = x;
		position.y = z; // yes, it is correct
		position.z = y; // yes, it is correct		
		uint64 currentServerConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
		DWORD time = GetTickCount();
		EnterCriticalSection(&serverDataCriticalSection); 
		if (nickname == serverIdToData[currentServerConnectionHandlerID].myNickname) 
		{

			float radians = viewAngle * ((float) M_PI / 180.0f);
			TS3_VECTOR look;
			look.x = sin(radians);
			look.z = cos(radians);
			look.y = 0;

			serverIdToData[currentServerConnectionHandlerID].nicknameToClientData[nickname].clientId = serverIdToData[currentServerConnectionHandlerID].myID;
			serverIdToData[currentServerConnectionHandlerID].nicknameToClientData[nickname].clientPosition = position;
			serverIdToData[currentServerConnectionHandlerID].nicknameToClientData[nickname].positionTime = time;
			serverIdToData[currentServerConnectionHandlerID].myPosition = position;

			LeaveCriticalSection(&serverDataCriticalSection);
			DWORD errorCode = ts3Functions.systemset3DListenerAttributes(currentServerConnectionHandlerID, &position, &look, NULL);
			EnterCriticalSection(&serverDataCriticalSection); 
			if(errorCode != ERROR_ok)
			{
				printf("DEBUG: Failed to set own 3d position. Error code %d\n", errorCode);
			}	
			else
			{
				printf("DEBUG: OWN 3D POSITION SET.\n");
			}			
		} 
		else 
		{
			if (serverIdToData[currentServerConnectionHandlerID].nicknameToClientData.count(nickname))
			{
				serverIdToData[currentServerConnectionHandlerID].nicknameToClientData[nickname].clientPosition = position;				
				serverIdToData[currentServerConnectionHandlerID].nicknameToClientData[nickname].positionTime = time;
				LeaveCriticalSection(&serverDataCriticalSection);
				if (ts3Functions.channelset3DAttributes(currentServerConnectionHandlerID, serverIdToData[currentServerConnectionHandlerID].nicknameToClientData[nickname].clientId, &position) != ERROR_ok)
				{
					printf("Can't set client 3D position");
				}
				EnterCriticalSection(&serverDataCriticalSection);
				
			}			
		}				
		LeaveCriticalSection(&serverDataCriticalSection);
	} 
	else if (tokens.size() == 2 && tokens[0] == "TANGENT")
	{
		bool pressed = (tokens[1] == "PRESSED");
		uint64 serverId = ts3Functions.getCurrentServerConnectionHandlerID();

		bool changed = false;
		EnterCriticalSection(&serverDataCriticalSection);		
		changed = (serverIdToData[serverId].tangentPressed != pressed);
		serverIdToData[serverId].tangentPressed = pressed;
		LeaveCriticalSection(&serverDataCriticalSection);		
		if (changed)
		{			
			// broadcast info about tangent pressed over all client
			//std::string commandToBroadcast = command + "@" + "Archie Smith";
			std::string commandToBroadcast = command + "@" + serverIdToData[ts3Functions.getCurrentServerConnectionHandlerID()].myNickname;
			ts3Functions.sendPluginCommand(ts3Functions.getCurrentServerConnectionHandlerID(), pluginID, commandToBroadcast.c_str(), PluginCommandTarget_CURRENT_CHANNEL, NULL, NULL);			

			if (pressed)
			{
				vadEnabled = hlp_checkVad();
				hlp_disableVad();
			} 
			else
			{
				if (vadEnabled)	hlp_enableVad();				
			}
			if((ts3Functions.setClientSelfVariableAsInt(serverId, CLIENT_INPUT_DEACTIVATED, pressed ? INPUT_ACTIVE : INPUT_DEACTIVATED)) != ERROR_ok) {
				printf("Can't active talking by tangent");
			}
			DWORD error = ts3Functions.flushClientSelfUpdates(serverId, NULL) ; 
			if(error != ERROR_ok && error != ERROR_ok_no_update) {
				printf("Can't flush self updates");
			}
		}		
	}
}

void processOldPositions(uint64 serverConnectionHandlerID)
{
	DWORD time = GetTickCount();
	EnterCriticalSection(&serverDataCriticalSection);
	for (auto it = serverIdToData[serverConnectionHandlerID].nicknameToClientData.begin(); it != serverIdToData[serverConnectionHandlerID].nicknameToClientData.end(); it++)
	{
		if (time - it->second.positionTime > 5000) // 5 seconds without updates of client position
		{
			it->second.clientPosition = serverIdToData[serverConnectionHandlerID].myPosition;
			LeaveCriticalSection(&serverDataCriticalSection);
			if (ts3Functions.channelset3DAttributes(serverConnectionHandlerID, it->second.clientId, NULL) != ERROR_ok)
			{
				printf("Can't set default for user");
			}
			EnterCriticalSection(&serverDataCriticalSection);
		}
	}
	LeaveCriticalSection(&serverDataCriticalSection);
}

DWORD WINAPI PipeThread( LPVOID lpParam )
{
	HANDLE pipe = INVALID_HANDLE_VALUE;
	int errorCount = 0;
	while (!exitThread)
	{
		if (pipe == INVALID_HANDLE_VALUE) pipe = openPipe();		

		DWORD numBytesRead = 0;
		DWORD numBytesAvail = 0;
		bool sleep = true;
		if (PeekNamedPipe(pipe, NULL, 0, &numBytesRead, &numBytesAvail, NULL)) 
		{
			if (numBytesAvail > 0) 
			{					
				char buffer[4096];
				memset(buffer, 0, 4096);
		
				BOOL result = ReadFile(
							pipe,
							buffer, // the data from the pipe will be put here
							4096, // number of bytes allocated
							&numBytesRead, // this will store number of bytes actually read
							NULL // not using overlapped IO
						);
				if (result) {
					sleep = false;
					if (!pipeConnected)
					{
						pipeConnected = true;
						updateDebugInfo();
					}
					processGameCommand(std::string(buffer));
				} else {
					if (pipeConnected) 
					{
						pipeConnected = false;
						updateDebugInfo();
					}
					errorCount++;
					Sleep(1000);
					pipe = openPipe();					
				}		
			} 
		} 
		else 
		{
			errorCount++;
			if (pipeConnected) 
			{
				pipeConnected = false;				
				updateDebugInfo();
			}			
			Sleep(1000);
			pipe = openPipe();			
		}
		if (sleep) 
		{
			processOldPositions(ts3Functions.getCurrentServerConnectionHandlerID());
			Sleep(1);
		}
		if (errorCount > 10)
		{
			centerAll(ts3Functions.getCurrentServerConnectionHandlerID());
			errorCount = 0;
		}
	}	
	CloseHandle(pipe);
	pipe = INVALID_HANDLE_VALUE;
	return NULL;
}

void updateNicknamesList(uint64 serverConnectionHandlerID) {	

	std::vector<anyID> clients = getChannelClients(serverConnectionHandlerID, getCurrentChannel(serverConnectionHandlerID));
	for (auto clientIdIterator = clients.begin(); clientIdIterator != clients.end(); clientIdIterator++)
	{
		anyID clientId = *clientIdIterator;		
		char* name;
		if(ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientId, CLIENT_NICKNAME, &name) != ERROR_ok) {
			printf("Error getting client nickname\n");
			continue;
		} 
		else 
		{
			EnterCriticalSection(&serverDataCriticalSection);			
			std::string clientNickname(name);
			if (!serverIdToData[serverConnectionHandlerID].nicknameToClientData.count(clientNickname))
			{
				serverIdToData[serverConnectionHandlerID].nicknameToClientData[clientNickname] = CLIENT_DATA();
			}
			serverIdToData[serverConnectionHandlerID].nicknameToClientData[clientNickname].clientId = clientId;
			LeaveCriticalSection(&serverDataCriticalSection);

			ts3Functions.freeMemory(name);
		}					
	}
	std::string myNickname = getMyNickname(serverConnectionHandlerID);
	EnterCriticalSection(&serverDataCriticalSection);			
	serverIdToData[serverConnectionHandlerID].myNickname = myNickname;
	LeaveCriticalSection(&serverDataCriticalSection);

}

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 19

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128


#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {	
#ifdef _WIN32
	/* TeamSpeak expects UTF-8 encoded characters. Following demonstrates a possibility how to convert UTF-16 wchar_t into UTF-8. */
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if(!result) {
		const wchar_t* name = L"Task Force Arma 3 Radio";
		if(wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
			result = "Task Force Arma 3 Radio";  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return "Task Force Arma 3 Radio";
#endif
}

/* Plugin version */
const char* ts3plugin_version() {
    return "0.1";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "[TF]Nkey";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "Simple Radio Addon for Arma 3 for Task Force 141 squad";
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
    char appPath[PATH_BUFSIZE];
    char resourcesPath[PATH_BUFSIZE];
    char configPath[PATH_BUFSIZE];
	char pluginPath[PATH_BUFSIZE];

    /* Your plugin init code here */
    printf("PLUGIN: init\n");

    /* Example on how to query application, resources and configuration paths from client */
    /* Note: Console client returns empty string for app and resources path */
    ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
    ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
    ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE);


	InitializeCriticalSection(&serverDataCriticalSection);

	exitThread = FALSE;
	thread = CreateThread(NULL, 0, PipeThread, NULL, 0, NULL);

	printf("PLUGIN: App path: %s\nResources path: %s\nConfig path: %s\nPlugin path: %s\n", appPath, resourcesPath, configPath, pluginPath);

	centerAll(ts3Functions.getCurrentServerConnectionHandlerID());
	updateNicknamesList(ts3Functions.getCurrentServerConnectionHandlerID());


    return 0;  /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
	/* -2 is a very special case and should only be used if a plugin displays a dialog (e.g. overlay) asking the user to disable
	 * the plugin again, avoiding the show another dialog by the client telling the user the plugin failed to load.
	 * For normal case, if a plugin really failed to load because of an error, the correct return value is 1. */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
    /* Your plugin cleanup code here */
    printf("PLUGIN: shutdown\n");
	exitThread = TRUE;
	Sleep(1000);
	DWORD exitCode;
	BOOL result = GetExitCodeThread(thread, &exitCode);
	if (!result || exitCode == STILL_ACTIVE) 
	{
		printf("PLUGIN: thread not terminated");
	}
	thread = INVALID_HANDLE_VALUE;
	centerAll(ts3Functions.getCurrentServerConnectionHandlerID());
	exitThread = FALSE;

	/*
	 * Note:
	 * If your plugin implements a settings dialog, it must be closed and deleted here, else the
	 * TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	 */

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure() {
	printf("PLUGIN: offersConfigure\n");
	/*
	 * Return values:
	 * PLUGIN_OFFERS_NO_CONFIGURE         - Plugin does not implement ts3plugin_configure
	 * PLUGIN_OFFERS_CONFIGURE_NEW_THREAD - Plugin does implement ts3plugin_configure and requests to run this function in an own thread
	 * PLUGIN_OFFERS_CONFIGURE_QT_THREAD  - Plugin does implement ts3plugin_configure and requests to run this function in the Qt GUI thread
	 */
	return PLUGIN_OFFERS_NO_CONFIGURE;  /* In this case ts3plugin_configure does not need to be implemented */
}

/* Plugin might offer a configuration window. If ts3plugin_offersConfigure returns 0, this function does not need to be implemented. */
void ts3plugin_configure(void* handle, void* qParentWidget) {
    printf("PLUGIN: configure\n");
}

/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
	printf("PLUGIN: registerPluginID: %s\n", pluginID);
}

/* Plugin command keyword. Return NULL or "" if not used. */
const char* ts3plugin_commandKeyword() {
	return "";
}

/* Plugin processes console command. Return 0 if plugin handled the command, 1 if not handled. */
int ts3plugin_processCommand(uint64 serverConnectionHandlerID, const char* command) {
	return 0;  /* Plugin handled command */
}

/* Client changed current server connection handler */
void ts3plugin_currentServerConnectionChanged(uint64 serverConnectionHandlerID) {
    printf("PLUGIN: currentServerConnectionChanged %llu (%llu)\n", (long long unsigned int)serverConnectionHandlerID, (long long unsigned int)ts3Functions.getCurrentServerConnectionHandlerID());
}

/*
 * Implement the following three functions when the plugin should display a line in the server/channel/client info.
 * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
 */

/* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle() {
	return "Task Force Radio Status";
}

/*
 * Dynamic content shown in the right column in the info frame. Memory for the data string needs to be allocated in this
 * function. The client will call ts3plugin_freeMemory once done with the string to release the allocated memory again.
 * Check the parameter "type" if you want to implement this feature only for specific item types. Set the parameter
 * "data" to NULL to have the client ignore the info data.
 */
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {
	char* name;

	/* For demonstration purpose, display the name of the currently selected server, channel or client. */
	switch(type) {
		case PLUGIN_SERVER:
			if(ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &name) != ERROR_ok) {
				printf("Error getting virtual server name\n");
				return;
			}
			break;
		case PLUGIN_CHANNEL:
			if(ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_NAME, &name) != ERROR_ok) {
				printf("Error getting channel name\n");
				return;
			}
			break;
		case PLUGIN_CLIENT:
			if(ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_META_DATA, &name) != ERROR_ok) {
				printf("Error getting client nickname\n");
				return;
			}
			break;
		default:
			printf("Invalid item type: %d\n", type);
			data = NULL;  /* Ignore */
			return;
	}

	*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  /* Must be allocated in the plugin! */
	snprintf(*data, INFODATA_BUFSIZE, "INFO: [I]\"%s\"[/I]", name);  /* bbCode is supported. HTML is not supported */
	ts3Functions.freeMemory(name);
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
	free(data);
}

/*
 * Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
 * the user manually disabled it in the plugin dialog.
 * This function is optional. If missing, no autoload is assumed.
 */
int ts3plugin_requestAutoload() {
	return 0;  /* 1 = request autoloaded, 0 = do not request autoload */
}

/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}

/*
 * Initialize plugin menus.
 * This function is called after ts3plugin_init and ts3plugin_registerPluginID. A pluginID is required for plugin menus to work.
 * Both ts3plugin_registerPluginID and ts3plugin_freeMemory must be implemented to use menus.
 * If plugin menus are not used by a plugin, do not implement this function or return NULL.
 */
void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
}

/* Helper function to create a hotkey */
static struct PluginHotkey* createHotkey(const char* keyword, const char* description) {
	struct PluginHotkey* hotkey = (struct PluginHotkey*)malloc(sizeof(struct PluginHotkey));
	_strcpy(hotkey->keyword, PLUGIN_HOTKEY_BUFSZ, keyword);
	_strcpy(hotkey->description, PLUGIN_HOTKEY_BUFSZ, description);
	return hotkey;
}

/* Some makros to make the code to create hotkeys a bit more readable */
#define BEGIN_CREATE_HOTKEYS(x) const size_t sz = x + 1; size_t n = 0; *hotkeys = (struct PluginHotkey**)malloc(sizeof(struct PluginHotkey*) * sz);
#define CREATE_HOTKEY(a, b) (*hotkeys)[n++] = createHotkey(a, b);
#define END_CREATE_HOTKEYS (*hotkeys)[n++] = NULL; assert(n == sz);

/*
 * Initialize plugin hotkeys. If your plugin does not use this feature, this function can be omitted.
 * Hotkeys require ts3plugin_registerPluginID and ts3plugin_freeMemory to be implemented.
 * This function is automatically called by the client after ts3plugin_init.
 */
void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys) {
	/* Register hotkeys giving a keyword and a description.
	 * The keyword will be later passed to ts3plugin_onHotkeyEvent to identify which hotkey was triggered.
	 * The description is shown in the clients hotkey dialog. */
	BEGIN_CREATE_HOTKEYS(3);  /* Create 3 hotkeys. Size must be correct for allocating memory. */
	CREATE_HOTKEY("keyword_1", "Test hotkey 1");
	CREATE_HOTKEY("keyword_2", "Test hotkey 2");
	CREATE_HOTKEY("keyword_3", "Test hotkey 3");
	END_CREATE_HOTKEYS;

	/* The client will call ts3plugin_freeMemory to release all allocated memory */
}

/************************** TeamSpeak callbacks ***************************/
/*
 * Following functions are optional, feel free to remove unused callbacks.
 * See the clientlib documentation for details on each function.
 */

/* Clientlib */


void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
    /* Some example code following to show how to use the information query functions. */
	unsigned int errorCode;
	if(newStatus == STATUS_CONNECTION_ESTABLISHED)
	{	
		std::string myNickname = getMyNickname(serverConnectionHandlerID);
		anyID myId = getMyId(serverConnectionHandlerID);
		EnterCriticalSection(&serverDataCriticalSection);

		serverIdToData[serverConnectionHandlerID] = SERVER_RADIO_DATA();
		serverIdToData[serverConnectionHandlerID].myID = myId;				
		serverIdToData[serverConnectionHandlerID].myNickname = myNickname;

		LeaveCriticalSection(&serverDataCriticalSection);
	
		
		updateNicknamesList(serverConnectionHandlerID);

		// Set system 3d settings
		errorCode = ts3Functions.systemset3DSettings(serverConnectionHandlerID, 1.0f, 1.0f);
		if(errorCode != ERROR_ok)
		{
			printf("DEBUG: Failed to set 3d settings. Error code %d\n",errorCode);
		}
		else
		{
			printf("DEBUG: 3d settings set.\n");
		}
	} 
	else if (newStatus == STATUS_DISCONNECTED)
	{
		serverIdToData.erase(serverConnectionHandlerID);
	}
}

void ts3plugin_onNewChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID) {
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onNewChannelCreatedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onDelChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onChannelMoveEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 newChannelParentID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onUpdateChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onUpdateChannelEditedEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onUpdateClientEvent(uint64 serverConnectionHandlerID, anyID clientID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {	
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onClientMoveSubscriptionEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility) {
}

void ts3plugin_onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* timeoutMessage) {
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
	updateNicknamesList(serverConnectionHandlerID);
}

void ts3plugin_onClientIDsEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, anyID clientID, const char* clientName) {
}

void ts3plugin_onClientIDsFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerEditedEvent(uint64 serverConnectionHandlerID, anyID editerID, const char* editerName, const char* editerUniqueIdentifier) {
}

void ts3plugin_onServerUpdatedEvent(uint64 serverConnectionHandlerID) {
}

int ts3plugin_onServerErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, const char* extraMessage) {
	printf("PLUGIN: onServerErrorEvent %llu %s %d %s\n", (long long unsigned int)serverConnectionHandlerID, errorMessage, error, (returnCode ? returnCode : ""));
	if(returnCode) {
		/* A plugin could now check the returnCode with previously (when calling a function) remembered returnCodes and react accordingly */
		/* In case of using a a plugin return code, the plugin can return:
		 * 0: Client will continue handling this error (print to chat tab)
		 * 1: Client will ignore this error, the plugin announces it has handled it */
		return 1;
	}
	return 0;  /* If no plugin return code was used, the return value of this function is ignored */
}

void ts3plugin_onServerStopEvent(uint64 serverConnectionHandlerID, const char* shutdownMessage) {
}

int ts3plugin_onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID, const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored) {
    printf("PLUGIN: onTextMessageEvent %llu %d %d %s %s %d\n", (long long unsigned int)serverConnectionHandlerID, targetMode, fromID, fromName, message, ffIgnored);
    return 0;  /* 0 = handle normally, 1 = client will ignore the text message */
}

void ts3plugin_onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID) {	
}

void ts3plugin_onConnectionInfoEvent(uint64 serverConnectionHandlerID, anyID clientID) {
}

void ts3plugin_onServerConnectionInfoEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelSubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelSubscribeFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelUnsubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelUnsubscribeFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelDescriptionUpdateEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelPasswordChangedEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onPlaybackShutdownCompleteEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onSoundDeviceListChangedEvent(const char* modeID, int playOrCap) {
}

// part taken from: https://github.com/MadStyleCow/A2TS_Rebuild/blob/master/src/ts3plugin.cpp#L1772
void RadioNoiseDSP(float slevel, short * samples, int channels, int sampleCount)
{
	float l = 1 / (slevel+0.1f) / 64;	
	// 3d sound to mono
	for (int i = 0; i < sampleCount * 2; i+= channels)
	{
		short no3D = 0;
		for (int j = 0; j < channels; j++)
		{
			no3D += samples[i + j];
		}
		for (int j = 0; j < channels; j++)
		{
			samples[i + j] += no3D;
		}		
	}
	
	for (int i = 0; i < sampleCount * channels; i++)
	{
		float d = (float)samples[i]/SHRT_MAX;
		float pdl = 0.0f;
		float pdh1 = 0.0f;
		float pdh2 = 0.0f;
		float k = (((float)rand()/RAND_MAX)*(1-slevel)*2+1);
		float n = ((float)rand()/RAND_MAX)*2-1;
		d *= k;
		//noise
		d += n*l;
		if (i > 0)
		{
			d = pdl + 0.15 * (d - pdl);
			pdl = d;
		}
		else
		{
			pdl = d;
		}
		if (i > 0)
		{
			float pd = d;
			d = 0.85 * (pdh1 + d - pdh2);
			pdh1 = d;
			pdh2 = pd;
		}
		else
		{
			pdh1 = d;
			pdh2 = d;
		}
		d *= 2;
		if (d > 1)
			d = 1;
		else if (d < -1)
			d = -1;
		samples[i] = d*(SHRT_MAX-1);
	}
}

void ts3plugin_onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels) {	
}

void ts3plugin_onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {
	if (hasClientData(serverConnectionHandlerID, clientID))
	{
		bool overRadio = getClientData(serverConnectionHandlerID, clientID).tangentPressed;	
		if (overRadio)
		{		
			RadioNoiseDSP(1.0, samples, channels, sampleCount);
		}
	}
}

void ts3plugin_onEditMixedPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {
}

void ts3plugin_onEditCapturedVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited) {
}

void ts3plugin_onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume) {	
	if (hasClientData(serverConnectionHandlerID, clientID))
	{
		CLIENT_DATA data = getClientData(serverConnectionHandlerID, clientID);
		if (!data.tangentPressed)
		{
			float calculatedVolume = 1.0f - distance * 0.05f; // ~ 20 meters of hearing range.
			if (calculatedVolume < 0.0f)  calculatedVolume = 0.0f;
			*volume = calculatedVolume;			
		} 
		else 
		{
			*volume = 1.0f;
		}	
	}
}


void ts3plugin_onCustom3dRolloffCalculationWaveEvent(uint64 serverConnectionHandlerID, uint64 waveHandle, float distance, float* volume) {
}

void ts3plugin_onUserLoggingMessageEvent(const char* logMessage, int logLevel, const char* logChannel, uint64 logID, const char* logTime, const char* completeLogString) {
}

/* Clientlib rare */

void ts3plugin_onClientBanFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, uint64 time, const char* kickMessage) {
}

int ts3plugin_onClientPokeEvent(uint64 serverConnectionHandlerID, anyID fromClientID, const char* pokerName, const char* pokerUniqueIdentity, const char* message, int ffIgnored) {
    return 0;  /* 0 = handle normally, 1 = client will ignore the poke */
}

void ts3plugin_onClientSelfVariableUpdateEvent(uint64 serverConnectionHandlerID, int flag, const char* oldValue, const char* newValue) {
}

void ts3plugin_onFileListEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path, const char* name, uint64 size, uint64 datetime, int type, uint64 incompletesize, const char* returnCode) {
}

void ts3plugin_onFileListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path) {
}

void ts3plugin_onFileInfoEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* name, uint64 size, uint64 datetime) {
}

void ts3plugin_onServerGroupListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, const char* name, int type, int iconID, int saveDB) {
}

void ts3plugin_onServerGroupListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
}

void ts3plugin_onServerGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onServerGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID) {
}

void ts3plugin_onServerGroupClientListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, uint64 clientDatabaseID, const char* clientNameIdentifier, const char* clientUniqueID) {
}

void ts3plugin_onChannelGroupListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, const char* name, int type, int iconID, int saveDB) {
}

void ts3plugin_onChannelGroupListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID) {
}

void ts3plugin_onChannelPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onClientPermListEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID) {
}

void ts3plugin_onChannelClientPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID) {
}

void ts3plugin_onClientChannelGroupChangedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, uint64 channelID, anyID clientID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

int ts3plugin_onServerPermissionErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, unsigned int failedPermissionID) {
	return 0;  /* See onServerErrorEvent for return code description */
}

void ts3plugin_onPermissionListGroupEndIDEvent(uint64 serverConnectionHandlerID, unsigned int groupEndID) {
}

void ts3plugin_onPermissionListEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, const char* permissionName, const char* permissionDescription) {
}

void ts3plugin_onPermissionListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onPermissionOverviewEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, uint64 channelID, int overviewType, uint64 overviewID1, uint64 overviewID2, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onPermissionOverviewFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

void ts3plugin_onClientNeededPermissionsEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, int permissionValue) {
}

void ts3plugin_onClientNeededPermissionsFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onFileTransferStatusEvent(anyID transferID, unsigned int status, const char* statusMessage, uint64 remotefileSize, uint64 serverConnectionHandlerID) {
}

void ts3plugin_onClientChatClosedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity) {
}

void ts3plugin_onClientChatComposingEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity) {
}

void ts3plugin_onServerLogEvent(uint64 serverConnectionHandlerID, const char* logMsg) {
}

void ts3plugin_onServerLogFinishedEvent(uint64 serverConnectionHandlerID, uint64 lastPos, uint64 fileSize) {
}

void ts3plugin_onMessageListEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, uint64 timestamp, int flagRead) {
}

void ts3plugin_onMessageGetEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, const char* message, uint64 timestamp) {
}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {
}

void ts3plugin_onClientNamefromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {
}

void ts3plugin_onClientNamefromDBIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {
}

void ts3plugin_onComplainListEvent(uint64 serverConnectionHandlerID, uint64 targetClientDatabaseID, const char* targetClientNickName, uint64 fromClientDatabaseID, const char* fromClientNickName, const char* complainReason, uint64 timestamp) {
}

void ts3plugin_onBanListEvent(uint64 serverConnectionHandlerID, uint64 banid, const char* ip, const char* name, const char* uid, uint64 creationTime, uint64 durationTime, const char* invokerName,
							  uint64 invokercldbid, const char* invokeruid, const char* reason, int numberOfEnforcements, const char* lastNickName) {
}

void ts3plugin_onClientServerQueryLoginPasswordEvent(uint64 serverConnectionHandlerID, const char* loginPassword) {
}

void processPluginCommand(std::string command)
{
	std::vector<std::string> tokens = split(command, '@'); // may not be used in nickname
	if (tokens.size() == 3 && tokens[0] == "TANGENT")
	{
		bool pressed = (tokens[1] == "PRESSED");
		std::string nickname = tokens[2];
		uint64 serverId = ts3Functions.getCurrentServerConnectionHandlerID();

		EnterCriticalSection(&serverDataCriticalSection);
		CLIENT_DATA clientData = serverIdToData[serverId].nicknameToClientData[nickname];
		TS3_VECTOR myPosition = serverIdToData[serverId].myPosition;
		
		if (serverIdToData.count(serverId)) 
		{
			if (nickname != serverIdToData[serverId].myNickname) // ignore command from yourself
			{
				serverIdToData[serverId].nicknameToClientData[nickname].tangentPressed = pressed;
			}
		}		
		LeaveCriticalSection(&serverDataCriticalSection);
	}
}

void ts3plugin_onPluginCommandEvent(uint64 serverConnectionHandlerID, const char* pluginName, const char* pluginCommand) {
	printf("ON PLUGIN COMMAND: %s %s\n", pluginName, pluginCommand);
	if(serverConnectionHandlerID == ts3Functions.getCurrentServerConnectionHandlerID())
	{
		if(strcmp(pluginName, PLUGIN_NAME) != 0)
		{
			printf("PLUGIN: Plugin command event failure.\n");
		}
		else
		{
			
			processPluginCommand( std::string(pluginCommand));
		}
	}
}

void ts3plugin_onIncomingClientQueryEvent(uint64 serverConnectionHandlerID, const char* commandText) {
}

void ts3plugin_onServerTemporaryPasswordListEvent(uint64 serverConnectionHandlerID, const char* clientNickname, const char* uniqueClientIdentifier, const char* description, const char* password, uint64 timestampStart, uint64 timestampEnd, uint64 targetChannelID, const char* targetChannelPW) {
}

/* Client UI callbacks */

/*
 * Called from client when an avatar image has been downloaded to or deleted from cache.
 * This callback can be called spontaneously or in response to ts3Functions.getAvatar()
 */
void ts3plugin_onAvatarUpdated(uint64 serverConnectionHandlerID, anyID clientID, const char* avatarPath) {
	/* If avatarPath is NULL, the avatar got deleted */
	/* If not NULL, avatarPath contains the path to the avatar file in the TS3Client cache */
	if(avatarPath != NULL) {
		printf("onAvatarUpdated: %llu %d %s\n", (long long unsigned int)serverConnectionHandlerID, clientID, avatarPath);
	} else {
		printf("onAvatarUpdated: %llu %d - deleted\n", (long long unsigned int)serverConnectionHandlerID, clientID);
	}
}

/*
 * Called when a plugin menu item (see ts3plugin_initMenus) is triggered. Optional function, when not using plugin menus, do not implement this.
 * 
 * Parameters:
 * - serverConnectionHandlerID: ID of the current server tab
 * - type: Type of the menu (PLUGIN_MENU_TYPE_CHANNEL, PLUGIN_MENU_TYPE_CLIENT or PLUGIN_MENU_TYPE_GLOBAL)
 * - menuItemID: Id used when creating the menu item
 * - selectedItemID: Channel or Client ID in the case of PLUGIN_MENU_TYPE_CHANNEL and PLUGIN_MENU_TYPE_CLIENT. 0 for PLUGIN_MENU_TYPE_GLOBAL.
 */
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {	
}

/* This function is called if a plugin hotkey was pressed. Omit if hotkeys are unused. */
void ts3plugin_onHotkeyEvent(const char* keyword) {
	printf("PLUGIN: Hotkey event: %s\n", keyword);
	/* Identify the hotkey by keyword ("keyword_1", "keyword_2" or "keyword_3" in this example) and handle here... */
}

/* Called when recording a hotkey has finished after calling ts3Functions.requestHotkeyInputDialog */
void ts3plugin_onHotkeyRecordedEvent(const char* keyword, const char* key) {
}
