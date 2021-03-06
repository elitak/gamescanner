
#include "stdafx.h"
#include <vector>
#include <string>
#include <map>
#include <strsafe.h>
#define _DEFINE_DEPRECATED_HASH_CLASSES 0
#include <hash_map>
#include <Winsock2.h.>
#include "resource.h"

#ifndef __ETSV__
#define __ETSV__

#pragma pack(1)

using namespace stdext;
using namespace std;

#define XML_READ_OK		0
#define XML_READ_ERROR	1

#define MAX_PACKETS 1000

#define QW_ENGINE 100
#define Q3_ENGINE 101
#define Q4_ENGINE 102
#define VALVE_ENGINE 103
#define GAMESPYv4_ENGINE 104
#define COD4_ENGINE 105
#define WOLF_ENGINE 106
#define CODWW_ENGINE 107
#define BFBC2_ENGINE 108

#define MAX_MASTERS_SERVERS 10

#define EXPORT_FLAG_ALL					1
#define EXPORT_FLAG_FAVORITES			2
#define EXPORT_FLAG_PRIVATE_PASSWORD	4
#define EXPORT_FLAG_RCON_PASSWORD		8
#define EXPORT_PLAIN_TEXT				16  //plain vanilla text file ip:port format
#define EXPORT_GAMESCANNER_SL_FILE		32  //Game scanner exported server list


#define SEARCH_PLAYER 1
#define SEARCH_SERVERNAME 2
#define SEARCH_IP 3
#define SEARCH_RULE 4

#define TIMER_EVENT					1001
#define TIMER_EVENT_RSS				1002
#define IDT_1SECOND					1003
#define IDT_DETECT_GAME				1004
#define IDT_MONITOR_QUERY_SERVERS	1005
#define IDT_AUTO_REFRESH_TIMER		1006

#define MAX_VAR_LEN 100
#define MAX_NAME_LEN 100
#define MAX_IP_LEN 80
#define MAX_VAR MAX_VAR_LEN
#define MAX_COUNTRYNAME_LEN 50
#define MAX_MODNAME_LEN 25
#define MAX_MAPNAME_LEN 40
#define MAX_RCON_LEN 40
#define MAX_PASSWORD_LEN 40
#define MAX_VERSION_LEN 50
#define MAX_GAMETYPENAME_LEN 30
#define MAX_STATUS_LEN 40
#define MAX_FS_GAME_LEN 40

#define MAX_BUDDYNAME_LEN 40

#define	WM_USER_SHELLICON		WM_USER + 13
#define SHELL_CALLBACK			WM_USER+14
#define WM_REFRESHSERVERLIST	WM_USER+15
//#define WM_INITVIEWS			WM_USER+16
#define WM_START_PING			WM_USER+17
#define WM_STOP_PING			WM_USER+18
#define WM_GETSERVERLIST_START			WM_USER+19
#define WM_GETSERVERLIST_STOP			WM_USER+20
#define WM_UPDATESERVERLIST_COLUMNS		WM_USER+21
#define WM_START_TRACERT		WM_USER+22
#define WM_DETECT_GAME_RUNNING		WM_USER+23
#define WM_REINIT_COUNTRYFILTER WM_USER+24
#define WM_SAVE_SERVERLIST_COLUMN_STATE WM_USER+25
#define WM_REPAINT_ALL_WINDOWS WM_USER+26

#define WIN_MAINTREEVIEW	0
#define WIN_SERVERLIST		1
#define WIN_BUDDYLIST		2
#define WIN_TABCONTROL		3
#define WIN_MAPPREVIEW		4
#define WIN_PLAYERS			5
#define WIN_RCON			6
#define WIN_RULES			7
#define WIN_LOGGER			8
#define WIN_STATUS			9
#define WIN_PROGRESSBAR		10
#define WIN_PING			11
#define WIN_MAIN			12
#define WIN_MAX				13  //always increase

#define ICO_INFO		8
#define ICO_WARNING		6
#define ICO_EMPTY		-1

#define TOOLBAR_Y_OFFSET 27  //35
#define STATUSBAR_Y_OFFSET 25
#define TABSIZE_Y 22

//#define MAX_VAR 80
//#define MAX_ONLINE_USER 60
#define HOTKEY_ID 40022
#define HOTKEY_ID_CTRL_C 40099

#define WS_EX_LAYERED	0x00080000
#define LWA_COLORKEY	0x00000001
#define LWA_ALPHA		0x00000002
#define ULW_COLORKEY    0x00000001
#define ULW_ALPHA		0x00000002
#define ULW_OPAQUE      0x00000004

typedef BOOL(WINAPI *SLWA)(HWND, COLORREF, BYTE, DWORD);

#define MIN_TRANSPARANCY 30

#define FAVORITE			998
#define PRIVATE				999

#define REDRAWLIST_FAVORITES_PUBLIC		0x00000001
#define REDRAWLIST_FAVORITES_PRIVATE	0x00000002
#define REDRAWLIST_HISTORY				0x00000004
#define FORCE_SCAN_FILTERED				0x00000008
#define REDRAW_SERVERLIST				16
#define SCAN_SERVERLIST					32
#define REDRAW_MONITOR_SERVERS			64		
#define REDRAWLIST_CUSTOM_FILTER		128


#define ET_SERVERLIST		0
#define ETQW_SERVERLIST		1
#define Q3_SERVERLIST		2
#define Q4_SERVERLIST		3
#define RTCW_SERVERLIST		4
#define COD_SERVERLIST		5
#define COD2_SERVERLIST		6
#define WARSOW_SERVERLIST	7
#define COD4_SERVERLIST		8
#define CS_SERVERLIST		9
#define CSCZ_SERVERLIST		10
#define CSS_SERVERLIST		11
#define QW_SERVERLIST		12
#define Q2_SERVERLIST		13
#define OPENARENA_SERVERLIST 14
#define HL2_SERVERLIST		15			//Half-Life 2
#define UTERROR_SERVERLIST  16
#define HL_SERVERLIST		18	
#define NEXUIZ_SERVERLIST	17
#define DOOM3_SERVERLIST	19
#define COD5_SERVERLIST		20
#define UT3_SERVERLIST		21
#define CODUO_SERVERLIST	22
#define WOLF_SERVERLIST		23

#define REDRAW_CURRENT_LIST				100
#define SHOW_FAVORITES_PUBLIC			101
#define SHOW_FAVORITES_PRIVATE			102
#define SHOW_HISTORY					103
#define SHOW_MONITOR					104
#define SHOW_CUSTOM_FAVORITE_FILTER					105

#define DO_NOTHING				999
#define DO_NOTHING_				0
#define DO_REDRAW_SERVERLIST	1

#define DO_HISTORY				7
#define DO_GLOBAL_FILTER_		8
#define DO_GAME_SPECIFIC_FILTER		9
#define DO_COUNTRY_FILTER		10
#define DO_GLOBAL_EDIT_FILTER	11
#define DO_MONITOR						12

#define GLOBAL_FILTER				-1

#define DO_CUSTOM_MODIFIER				13
#define DO_CUSTOM_FAVORITE_FILTER		14
#define DO_CUSTOM_GROUP_FILTER			15
#define DO_CUSTOM_GROUP_FILTER_WITH_SUB	16
#define DO_GLOBAL_FILTER_PARENT			23
#define DO_HIDE_OFFLINE_SERVERS			50

#define DO_REGION_FOLDER				60
#define DO_REGION_SELECTION				61


#define UNCHECKED_ICON 9
#define CHECKED_ICON 10
#define GRAY_CHECKED_ICON 11

#define MONITOR_AUTOJOIN					1
#define MONITOR_NOTIFY_ACTIVITY				2
#define MONITOR_NOTIFY_FREE_SLOTS			4
#define MONITOR_NOTIFY_MAP_CHANGE			8


//Generic filter defines
#define FILTER_UNKNOWN			0

/*#define FILTER_PB				1  //Punkbuster
#define FILTER_SHOW_PRIVATE		2  //Private
#define FILTER_FULL				4  //Full servers
#define FILTER_EMPTY			8  //Empty
#define FILTER_OFFLINE			16  //Offline
#define FILTER_BOTS				64  //Bots
#define FILTER_HIDE_PRIVATE		128  
#define FILTER_SHOW_TV			129
#define FILTER_HIDE_TV          130
#define FILTER_SHOW_HARDCORE			131  
#define FILTER_HIDE_HARDCORE			132
#define FILTER_SHOW_KILLCAM			133 
#define FILTER_HIDE_KILLCAM			134 
#define FILTER_SHOW_FRIENDLYFIRE			135 
#define FILTER_HIDE_FRIENDLYFIRE			136 
#define FILTER_SHOW_VEHICLE			137  //COd 4 helicopter and Cod 5 = tanks????? not sure if this works or not
#define FILTER_HIDE_VEHICLE			138 
*/

#define FILTER_CUSTOM_MODIFIER	200
#define FILTER_CUSTOM_PARENT    201
#define FILTER_CUSTOM_FILTER    202
#define FILTER_CUSTOM_FILTER_GT  205
#define FILTER_CUSTOM_FILTER_MOD 206

#define FILTER_MOD				24
#define FILTER_GAMETYPE			25
#define FILTER_PURE				26
#define FILTER_RANKED			27
#define FILTER_PING				28
#define FILTER_VERSION			29
#define FILTER_MAP				30
#define FILTER_REGION			31  //Used for steam/valve stuff
#define FILTER_FAVORITERS		32  //Favorites
#define FILTER_DEDICATED		33
#define FILTER_MODE				34  //Used for steam/valve stuff 
#define FILTER_MIN_PLY			100
#define FILTER_MAX_PLY			101

#define FILTER_REGION_US_EAST           1
#define FILTER_REGION_US_WEST			2
#define FILTER_REGION_SOUTH_AMERICA     4
#define FILTER_REGION_EUROPE		8
#define FILTER_REGION_ASIA			16
#define FILTER_REGION_AUSTRALIA		32
#define FILTER_REGION_MIDDLE_EAST   64
#define FILTER_REGION_AFRICA		128
#define FILTER_REGION_ROW			256

#define VERSION_UNKNOWN      0

#define GAMETYPE_UNKNOWN	 0


#define COL_BUDDY_NAME			0
#define COL_BUDDY_SERVERNAME	1
#define COL_BUDDY_IP			3

#define COL_PB			0
#define COL_PRIVATE		1
#define COL_RANKED		2
#define COL_SERVERNAME	3
#define COL_VERSION		4
#define COL_GAMETYPE	5
#define COL_MAP			6
#define COL_MOD			7
#define COL_BOTS		8
#define COL_PLAYERS		9
#define COL_COUNTRY		10
#define COL_PING		11
#define COL_IP			12
#define COL_STATUS		13

//if you add something ensure to increase MAX_COLUMNS below
#define MAX_COLUMNS COL_STATUS+1

struct _CountryFilter 
{
	int counter;

	LPARAM lParam[255];
	char szShortCountryName[255][4];

};

struct GAMEFILTER{
	string sFriendlyName;
	string sStrValue;
	DWORD dwValue;
	DWORD dwExactMatch;
};



struct FILTER_SETTINGS
{
	DWORD dwRegion;
	BOOL bPunkbuster;
	BOOL bHideOfflineServers;
	BOOL bRanked;
	BOOL bPure;
	BOOL bNoBots;
	BOOL bDedicated;

};



struct APP_SETTINGS_NEW
{
   char szEXT_EXE_PATH[MAX_PATH];
   char szEXT_EXE_CMD[MAX_PATH];

   BOOL bXMPP_Active;
   char szXMPP_SERVER[MAX_PATH];
   char szXMPP_USERNAME[MAX_PATH];
   char szXMPP_PASSWORD[MAX_PATH];
   DWORD dwXMPP_PORT;


   char szEXT_EXE_WINDOWNAME[MAX_PATH];
   char szOnReturn_EXE_PATH[MAX_PATH];
   char szOnReturn_EXE_CMD[MAX_PATH];
   BOOL bRegisterWebProtocols;
   BOOL bAutoRefreshServerlist;
   BOOL bCloseOnConnect;
   BOOL bAutostart;
   BOOL bUseBuddySndNotify;
   BOOL bBuddyNotify;
   int bUseShortCountry;
   BOOL bUse_minimize;
   DWORD dwMinimizeMODKey;
   char cMinimizeKey;
   BOOL bEXTMinimize;
   BOOL bEXTClose;
   char szET_WindowName[MAX_PATH];
   int bNoMapResize;
   int bUse_EXT_APP;
   BOOL bUse_EXT_APP2;
   BOOL bUSE_SCREEN_RESTORE;
   FILTER_SETTINGS filter;
   FILTER_SETTINGS filterFavorites;
   FILTER_SETTINGS filterMaster;
   char g_cTransparancy;
   struct timeval socktimeout;  //not yet implemented see below
   BOOL bShowMinimizePopUp;
   BOOL bShowBuddyList;
   BOOL bShowMapPreview;
   BOOL bShowRCON;
   BOOL bShowServerRules;
   BOOL bShowPlayerList;
   BOOL bShowServerList;
   BOOL	bAutoRefreshActive;
   int iAutoRefreshEveryMinuteInterval;
   BOOL bSortBuddyAsc;
   BOOL bSortPlayerAsc;
   BOOL bPlayerNameAsc;
   BOOL bSortPlayerServerNameAsc;
   BOOL bUseColorEncodedFont;
   char cPlayerColumnSort;
   char cBuddyColumnSort;
   char cServerColumnSort;
   char szNotifySoundWAVfile[MAX_PATH];
   BOOL bUseCountryFilter;
   BOOL bUseMIRC;
   DWORD dwRetries;
   DWORD dwThreads;
   DWORD dwSleep;
   int nWindowState;
   char szLanguageFilename[MAX_PATH];
};

struct SERVER_INFO;
typedef SERVER_INFO* LPSERVERINFO;

struct PLAYERDATA;

struct PLAYERDATA
{
	
	char iPlayer;  //Player number or index
	DWORD ping;
	DWORD rate;
	DWORD time;
	DWORD dwServerIndex;
	char *szPlayerName;
	char *szClanTag;
	char *szTeam;
	BYTE ClanTagPos;
	BYTE isBot;
	char cGAMEINDEX;
	SERVER_INFO *pServerInfo;
	PLAYERDATA *pNext;
};

typedef struct PLAYERDATA* LPPLAYERDATA;

typedef vector<LPPLAYERDATA> vecPlyList;  


struct SERVER_RULES
{
	char *name;
	char *value;
	SERVER_RULES *pNext;
};
typedef struct SERVER_RULES* LPSERVER_RULES;



struct SERVER_INFO
{	
	UINT cGAMEINDEX;  //RTCW, ET, Quake 4....
	//BOOL bLocked;  //Simple lock 
	DWORD dwIndex;
	DWORD dwLVIndex; //index of the ListView for realtime listview update.
	char *szServerName;//[MAX_NAME_LEN];
	char szIPaddress[MAX_IP_LEN];
	DWORD dwIP;
	unsigned short usPort;
	unsigned short usQueryPort;
	char szShortCountryName[3];

	UINT nPlayers;
	UINT nMaxPlayers;	
	UINT nPrivateClients;
	short STEAM_AppId;
	
	char *szMap;//these pointer are used for quick look up during sorting
	string szMapPrevious;//these pointer are used for quick look up during sorting
	char *szMod;
	char *szGameTypeName;
	char *szVersion;
	char *szMode;			//[MAX_MODNAME_LEN];  //used for Q3 CPMA
	char *szFS_GAME;
	char *szSTATUS;

	time_t timeLastScan;
				

	DWORD dwTimeStamp; //not used
	DWORD dwPing;	
	int bPrivate;
	int bPunkbuster;
	int cPurge;          //Purge counter
	int cFavorite;  
	int cHistory;  //History.... connected to this server.
	int cRanked;
	int cBots;

	BYTE bUpdated;		

	WORD wMonitor;

	CRITICAL_SECTION	csLock; 
	PLAYERDATA *pPlayerData;
	SERVER_RULES *pServerRules;
	char szRCONPASS[MAX_RCON_LEN];
	char szPRIVATEPASS[MAX_PASSWORD_LEN];
	bool operator == (LPSERVERINFO lpSI) 
	{
		return ((dwIP == lpSI->dwIP) && (usQueryPort == lpSI->usQueryPort));	
	}
};



struct _WINDOW_CONTAINER{
	int idx;
	BOOL bShow;
	HWND hWnd;
	RECT rSize; //CLient size
	RECT rSizeWnd; //Window size
	RECT rMinSize;	
};
typedef _WINDOW_CONTAINER * LPWNDCONT;

typedef vector<LPSERVERINFO> vSRV_INF;

typedef vector<GAMEFILTER> vGF;


//For virtual list
struct REF_SERVER_INFO
{	
	SERVER_INFO *pServerInfo;
};
typedef REF_SERVER_INFO* LPREFSERVER_INF;


struct GAME_INSTALLATIONS
{
	string sName;
	string szGAME_PATH;
	string szGAME_CMD;
	//string sVersion;
	//string sMod;
	BOOL bActiveScript;
	string sFilterName;
	string sScript;
};

struct GAME_SPECIFIC_COLUMNS
{
	int iColumnIdx;
	string sRuleValue;
};

struct FILTER_CODE
{
	WORD LN;		//line number
	BYTE CMD;		//if, exec, launch ,remove...?
	BYTE secCMD;	//secondary command
	BYTE OP;		//primary operator code	
	BOOL bCompareMode; 
	BYTE constant1;
	BYTE constant2;
	string sValue1;
	string sValue2;
	WORD nextLN;
};
typedef vector<FILTER_CODE> vFILTER_CODE;

struct FILTER_SET
{
	string sFilterName;
	string sGroupName;
	int iFilterType;
	BOOL bResult;
	vFILTER_CODE vecFilter_Codes;
};

typedef vector<FILTER_SET> vFILTER_SETS;





/*
Reference server info, this will only point to the full server info container 
using Gametype var and index combined.
*/

typedef struct _ipport
{
	UINT32 uiIP;
	UINT16 uiPort;
}_IPPORT;

_IPPORT p;


typedef pair <int, _IPPORT> IPPORT_Pair;
typedef hash_multimap <int, _IPPORT> IPPortHash;
typedef vector<SERVER_INFO*> veclpSI;

typedef vector<REF_SERVER_INFO> vREF_SRV_INF;
typedef vector<REF_SERVER_INFO> vREF_SRV_INF;
typedef pair <int, int> Int_Pair;
typedef hash_multimap <int, int> serverhash;

// create an empty hash_multimap hmp0 of key type integer
typedef vector<GAME_INSTALLATIONS> vGAME_INSTALLS;
typedef vector<GAME_SPECIFIC_COLUMNS> vGAME_SPECIFIC_COLUMNS;

struct GAME_INFO;



struct GAME_INFO
{
	BOOL bActive;
	char cGAMEINDEX;
	BOOL bLockServerList;
	char szGAME_NAME[MAX_PATH];
	char szGAME_SHORTNAME[MAX_PATH];
	char szMAP_MAPPREVIEW_PATH[MAX_PATH];
	char szMAP_YAWN_PATH[MAX_PATH];
	char szMasterServerIP[10][MAX_PATH];	
	BOOL bUseHTTPServerList[10];
	char szGameProtocol[10][5];
	char szWebProtocolName[20];  //etqw://  et:// etc..
	int nMasterServers;
	char szMasterQueryString[MAX_PATH];
	char szServerRequestInfo[MAX_PATH]; //0xFF 0xFF etc getInfo
	

	DWORD dwViewFlags;
	DWORD dwMasterServerPORT;
	DWORD dwDefaultPort;
	DWORD dwProtocol;
	DWORD dwTotalServers;
	HTREEITEM hTI;
	UINT iIconIndex;
	char szFilename[MAX_PATH];

	DWORD dwScanIdx;
	time_t lastScanTimeStamp;
	int GAME_ENGINE;	

	vSRV_INF vSI;
	vREF_SRV_INF vRefListSI;
	vREF_SRV_INF vRefScanSI;
	serverhash shash;

	vGAME_INSTALLS vGAME_INST_DEFAULT;  //This is used for config
	vGAME_INSTALLS vGAME_INST;
	vGAME_SPECIFIC_COLUMNS vGAME_SPEC_COL;
	vFILTER_SETS vFilterSets;
	vFILTER_SETS vFilterSetsFavorites;
	FILTER_SETTINGS filter;
    char *(*colorfilter)(const char *szIn, char *szOut,unsigned int length);
	LRESULT (*Draw_ColorEncodedText)(RECT rc, LPNMLVCUSTOMDRAW pListDraw , char *pszText);
	DWORD (*GetServersFromMasterServer)(GAME_INFO *pGI, int nMasterIdx);
	DWORD (*GetServerStatus)(SERVER_INFO *pSI,long (__cdecl*)(LPPLAYERDATA pPlayers),long (__cdecl*)(LPSERVER_RULES pServerRules));
};

typedef std::map<int,GAME_INFO>GamesMap;

 

struct _MYTREEITEM
{
	HTREEITEM hTreeItem;

	std::string sName;
	std::string sElementName;
	std::string sScript;
	DWORD dwState;
	DWORD dwType;
	DWORD dwLevel; //XML hierachy
	int iIconIndex;
	int bExpanded;
	bool bVisible;
	bool bDelete;
	DWORD dwIndex;
	GAME_INFO *pGI;
	int cGAMEINDEX;
};


struct _CUSTOM_COLUMN
{
	unsigned short id;
	//std::string sName;
	std::string sColumnName;
	LV_COLUMN   lvColumn;
	int bActive;
	int bSortAsc;
	short columnIdx;
	short columnIdxToSave;
};


inline bool operator == (SERVER_INFO pSIa, SERVER_INFO pSIb)
{
	return ((pSIa.dwIP == pSIb.dwIP) && (pSIa.usQueryPort == pSIb.usQueryPort));
}


struct ET_COLOR_CODES
{
	COLORREF color;
};


#endif
