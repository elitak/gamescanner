
#include "structs_defines.h"
#include <tinyxml.h>

#pragma once
#ifndef __UTILZ__
#define __UTILZ__


//Debug OFF!!!
#ifndef _DEBUG

//#define dbg_print(exp) ((void)0)
#define dbg_print (void)
#define dbg_printNoLF (void)

#else

void dbg_print(TCHAR *szMsg, ...);
void dbg_printNoLF(char *szMsg, ...);

#endif



#define GS_LOG_WARNING 10
#define GS_LOG_INFO 20
#define GS_LOG_DEBUG 30
#define GS_LOG_ERROR 40

void GetServerLock(SERVER_INFO *pSrv);
void ReleaseServerLock(SERVER_INFO *pSrv);

char * ServerRule_Add(SERVER_RULES* &pLinkedListStart,char *szRuleName,char*szRuleValue);
int CustomStrCmp(TCHAR *a, TCHAR *b);
TCHAR *SplitIPandPORT(TCHAR *szIPport,DWORD &port);
BOOL ReplaceStrInStr(string &strToReplace,const char *szReplace,const char *szReplaceWith);
TCHAR *Get_RuleValue(const TCHAR *szRuleName,SERVER_RULES *pSR,int iCompareMode=0);
int UTILZ_ConvertEscapeCodes(TCHAR*pszInput,TCHAR*pszOutput,DWORD dwMaxBuffer);

BOOL UTILZ_checkforduplicates(GAME_INFO *pGI, int hash,DWORD dwIP, DWORD dwPort);
SOCKET getsock(const TCHAR *host, unsigned short port, int family, int socktype, int protocol);

#define getsockudp(host,port) getsock(host, port, AF_INET, SOCK_DGRAM, IPPROTO_UDP)
#define getsocktcp(host,port) getsock(host, port, AF_INET, SOCK_STREAM, IPPROTO_TCP)
DLGTEMPLATE * WINAPI DoLockDlgRes(LPCSTR lpszResName) ;
void SetDlgTrans(HWND hwnd,int trans);


//bool UTILZ_CheckForDuplicateServer(GAME_INFO *pGI, SERVER_INFO pSI);
TCHAR *colorfilter(const TCHAR* name,TCHAR *namefilter, unsigned int len);
char *colorfilterJK3(const char* name,char *namefilter, unsigned int len);
TCHAR *colorfilterQ4(const TCHAR* name,TCHAR *namefilter, unsigned int len);
TCHAR *colorfilterQW(const TCHAR *szInText,TCHAR *namefilter, unsigned int len);
TCHAR *colorfilterNEXUIZ(const TCHAR* name,TCHAR *namefilter, unsigned int len);
TCHAR * colorfilterUTF8(const TCHAR *szInText,TCHAR *namefilter, unsigned int len);
char *UTF8toMB(const char* inUtf8, char* outStr, int maxLen);
BOOL CenterWindow(HWND hwnd);
void RegisterProtocol(TCHAR *path);
void AddAutoRun(TCHAR *path);
void RemoveAutoRun();
void dbg_readbuf(const TCHAR *file, TCHAR *buf, size_t size);
void dbg_dumpbuf(const TCHAR *file, const void *buf, size_t size);
//void AddLogInfo(int color, TCHAR *lpszText, ...);
//void SetLogPath(const TCHAR *szPath);
TCHAR *getpacket(SOCKET s, size_t *len);
long CheckForDuplicateServer(GAME_INFO *pGI, SERVER_INFO *pSI);
DEVMODE GetScreenResolution(VOID);
BOOL	SetScreenResolution(DEVMODE DisplaySettings);
void SetThreadName( DWORD dwThreadID, TCHAR* threadName);
//HFONT SetFont(HWND hDlg, int nIDDlgItem);
HFONT  MyCreateFont(HWND hDlg,LONG height = 14, LONG weight = FW_NORMAL, TCHAR* pszFontFace=_T("Arial") );
void SetFontToDlgItem(HWND hDlg,HFONT hf,int nIDDlgItem);
void Show_ErrorDetails(LPTSTR lpszFunction);
void UTILZ_SetStatusText( int icon, TCHAR *szMsg,...);

void CleanUp_PlayerList(LPPLAYERDATA &pPL);
void CleanUp_ServerRules(LPSERVER_RULES &pSR);
void CleanUp_ServerInfo(SERVER_INFO *pSI);

void SetStatusText(int icon, const TCHAR *szMsg,...);
TCHAR *	ReadPacket(SOCKET socket, size_t *len);
TCHAR * DWORD_IP_to_szIP(DWORD dwIP);
DWORD NetworkNameToIP(TCHAR *host_name,TCHAR *port);
BOOL Set_RuleStr(SERVER_RULES *pServerRules, TCHAR *szRuleName, TCHAR *StrOut, size_t OutbufferSize);
BOOL Set_RuleInt(SERVER_RULES *pServerRules, TCHAR *szRuleName, int *IntOut);

const char * ReadCfgStr2(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize);
int ReadCfgInt2(TiXmlElement* pNode, char *szParamName, int& intVal);
const TCHAR *stristr(TCHAR *szString, const TCHAR *szSubstring);
int wildcmp(const char *string, const char *wild) ;
int wildicmp(const char *string, const char *wild);
#endif