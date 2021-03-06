#define _SECURE_SCL 0

#include "stdafx.h"
#include "q3.h"
#include "utilz.h"
#include "scanner.h"
//#include "..\..\iptocountry\iptocountry.h"
#include "gamemanager.h"


#pragma warning(disable : 4995)
#pragma warning(disable : 4996)


extern char EXE_PATH[_MAX_PATH+_MAX_FNAME];	
extern CGameManager gm;
extern CIPtoCountry g_IPtoCountry;
extern CLanguage g_lang;
extern bool g_bCancel;

//extern GamesMap gm.GamesInfo;
extern APP_SETTINGS_NEW AppCFG;
extern HWND g_hWnd;
extern SERVER_INFO *g_CurrentSRV;
extern BOOL ReplaceStrInStr(string &strToReplace,const char *szReplace,const char *szReplaceWith);

long (*Q3_UpdateServerListView)(DWORD index);
long (*Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo);
long (*Q3_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI);

DWORD Q3_dwThreadCounter=0;

extern CLogger g_log;

char szPlyType[5][12] = {TEXT("Connecting"), TEXT("Axis"), TEXT("Allies"), TEXT("Spectator"),TEXT("Unknown")};


void Q3_SetCallbacks(long (*UpdateServerListView)(DWORD index), 
					 long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),
					 long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q3_UpdateServerListView = UpdateServerListView;
	Callback_CheckForBuddy = _Callback_CheckForBuddy;
	Q3_InsertServerItem = InsertServerItem;
}


unsigned char* Warsow_patch(SOCKET pSocket,SERVER_INFO *pSI, DWORD *dwStartTick,size_t *packetlen)
{

	unsigned char *packet=NULL;
	char szWarsow042[] = {"\xFF\xFF\xFF\xFFgetinfo"};
	*packetlen = send(pSocket, szWarsow042, sizeof(szWarsow042), 0);	
	*dwStartTick = GetTickCount();
	packet=(unsigned char*)getpacket(pSocket, packetlen);
	if(packet!=NULL)
		return packet;

	if(*packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;

	}
	return NULL;
}


DWORD Q3_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules))
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	DWORD dwStartTick=0;

	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return (DWORD)0xdead;
	}
	pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->usPort);  
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return 0x000002;
	}

	size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
	{	
		char szShortName[4];			
		g_IPtoCountry.IPtoCountry(pSI->dwIP,szShortName);//optimized since v1.31 
		strncpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName,_TRUNCATE);
	}
	DWORD dwRetries=0;
	int len = 0; //(int)strlen(sendbuf);
	char sendbuf[80];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	len = UTILZ_ConvertEscapeCodes(gm.GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo,sendbuf,sizeof(sendbuf));
retry:


	if(gm.GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo!=NULL)
		packetlen = send(pSocket, sendbuf, len+1, 0);
	else
		packetlen=SOCKET_ERROR;

		
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
		return -1;
	}

	dwStartTick = GetTickCount();
	packet=(unsigned char*)getpacket(pSocket, &packetlen);
	if(pSI->cGAMEINDEX==WARSOW_SERVERLIST && packet==NULL)
		packet = Warsow_patch(pSocket,pSI,&dwStartTick,&packetlen);

	if(packet==NULL)
	{
		
		if(dwRetries<AppCFG.dwRetries)
		{
			

			dwRetries++;
			goto retry;
		}
	}

	if(packet) 
	{
		pSI->dwPing = (GetTickCount() - dwStartTick);

		GetServerLock(pSI);

		CleanUp_ServerInfo(pSI);


		//dbg_dumpbuf("dump.bin", packet, packetlen);
		SERVER_RULES *pServRules=NULL;
		char *end = (char*)((packet)+packetlen);
		
		char *pCurrPointer=NULL; //will contain the start address for the player data

		pCurrPointer = Q3_ParseServerRules(pServRules,(char*)packet,packetlen);
		pSI->pServerRules = pServRules;
		if(pServRules!=NULL)
		{		
		
			char szP_ET[150];
			if(pSI->cGAMEINDEX == ET_SERVERLIST)
			{			
				ZeroMemory(&szP_ET,sizeof(szP_ET));			
				char *szPVarValue=NULL;
				szPVarValue = Get_RuleValue("P",pServRules);
				if(szPVarValue!=NULL)
					strcpy(szP_ET,szPVarValue);
			}
			char *szVarValue=NULL;
			GAME_INFO *pGI = &gm.GamesInfo[pSI->cGAMEINDEX];
			pSI->szServerName = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_SERVERNAME).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szMap = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_MAP).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szMod = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_MOD).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szGameTypeName = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_GAMETYPE).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szVersion = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_VERSION).sRuleValue.c_str(),pSI->pServerRules);

			PLAYERDATA *pQ3Players=NULL;
			DWORD nPlayers=0;
			//---------------------------------
			//Retrieve players if any exsist...
			//---------------------------------
			switch(pSI->cGAMEINDEX)
			{
				case WARSOW_SERVERLIST:
					{
						const char *szTVvalue = Get_RuleValue((TCHAR*)"tv",pSI->pServerRules);
						if(szTVvalue!=NULL)
						{
							if(strcmp("1",szTVvalue)==0)
								pSI->szServerName = Get_RuleValue((TCHAR*)"tv_name",pSI->pServerRules);
							
							const char* szClients = Get_RuleValue((TCHAR*)"clients",pSI->pServerRules);
							if(szClients!=NULL)
								nPlayers =  atoi(szClients);

						} else
						{
							pQ3Players = Q3_ParsePlayers2(pSI,pCurrPointer,end,&nPlayers);
						}

					}
					break;
				case QW_SERVERLIST:
				case Q2_SERVERLIST:
					pQ3Players = QW_ParsePlayers(pSI,pCurrPointer,end,&nPlayers);
					break;
				default:
					pQ3Players = Q3_ParsePlayers2(pSI,pCurrPointer,end,&nPlayers,szP_ET);
				break;
			}
			
			pSI->pPlayerData = pQ3Players;

			//-----------------------------------
			//Update server info from rule values
			//-----------------------------------
			pSI->bUpdated = 1;
			pSI->cPurge = 0;
			pSI->nPlayers = nPlayers;
			


			szVarValue= Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_PRIVATE).sRuleValue.c_str(),pSI->pServerRules);
			if(szVarValue!=NULL)
				pSI->bPrivate = (char)atoi(szVarValue);

			szVarValue = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_PB).sRuleValue.c_str(),pServRules);		
			if(szVarValue!=NULL)
				pSI->bPunkbuster = (char)atoi(szVarValue);


			//getting status value
			switch(pSI->cGAMEINDEX)
			{	
				case QW_SERVERLIST:
						pSI->szSTATUS = Get_RuleValue("status",pSI->pServerRules);
				break;
				case Q2_SERVERLIST:
					{
						//Lets enumerate through vars for the best match to sue as a status...
						pSI->szSTATUS = Get_RuleValue("time_remaining",pSI->pServerRules);
						if(pSI->szSTATUS==NULL)						
						{
							pSI->szSTATUS = Get_RuleValue("#time_left",pSI->pServerRules);
							if(pSI->szSTATUS==NULL)
							{
								pSI->szSTATUS = Get_RuleValue("gamestats",pSI->pServerRules);
							}
						}
					}
				break;
				case Q3_SERVERLIST:
						pSI->szSTATUS = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_STATUS).sRuleValue.c_str(),pSI->pServerRules);
					break;
			}

			szVarValue = Get_RuleValue("sv_privateClients",pServRules);
			if(szVarValue!=NULL)
				pSI->nPrivateClients = atoi(szVarValue);

			szVarValue = Get_RuleValue("sv_maxclients",pServRules);
			if(szVarValue!=NULL)
			{
				unsigned int maxClient = atoi(szVarValue);
				if(maxClient>pSI->nPrivateClients)
					pSI->nMaxPlayers = maxClient-pSI->nPrivateClients;
				else
					pSI->nMaxPlayers = pSI->nPrivateClients-maxClient;

			}
			else
			{ //for QW
				szVarValue = Get_RuleValue("maxclients",pServRules);
				if(szVarValue!=NULL)
					pSI->nMaxPlayers = atoi(szVarValue)-pSI->nPrivateClients;
			}

			time(&pSI->timeLastScan);

	

		} //end if(pServRules!=NULL)
	
		ReleaseServerLock(pSI);
		if(Callback_CheckForBuddy!=NULL)
			Callback_CheckForBuddy(pSI->pPlayerData,pSI);

		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;   //increase purge counter when the server is not responding

	if(UpdatePlayerListView!=NULL) 
		UpdatePlayerListView(pSI->pPlayerData);
		
	if(UpdateRulesListView!=NULL)
		UpdateRulesListView(pSI->pServerRules);

	closesocket(pSocket);
//	pSI->bLocked = FALSE;
	return 0;
}


DWORD EA_ConnectToMasterServer(GAME_INFO *pGI, int iMasterIdx)
{


	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

   
    // Start the child process. 
 /*   if( !CreateProcess( NULL,   // No module name (use command line)
         "ealist -o 1 -n bfbc2-pc -a kallekula2010 Ku**fu***l mohair-pc",        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
       MessageBox(NULL,"ealist not found","Error!",MB_OK);
        return 1;
    }

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
*/
	Parse_FileServerList(pGI,"bfbc2-pc.gsl",EXE_PATH);
	return 0;
}


DWORD BFBC2_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules))
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	DWORD dwStartTick=0;

	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return (DWORD)0xdead;
	}
	pSI->usQueryPort = 48888;

	pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->usQueryPort);  
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return 0x000002;
	}

	size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
	{	
		char szShortName[4];			
		g_IPtoCountry.IPtoCountry(pSI->dwIP,szShortName);//optimized since v1.31 
		strncpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName,_TRUNCATE);
	}
	DWORD dwRetries=0;
	int len = 0; //(int)strlen(sendbuf);
	char sendbuf[80];
	ZeroMemory(sendbuf,sizeof(sendbuf));

	//memcpy( buf, "\x00\x00\x00\x00\x1b\x00\x00\x00\x01\x00\x00\x00\x0a\x00\x00\x00serverInfo\x00", size );
	//memcpy( buf, "\x01\x00\x00\x00\x15\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00quit\x00", size );

	len = UTILZ_ConvertEscapeCodes(gm.GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo,sendbuf,sizeof(sendbuf));
retry:


	//if(gm.GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo!=NULL)
		packetlen = send(pSocket, "\x00\x00\x00\x00\x1b\x00\x00\x00\x01\x00\x00\x00\x0a\x00\x00\x00serverInfo\x00", 27, 0);
//	else
//		packetlen=SOCKET_ERROR;

		
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
		return -1;
	}

	dwStartTick = GetTickCount();
	packet=(unsigned char*)getpacket(pSocket, &packetlen);

	if(packet==NULL)
	{
		if(dwRetries<AppCFG.dwRetries)
		{
			dwRetries++;
			goto retry;
		}
	}

	if(packet) 
	{
		pSI->dwPing = (GetTickCount() - dwStartTick);

		GetServerLock(pSI);
		if(pSI->pPlayerData!=NULL)
			CleanUp_PlayerList(pSI->pPlayerData);
		pSI->pPlayerData = NULL;

		if(pSI->pServerRules!=NULL)
			CleanUp_ServerRules(pSI->pServerRules);
		pSI->pServerRules = NULL;


		//dbg_dumpbuf("dump.bin", packet, packetlen);
		SERVER_RULES *pServRules=NULL;
		char *end = (char*)((packet)+packetlen);
		
		char *pCurrPointer=NULL; //will contain the start address for the player data

		//pCurrPointer = Q3_ParseServerRules(pServRules,(char*)packet,packetlen);
		pSI->pServerRules = pServRules;
		if(pServRules!=NULL)
		{		

			char *szVarValue=NULL;
			GAME_INFO *pGI = &gm.GamesInfo[pSI->cGAMEINDEX];
//			pSI->szServerName = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_SERVERNAME).sRuleValue.c_str(),pSI->pServerRules);
//			pSI->szMap = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_MAP).sRuleValue.c_str(),pSI->pServerRules);
//			pSI->szMod = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_MOD).sRuleValue.c_str(),pSI->pServerRules);
//			pSI->szGameTypeName = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_GAMETYPE).sRuleValue.c_str(),pSI->pServerRules);
//			pSI->szVersion = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_VERSION).sRuleValue.c_str(),pSI->pServerRules);

			PLAYERDATA *pQ3Players=NULL;
			DWORD nPlayers=0;
			//---------------------------------
			//Retrieve players if any exsist...
			//---------------------------------
//			pQ3Players = Q3_ParsePlayers2(pSI,pCurrPointer,end,&nPlayers,szP_ET);
			
			pSI->pPlayerData = pQ3Players;

			//-----------------------------------
			//Update server info from rule values
			//-----------------------------------
			pSI->bUpdated = 1;
			pSI->cPurge = 0;
			pSI->nPlayers = nPlayers;
			


			szVarValue= Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_PRIVATE).sRuleValue.c_str(),pSI->pServerRules);
			if(szVarValue!=NULL)
				pSI->bPrivate = (char)atoi(szVarValue);

			szVarValue = Get_RuleValue((TCHAR*)pGI->vGAME_SPEC_COL.at(COL_PB).sRuleValue.c_str(),pServRules);		
			if(szVarValue!=NULL)
				pSI->bPunkbuster = (char)atoi(szVarValue);

			szVarValue = Get_RuleValue("sv_privateClients",pServRules);
			if(szVarValue!=NULL)
				pSI->nPrivateClients = atoi(szVarValue);

			time(&pSI->timeLastScan);

	

		} //end if(pServRules!=NULL)
	
		ReleaseServerLock(pSI);
		if(Callback_CheckForBuddy!=NULL)
			Callback_CheckForBuddy(pSI->pPlayerData,pSI);

		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;   //increase purge counter when the server is not responding

	if(UpdatePlayerListView!=NULL) 
		UpdatePlayerListView(pSI->pPlayerData);
		
	if(UpdateRulesListView!=NULL)
		UpdateRulesListView(pSI->pServerRules);

	closesocket(pSocket);
//	pSI->bLocked = FALSE;
	return 0;
}



DWORD COD4_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules))
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	DWORD dwStartTick=0;

	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return (DWORD)0x000001;
	}

	pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->usPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return 0x000002;
	}

	size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
	{
	
		char szShortName[4];			
		g_IPtoCountry.IPtoCountry(pSI->dwIP,szShortName);//optimized since v1.31 
		strncpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName,_TRUNCATE);
	}
	DWORD dwRetries=0;
	int len = 0; //(int)strlen(sendbuf);
	char sendbuf[80];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	len = UTILZ_ConvertEscapeCodes(gm.GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo,sendbuf,sizeof(sendbuf));
retry:
	if(gm.GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo!=NULL)
		packetlen = send(pSocket, sendbuf, len+1, 0);
	else
		packetlen=SOCKET_ERROR;

		
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
	//	pSI->bLocked = FALSE;
		return -1;
	}

	dwStartTick = GetTickCount(); //ping data start 
	packet=(unsigned char*)getpacket(pSocket, &packetlen);
	if(packet==NULL)
	{
		if(dwRetries<AppCFG.dwRetries)
		{
			dwRetries++;
			goto retry;
		}
	}

	if(packet) 
	{
		pSI->dwPing = (GetTickCount() - dwStartTick); //ping data stop
		//dbg_dumpbuf("dump.bin", packet, packetlen);

		GetServerLock(pSI);

		CleanUp_ServerInfo(pSI);

		SERVER_RULES *pServRules=NULL;
		char *end = (char*)((packet)+packetlen);
		
		char *pCurrPointer=NULL; //will contain the start address for the player data

		pCurrPointer = Q3_ParseServerRules(pServRules,(char*)packet,packetlen);
		pSI->pServerRules = pServRules;

		if(pServRules!=NULL)
		{		
		
			PLAYERDATA *pQ3Players=NULL;
			DWORD nPlayers=0;
			//---------------------------------
			//Retrieve players if any exsist...
			//---------------------------------

			pSI->szServerName = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_SERVERNAME).sRuleValue.c_str(),pSI->pServerRules,1);
			pSI->szMap = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_MAP).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szMod = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_MOD).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szGameTypeName = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_GAMETYPE).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szVersion = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_VERSION).sRuleValue.c_str(),pSI->pServerRules);

			pQ3Players = Q3_ParsePlayers2(pSI,pCurrPointer,end,&nPlayers);

	
			pSI->pPlayerData = pQ3Players;

			//-----------------------------------
			//Update server info from rule values
			//-----------------------------------

			pSI->bUpdated = 1;
			pSI->cPurge = 0;
			pSI->nPlayers = nPlayers;
			char *szVarValue=NULL;
			char *pVarValue = NULL;
	
			szVarValue = Get_RuleValue("pswrd",pServRules);  //CoD & Cod2
			if(szVarValue!=NULL)
				pSI->bPrivate = atoi(szVarValue);


			szVarValue = Get_RuleValue("sv_punkbuster",pServRules);
			if(szVarValue!=NULL)
				pSI->bPunkbuster = (char)atoi(szVarValue);


			szVarValue = Get_RuleValue("sv_privateClients",pServRules);
			if(szVarValue!=NULL)
				pSI->nPrivateClients = atoi(szVarValue);

			szVarValue = Get_RuleValue("sv_maxclients",pServRules);
			if(szVarValue!=NULL)
			{
				unsigned int maxClient = atoi(szVarValue);
				if(maxClient>pSI->nPrivateClients)
					pSI->nMaxPlayers = maxClient-pSI->nPrivateClients;
				else
					pSI->nMaxPlayers = pSI->nPrivateClients-maxClient;

			}
			free(packet);
			packet =NULL;
		//	pCurrPointer = Q3_ParseServerRules(pServRules,(char*)packet,packetlen);

			ZeroMemory(sendbuf,sizeof(sendbuf));
			len = UTILZ_ConvertEscapeCodes("\xFF\xFF\xFF\xFFgetinfo x73",sendbuf,sizeof(sendbuf));
		//	Sleep(50);		
			packetlen = send(pSocket, sendbuf, len+1, 0);
			packet=(unsigned char*)getpacket(pSocket, &packetlen);

			if(packet) 
			{
				//dbg_dumpbuf("dump.bin", packet, packetlen);
				SERVER_RULES *pServRulestemp=NULL;
				SERVER_RULES *pServRules2=NULL;
				char *end = (char*)((packet)+packetlen);
				
				char *pCurrPointer2=NULL; //will contain the start address for the player data

				pCurrPointer2 = Q3_ParseServerRules(pServRules2,(char*)packet,packetlen);
				pServRulestemp = pSI->pServerRules ;
				while(pServRulestemp!=NULL)
				{
					if(pServRulestemp->pNext==NULL)
						break;
					pServRulestemp = pServRulestemp->pNext;

				}
				
				pServRulestemp->pNext = pServRules2;
	
				free(packet);
				packet = NULL;
			}
			time(&pSI->timeLastScan);

		} //end if(pServRules!=NULL)

		ReleaseServerLock(pSI);
		
		if(Callback_CheckForBuddy!=NULL)  //This has to be done after the lock to avoid deadlock
			Callback_CheckForBuddy(pSI->pPlayerData,pSI);	

	} //end if(packet)
	else
		pSI->cPurge++;   //increase purge counter when the server is not responding



	if(UpdatePlayerListView!=NULL) 
		UpdatePlayerListView(pSI->pPlayerData);
		
	if(UpdateRulesListView!=NULL)
		UpdateRulesListView(pSI->pServerRules);

	
	closesocket(pSocket);
//	pSI->bLocked = FALSE;
	return 0;
}



DWORD Q3_ParseServers(char * p, DWORD length, GAME_INFO *pGI)
{
	DWORD dwNewTotalServers = 0;
	Q3DATA *Q3d;
	Q3d = (Q3DATA*)p;

	SERVER_INFO ptempSI;
	DWORD idx = pGI->vSI.size();	
	DWORD *dwIP=NULL;
	if (p==NULL)
		return 0;

	char *end;
	end = p+length-10;
/*
Quake 2 master server response
0x011F0BBB  01 00 00 00 00 00 00 00 00 00 00 00 00 40 27 00 00 01 00 00 00 39 e7 00 00 fd fd fd fd ff ff ff ff 73 65 72 76 65 72 73 20  .............@'......9�..��������servers 
0x011F0BE4  45 09 a8 04 6d 19 d0 2b 0f c7 6d 06 26 67 08 62 6d 0c 26 67 08 62 6d 0b c1 6e 7a d7 6d 06 48 e8 e4 ba 6d 1c d9 aa 42 53 6d  E.�.m.�+.�m.&g.bm.&g.bm.�nz�m.H��m.٪BSm
            1  2   3  4  5 6  1  2                                                                                                      23456123456123456

0x01ECB9B8  ff ff ff ff 73 65 72 76 65 72 73 20 45 09 a8 04 6d 19 d0 2b 0f c7 6d 06 26 67 08 62 6d 0c 26 67 08 62 6d 0b c1 6e 7a d7 6d  ����servers E.�.m.�+.�m.&g.bm.&g.bm.�nz�m
0x01ECB9E1  06 48 e8 e4 ba 6d 1c d9 aa 42 53 6d 06 d9 aa 42 53 6d 10 c3 7a d9 13 6d 24 cb ce 5f 01 6d 01 cb ce 5f 01 6d 0c cb ce 5f 01  .H��m.٪BSm.٪BSm.�z�.m$��_.m.��_.m.��_.


Quake 3
0x01EF1F80  ff ff ff ff 67 65 74 73 65 72 76 65 72 73 52 65 73 70 6f 6e 73 65 5c d8 b4 ed 0a a1 9b 5c d9 4f b6 f7 82 19 5c d8 b4 ed 0e  ����getserversResponse\ش�.�.\�O��..\ش�.
0x01EF1FA9  d7 90 5c d8 b4 ed 0e 6d 42 5c 4e 3b 70 80 6d 38 5c d8 b4 ed 0d f4 b4 5c d8 b4 ed 0e 6d 38 5c 57 f9 b7 f2 f4 f3 5c 59 95 c2  �.\ش�.mB\N;p�m8\ش�.��\ش�.m8\W�����\Y.�

CoD 2                                                                                                     \
0x01249028  ff ff ff ff 67 65 74 73 65 72 76 65 72 73 52 65 73 70 6f 6e 73 65 0a 00 5c 4e 8f 19 5d d9 03 5c d5 ef d0 16 71 21 5c 48 33  ����getserversResponse..\N..]�.\���.q!\H3
0x01249051  3c 15 71 20 5c 43 e4 0b 45 71 20 5c 51 13 db d1 71 34 5c 54 c8 fc e9 69 91 5c 51 00 d9 b1 6a 3e 5c d8 06 e1 69 71 20 5c 51  <.q \C�.Eq \Q.��q4\T���i�\Q.ٱj>\�.�iq \Q

CoD 4                                                                                                    \
0x01249028  ff ff ff ff 67 65 74 73 65 72 76 65 72 73 52 65 73 70 6f 6e 73 65 0a 00 5c 41 63 f6 4f 71 20 5c 5b 79 88 c3 71 25 5c c1 11  ����getserversResponse..\Ac�Oq \[y��q%\�.
0x01249051  db 28 71 20 5c 55 be 0b 13 71 20 5c 52 62 e1 b6 71 20 5c 57 76 44 9a 71 20 5c d1 61 55 71 71 20 5c 43 a7 ad f2 d7 20 5c d8  �(q \U...q \Rb�q \WvD�q \�aUqq \C���� \�


*/
	int pbytes=0;
	//Scan to start
	while((p[0]!=0x5c) && (p[0]!=0x20))
	{
		pbytes++;
		p++;
		if(p>end)
			break;
	}
	end = p+(length-pbytes);
	p++;
	
	ZeroMemory(&ptempSI,sizeof(SERVER_INFO));
	ptempSI.dwPing = 9999;
	ptempSI.cGAMEINDEX = (char) pGI->cGAMEINDEX;
	strcpy(ptempSI.szShortCountryName,"zz");
	ptempSI.bUpdated = 0;	

	int hash = 0;
	while(p<end) 
	{	
		if((p[0]=='E') && (p[1]=='O') && (p[2]=='T') && (p[3]==0x00))
			break;
		else if((p[0]=='E') && (p[1]=='O') && (p[2]=='F') && (p[3]==0x00))
			break;

		//Parse and initialize server info
		dwIP = (DWORD*)&p[0];
		ptempSI.dwIP = ntohl((DWORD)*dwIP); 
	
		p+=4;
		ptempSI.usPort  = ((p[0])<<8);
		ptempSI.usPort |=(unsigned char)(p[1]);
		//ptempSI.usPort &= 0x0000FFFF;	//safe, ensure max port value
		
		if(pGI->cGAMEINDEX == Q2_SERVERLIST)
			p+=2; //q2
		else
			p+=3; //q3
		
		ptempSI.usQueryPort = ptempSI.usPort;
		hash = ptempSI.dwIP + ptempSI.usPort;

		if(UTILZ_checkforduplicates(pGI,  hash,ptempSI.dwIP, ptempSI.usPort)==FALSE)
		{	
			strcpy_s(ptempSI.szIPaddress,sizeof(ptempSI.szIPaddress),DWORD_IP_to_szIP(ptempSI.dwIP));
			ptempSI.dwIndex = idx++;

			SERVER_INFO *pNewSrv = (SERVER_INFO*)calloc(1,sizeof(SERVER_INFO));
			memcpy(pNewSrv,&ptempSI,sizeof(SERVER_INFO));
		
			InitializeCriticalSection(&pNewSrv->csLock);

			pGI->shash.insert(Int_Pair(hash,ptempSI.dwIndex) );
			pGI->vSI.push_back(pNewSrv);
			dwNewTotalServers++;
		}
		//end serverexsist

	} //end while
	return dwNewTotalServers;
}


PLAYERDATA *QW_ParsePlayers(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers)
{
	PLAYERDATA *pPlayers=NULL;

	if(pointer[0]!=0)
	{
		//Parseplayers
	
		PLAYERDATA *pCurrentPlayer=NULL;
		
		while(pointer<end)
		{
			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			if(player==NULL) //Out of memory ?
				return pPlayers;
			player->pNext = NULL;
			player->pServerInfo = pSI;
			player->cGAMEINDEX = pSI->cGAMEINDEX;
				
			char *endOfString = strchr(pointer,' ');
			if(endOfString!=NULL)
			{
				endOfString[0] = 0;
				player->iPlayer =  atoi(pointer);
				pointer+=strlen(pointer)+1;

				endOfString = strchr(pointer,' ');
				if(endOfString!=NULL)
				{

					endOfString[0] = 0;
					if(pointer[0]!='S')
					{
						player->rate = atoi(pointer);				
					} else
					{
						player->szTeam = strdup("Spectator");
					}
					pointer+=strlen(pointer)+1;
				}
				endOfString = strchr(pointer,' ');
				if(endOfString!=NULL)
				{
					endOfString[0] = 0;
					player->time = atoi(pointer);
					//skip time				
					pointer+=strlen(pointer)+1;
				}
				endOfString = strchr(pointer,' ');
				if(endOfString!=NULL)                       
				{
					endOfString[0] = 0;
					player->ping = atoi(pointer);
					pointer+=strlen(pointer)+1;
					
				}				
					char *name= NULL;
					 name = strchr(pointer,'\"');				
					if(name!=NULL)
						pointer = ++name; //skip initial byte


				endOfString = strchr(pointer,'\"');
				if(endOfString!=NULL)
				{
					endOfString[0] = 0;
					player->szPlayerName= _strdup(pointer);  //got the name
				
					pointer+=strlen(pointer)+1;
				}

				//jump to end
				endOfString = strchr(pointer,0x0a);
				if(endOfString!=NULL)
				{
					endOfString[0] = 0;
					pointer+=strlen(pointer)+1;
				}							
			}
			player->dwServerIndex = pSI->dwIndex;

			if(pPlayers==NULL)
				pPlayers = pCurrentPlayer = player;
			else 
				pCurrentPlayer = pCurrentPlayer->pNext = player;

			*numPlayers= *numPlayers+1;	
			if(pointer[0]==0) 
				break;
		}
	}
	return pPlayers;
}
/* CoD 4
34 30 20 32 34 35 20 22 63 68 65 6e 22 0a                                             40 245 "chen".
31 36 38 20 31 31 39 20 22 4d 69 4e 64 46 72 45 61 4b 22 0a                          168 119 "MiNdFrEaK".
34 31 20 33 39 20 22 48 61 6e 64 53 6f 6d 65 22 0a                                    41 39 "HandSome".
30 20 39 39 39 20 22 4b 75 6e 67 46 75 44 75 63 6b 69 65 22 0a                         0 999 "KungFuDuckie".
32 20 35 37 20 22 48 45 52 42 45 52 54 22 0a 00 fd fd fd fd ab ab ab ab ab ab ab ab    2 57 "HERBERT"..������������	

*/
int ParseNum(char * &pData)
{
	char * p = pData;
	if(p==NULL)
		return 0;
	//char *endOfString = strchr(pData,' ');
	while(p[0]!=' ')
	{
		if(p[0]=='\"')
			return 0;
		p++;
	}
	if(p!=NULL)
	{
		p[0] = 0;
		int val = atoi(pData);		
		pData+=strlen(pData)+1;
		return val;
	}
	return 0;
}
//Don't forget to free up the new allocated string
char * ParseString(char* & pData)
{
	char *StartOfString= NULL;
	StartOfString = strchr(pData,'\"');				
	if(StartOfString!=NULL)
	{
		pData++; //skip initial byte which is "
		
		char *EndOfString = pData; //strrchr(pData,'\"');
		while(EndOfString[0]!='\"')
			EndOfString++;

		EndOfString[0]=0; //Add a termination NULL 			
		char *NewString= _strdup(pData);
		pData+=strlen(pData)+1;			
		return NewString;
	}
	return NULL;
}
/*
Player structures:

Warsow <v0.4
[rate] [ping] "playername" [team] [0x22]

Warsow v0.5
[rate] [ping] "playername" [team][0x0a]
last player: 
[rate] [ping] "playername" [team][0x0a][0x00]

*/
PLAYERDATA *Q3_ParsePlayers2(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers,char *szP)
{
	int Pindex =0;
	PLAYERDATA *pQ3Players=NULL;
	BOOL bGTVBug=FALSE;
	pSI->cBots = 0;
	if(pointer[0]!=0)
	{
		//Parseplayers	
		PLAYERDATA *pQ3CurrentPlayer=NULL;		
		while(pointer<end)
		{
			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			if(player==NULL) //Out of memory
				return pQ3Players;
			player->pNext = NULL;							
			player->szClanTag = NULL;	
			player->szTeam = NULL;	
			player->cGAMEINDEX = pSI->cGAMEINDEX;
			player->pServerInfo = pSI;
			player->dwServerIndex = pSI->dwIndex;

		
			player->rate = ParseNum(pointer);
			player->ping = ParseNum(pointer);
			int nexuizUnknown = ParseNum(pointer);
			player->szPlayerName = ParseString(pointer);

			
			switch(pSI->cGAMEINDEX)
			{				
				case NEXUIZ_SERVERLIST:
				case OPENARENA_SERVERLIST:
				case Q3_SERVERLIST:
					{
						if(player->ping == 0)
							pSI->cBots ++;

					}
					break;
				case ET_SERVERLIST: //ETpro for retrieving player status (connecting, spectating, allies & axis)
				{
					if(player->ping == 0)
						pSI->cBots ++;

					if(szP!=NULL)
					{
						int l = strlen(szP);
						while(Pindex<l)
						{
							if(szP[Pindex]!='-')
							{
								int _idx = (szP[Pindex]-48);
								player->szTeam = _strdup(&szPlyType[_idx][0]);
								Pindex++;					
								break;
							}
							Pindex++;						
						}
					}
					break;
				}
				case WARSOW_SERVERLIST:
				{
					char *szProtocol=NULL;
				//	GAME_INFO *pGI = &gm.GamesInfo[pSI->cGAMEINDEX];
				//	szProtocol = Get_RuleValue((TCHAR*)_T("protocol"),pSI->pServerRules);
					//if(szProtocol!=NULL)
					{
						//if(atoi(szProtocol)<11) //pre version 0.5
						{
							pointer++;
							char * p0a = strchr(pointer,0x0A);
							if(p0a!=NULL)
								p0a[0]=0;
							if(strcmp(pointer,"0")==0)
								player->szTeam = _strdup("Spectator");
							else if(strcmp(pointer,"2")==0)
								player->szTeam = _strdup("Red");
							else if(strcmp(pointer,"3")==0)
								player->szTeam= _strdup("Blue");
								
							pointer++;						
						}
					}
					break;
				}
			}
			//Player seperator
			pointer++; //normal 0x0a but for Warsow it is 0x22 (pre v0.5)

			if(pQ3Players==NULL)
				pQ3Players = pQ3CurrentPlayer = player;
			else 
				pQ3CurrentPlayer = pQ3CurrentPlayer->pNext = player;

			*numPlayers= *numPlayers+1;	
			if(pointer[0]==0) //Warsow fix
				break;
					
		}
		
	}
	return pQ3Players;
}

/*
QW
0x01049D28  ff ff ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 0a 5c 67 61 6d 65 6e 61 6d  ����infoResponse.\gamenam
0x01049D41  65 00 44 61 72 6b 70 6c 61 63 65 73 2d 51 75 61 6b 65 00 70 72 6f 74 6f 63  e.Darkplaces-Quake.protoc
0x01049D5A  6f 6c 00 33 00 63 6c 69 65 6e 74 73 00 30 00 73 76 5f 6d 61 78 63 6c 69 65  ol.3.clients.0.sv_maxclie
0x01049D73  6e 74 73 00 32 00 6d 61 70 6e 61 6d 65 00 64 6d 34 00 6d 61 78 73 70 65 63  nts.2.mapname.dm4.maxspec
0x01049D8C  74 61 74 6f 72 73 00 38 00 61 6c 6c 6f 77 5f 6c 75 6d 61 00 31 00 61 6c 6c  tators.8.allow_luma.1.all
0x01049DA5  6f 77 5f 62 75 6d 70 00 31 00 66 62 73 6b 69 6e 73 00 31 00 70 6d 5f 62 75  ow_bump.1.fbskins.1.pm_bu
0x01049DBE  6e 6e 79 73 70 65 65 64 63 61 70 00 30 00 70 6d 5f 73 6c 69 64 65 66 69 78  nnyspeedcap.0.pm_slidefix
0x01049DD7  00 30 00 70 6d 5f 73 6c 69 64 79 73 6c 6f 70 65 73 00 30 00 70 6d 5f 61 69  .0.pm_slidyslopes.0.pm_ai
0x01049DF0  72 73 74 65 70 00 30 00 70 6d 5f 77 61 6c 6c 6a 75 6d 70 00 30 00 70 6d 5f  rstep.0.pm_walljump.0.pm_
0x01049E09  73 74 65 70 68 65 69 67 68 74 00 31 38 00 2a 76 65 72 73 69 6f 6e 00 46 54  stepheight.18.*version.FT
0x01049E22  45 20 33 31 33 34 00 2a 7a 5f 65 78 74 00 36 33 00 70 6d 5f 6b 74 6a 75 6d  E 3134.*z_ext.63.pm_ktjum
0x01049E3B  70 00 31 00 2a 67 61 6d 65 64 69 72 00 71 77 00 6d 61 78 66 70 73 00 37 37  p.1.*gamedir.qw.maxfps.77
0x01049E54  00 68 6f 73 74 6e 61 6d 65 00 41 73 67 61 61 72 64 20 31 6f 6e 31 2f 32 6f  .hostname.Asgaard 1on1/2o
0x01049E6D  6e 32 2f 34 6f 6e 34 20 2d 20 4e 75 65 72 6e 62 65 72 67 2c 20 47 65 72 6d  n2/4on4 - Nuernberg, Germ
0x01049E86  61 6e 79 00 64 65 61 74 68 6d 61 74 63 68 00 33 00 74 65 61 6d 70 6c 61 79  any.deathmatch.3.teamplay
0x01049E9F  00 30 00 74 69 6d 65 6c 69 6d 69 74 00 31 30 00 fd fd fd fd ab ab ab ab ab  .0.timelimit.10.���������
0x01049EB8  ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 00 7c 20 9d 3d 88 79 0a 00 18  ���.............| .=�y...



Warsow 0.5               s t  a  t  u  s  R  e  s  p  o  n  s  e     \                             \  \  
0x02FB4E58  ff ff ff ff 73 74 61 74 75 73 52 65 73 70 6f 6e 73 65 0a 5c 63 68 61 6c 6c 65 6e 67 65 5c 5c 76 65 72 73 69 6f 6e 5c 30 2e  ����statusResponse.\challenge\\version\0.
0x02FB4E81  35 30 20 69 33 38 36 20 41 75 67 20 32 35 20 32 30 30 39 20 4c 69 6e 75 78 5c 66 73 5f 67 61 6d 65 5c 62 61 73 65 77 73 77  50 i386 Aug 25 2009 Linux\fs_game\basewsw
0x02FB4EAA  5c 67 5f 61 6e 74 69 6c 61 67 5c 31 5c 67 5f 67 61 6d 65 74 79 70 65 73 5f 61 76 61 69 6c 61 62 6c 65 5c 5c 67 5f 69 6e 73  \g_antilag\1\g_gametypes_available\\g_ins
0x02FB4ED3  74 61 67 69 62 5c 30 5c 67 5f 6d 61 74 63 68 5f 73 63 6f 72 65 5c 5c 67 5f 6d 61 74 63 68 5f 74 69 6d 65 5c 57 61 72 6d 75  tagib\0\g_match_score\\g_match_time\Warmu
0x02FB4EFC  70 5c 67 5f 6e 65 65 64 70 61 73 73 5c 30 5c 67 61 6d 65 64 61 74 65 5c 41 75 67 20 32 35 20 32 30 30 39 5c 67 61 6d 65 6e  p\g_needpass\0\gamedate\Aug 25 2009\gamen
0x02FB4F25  61 6d 65 5c 57 61 72 73 6f 77 5c 6d 61 70 6e 61 6d 65 5c 77 64 6d 31 5c 70 72 6f 74 6f 63 6f 6c 5c 31 31 5c 73 76 5f 63 68  ame\Warsow\mapname\wdm1\protocol\11\sv_ch
0x02FB4F4E  65 61 74 73 5c 30 5c 73 76 5f 68 6f 73 74 6e 61 6d 65 5c 57 61 72 73 6f 77 2e 6e 65 74 2e 61 75 20 30 2e 35 20 23 31 5c 73  eats\0\sv_hostname\Warsow.net.au 0.5 #1\s
0x02FB4F77  76 5f 6d 61 78 63 6c 69 65 6e 74 73 5c 31 36 5c 73 76 5f 6d 61 78 6d 76 63 6c 69 65 6e 74 73 5c 34 5c 73 76 5f 70 70 73 5c  v_maxclients\16\sv_maxmvclients\4\sv_pps\
0x02FB4FA0  32 30 5c 73 76 5f 70 75 72 65 5c 31 5c 73 76 5f 73 6b 69 6c 6c 6c 65 76 65 6c 5c 31 5c 67 61 6d 65 74 79 70 65 5c 64 75 65  20\sv_pure\1\sv_skilllevel\1\gametype\due
0x02FB4FC9  6c 5c 63 6c 69 65 6e 74 73 5c 30 0a 00 fd fd fd fd																		    l\clients\0..����


0x02FB4EA8  ff ff ff ff 73 74 61 74 75 73 52 65 73 70 6f 6e 73 65 0a 5c 63 68 61 6c 6c 65 6e 67 65 5c 5c 76 65 72 73 69 6f 6e 5c 30 2e 35 30 20 69 33 38 36 20 41 75 67 20 32 35 20 32 30 30  ����statusResponse.\challenge\\version\0.50 i386 Aug 25 200
0x02FB4EE3  39 20 4c 69 6e 75 78 5c 66 73 5f 67 61 6d 65 5c 62 61 73 65 77 73 77 5c 67 5f 61 6e 74 69 6c 61 67 5c 31 5c 67 5f 67 61 6d 65 74 79 70 65 73 5f 61 76 61 69 6c 61 62 6c 65 5c 5c  9 Linux\fs_game\basewsw\g_antilag\1\g_gametypes_available\\
0x02FB4F1E  67 5f 69 6e 73 74 61 67 69 62 5c 30 5c 67 5f 6d 61 74 63 68 5f 73 63 6f 72 65 5c 5c 67 5f 6d 61 74 63 68 5f 74 69 6d 65 5c 57 61 72 6d 75 70 5c 67 5f 6e 65 65 64 70 61 73 73 5c  g_instagib\0\g_match_score\\g_match_time\Warmup\g_needpass\
0x02FB4F59  30 5c 67 61 6d 65 64 61 74 65 5c 41 75 67 20 32 35 20 32 30 30 39 5c 67 61 6d 65 6e 61 6d 65 5c 57 61 72 73 6f 77 5c 6d 61 70 6e 61 6d 65 5c 77 64 6d 31 34 5c 70 72 6f 74 6f 63  0\gamedate\Aug 25 2009\gamename\Warsow\mapname\wdm14\protoc
0x02FB4F94  6f 6c 5c 31 31 5c 73 76 5f 63 68 65 61 74 73 5c 30 5c 73 76 5f 68 6f 73 74 6e 61 6d 65 5c 23 4c 69 6e 75 78 2e 65 73 20 40 20 69 72 63 2e 6d 69 6e 64 66 6f 72 67 65 2e 6f 72 67  ol\11\sv_cheats\0\sv_hostname\#Linux.es @ irc.mindforge.org
0x02FB4FCF  20 7c 20 57 53 57 20 53 70 61 69 6e 5c 73 76 5f 6d 61 78 63 6c 69 65 6e 74 73 5c 31 36 5c 73 76 5f 6d 61 78 6d 76 63 6c 69 65 6e 74 73 5c 34 5c 73 76 5f 70 70 73 5c 32 30 5c 73   | WSW Spain\sv_maxclients\16\sv_maxmvclients\4\sv_pps\20\s
0x02FB500A  76 5f 70 75 72 65 5c 31 5c 73 76 5f 73 6b 69 6c 6c 6c 65 76 65 6c 5c 31 5c 67 61 6d 65 74 79 70 65 5c 64 6d 5c 63 6c 69 65 6e 74 73 5c 30 0a 00 fd fd fd fd ab ab ab ab ab ab ab  v_pure\1\sv_skilllevel\1\gametype\dm\clients\0..�����������


0x02FDF688  ff ff ff ff 73 74 61 74 75 73 52 65 73 70 6f 6e 73 65 0a 5c 63 68 61 6c 6c 65 6e 67 65 00 00 76 65 72 73 69 6f 6e 00 30 2e 35 30 20 69 33 38 36 20 41 75 67 20 32 35 20 32 30 30  ����statusResponse.\challenge..version.0.50 i386 Aug 25 200
0x02FDF6C3  39 20 4c 69 6e 75 78 00 66 73 5f 67 61 6d 65 00 62 61 73 65 77 73 77 00 67 5f 61 6e 74 69 6c 61 67 00 31 00 67 5f 67 61 6d 65 74 79 70 65 73 5f 61 76 61 69 6c 61 62 6c 65 00 00  9 Linux.fs_game.basewsw.g_antilag.1.g_gametypes_available..
0x02FDF6FE  67 5f 69 6e 73 74 61 67 69 62 00 30 00 67 5f 6d 61 74 63 68 5f 73 63 6f 72 65 00 00 67 5f 6d 61 74 63 68 5f 74 69 6d 65 00 57 61 72 6d 75 70 00 67 5f 6e 65 65 64 70 61 73 73 00  g_instagib.0.g_match_score..g_match_time.Warmup.g_needpass.
0x02FDF739  30 00 67 61 6d 65 64 61 74 65 00 41 75 67 20 32 35 20 32 30 30 39 00 67 61 6d 65 6e 61 6d 65 00 57 61 72 73 6f 77 00 6d 61 70 6e 61 6d 65 00 77 64 6d 31 37 00 70 72 6f 74 6f 63  0.gamedate.Aug 25 2009.gamename.Warsow.mapname.wdm17.protoc
0x02FDF774  6f 6c 00 31 31 00 73 76 5f 63 68 65 61 74 73 00 30 00 73 76 5f 68 6f 73 74 6e 61 6d 65 00 46 75 63 6b 69 6e 67 46 72 6f 67 73 2e 66 72 20 70 75 62 6c 69 63 20 34 00 73 76 5f 6d  ol.11.sv_cheats.0.sv_hostname.FuckingFrogs.fr public 4.sv_m
0x02FDF7AF  61 78 63 6c 69 65 6e 74 73 00 31 36 00 73 76 5f 6d 61 78 6d 76 63 6c 69 65 6e 74 73 00 34 00 73 76 5f 70 70 73 00 32 30 00 73 76 5f 70 75 72 65 00 31 00 73 76 5f 73 6b 69 6c 6c  axclients.16.sv_maxmvclients.4.sv_pps.20.sv_pure.1.sv_skill
0x02FDF7EA  6c 65 76 65 6c 00 31 00 67 61 6d 65 74 79 70 65 00 64 61 00 63 6c 69 65 6e 74 73 00 34 00 30 20 36 38 20 22 5e 39 2e 5e 37 70 5e 39 72 5e 37 30 5e 30 5b 5e 37 46 72 75 5e 39 69  level.1.gametype.da.clients.4.0 68 "^9.^7p^9r^70^0[^7Fru^9i
0x02FDF825  5e 37 54 22 20 31 0a 30 20 37 32 20 22 5e 39 2e 5e 37 70 5e 39 72 5e 37 30 5e 30 5d 5e 39 50 5e 37 69 78 65 6c 22 20 31 0a 30 20 36 35 20 22 4d 69 73 74 65 72 20 53 70 61 74 65  ^7T" 1.0 72 "^9.^7p^9r^70^0]^9P^7ixel" 1.0 65 "Mister Spate
0x02FDF860  6e 21 22 20 31 0a 30 20 34 36 20 22 47 65 65 70 61 72 64 22 20 31 0a 00 fd fd fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 db 67 51 16 bd cd 07 1c 20 6a fb  n!" 1.0 46 "Geepard" 1..������������............�gQ..�.. j�

*/



char *Q3_ParseServerRules(SERVER_RULES* &pLinkedListStart,char *p,DWORD packetlen)
{
	SERVER_RULES *pSR=NULL;
	SERVER_RULES *pCurrentSR=NULL;
	Q3DATA_SERVER_INFO *Q3SI; 
	Q2DATA_SERVER_INFO *Q2SI;
	WARSOWDATA_SERVER_INFO *WSI;
	QWDATA_SERVER_SHORTINFO *QWSI;

	pLinkedListStart = NULL;
	Q3SI = (Q3DATA_SERVER_INFO *)p;
	Q2SI = (Q2DATA_SERVER_INFO *)p;
	QWSI = (QWDATA_SERVER_SHORTINFO *)p;
	WSI =  (WARSOWDATA_SERVER_INFO*)p;

	pSR=NULL;

	char *pointer=NULL;
	

	if(strncmp((char*)&Q3SI->leadData,"statusResponse",14)!=0) //start detecting most common first
	{
		if(strncmp((char*)&WSI->leadData,"infoResponse",12)!=0)
		{
			if (QWSI->leadData[0]=='n') //could it be a Quake world or
			{
				pointer=QWSI->data;
				if(strncmp((char*)&QWSI->data,"banned",6)==0)
					return NULL;
			}
			else
			{
				if(strncmp((char*)&Q2SI->leadData,"print",5)==0) // a  Quake 2 server?
					pointer=Q2SI->data;
				else
				{
					dbg_print("Unrecognized response from server!");
					return NULL;
				}
			}
		}
		else
		{
			pointer=WSI->data;  //it is Warsow
		}

	} else
	{
		pointer=Q3SI->data;
	}

	char *end = &p[packetlen];
	while(pointer<end)
	{
		if(pSR==NULL)
		{
			pSR = (SERVER_RULES *)calloc(1,sizeof(SERVER_RULES));			
			pLinkedListStart = (SERVER_RULES *) pCurrentSR = pSR;			
		}else
		{
			pCurrentSR->pNext = (SERVER_RULES *)calloc(1,sizeof(SERVER_RULES));
			pCurrentSR = pCurrentSR->pNext;					
		}
		
		char *endString = strchr(pointer,'\\');
		if(endString!=NULL)
		{
			endString[0] = 0;
			pCurrentSR->name = _strdup(pointer);
			pointer+=(int)strlen(pointer)+1;
		}
		endString = strchr(pointer,'\\');

		if(endString!=NULL)
		{
			bool finish=false;
			endString[0] = 0;
			char *pend = strchr(pointer,0x0a);
			if(pend!=NULL) //Warsow fix
			{
				pend[0] = 0;
				finish=true;
			}

			pCurrentSR->value = _strdup(pointer);
			pointer+=strlen(pointer)+1;
			
			if(finish)
				break;

		} else
		{
			//Last server rules ends with 0x0a
			endString = strchr(pointer,0x0a);
			if(endString!=NULL)
			{
				endString[0] = 0;
				pCurrentSR->value = _strdup(pointer);
				pointer+=strlen(pointer)+1;
			//	pCurrentSR->pNext = NULL;
				break;
			} else  //QW and Q2 servers..
			{
				pCurrentSR->value = _strdup(pointer);
				pointer+=strlen(pointer)+1;
				pointer++;
			}
		}
	
		if(pointer[0]==0x0a)
			break;

	}
	return pointer;
}


DWORD Q3_ConnectToMasterServer(GAME_INFO *pGI, int iMasterIdx)
{
	size_t packetlen=0;
	char sendbuf[80];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	SOCKET ConnectSocket;

//	UINT_PTR timerProgressWait = SetTimer(g_hWnd,IDT_TIMER_1SECOND,100,NULL);
	
	int i = 0;
	unsigned char *packet[MAX_PACKETS];
	size_t packet_len[MAX_PACKETS];

	for(i=0; i<MAX_PACKETS;i++)
		packet[i] = NULL;

	i = 0;

	char szIP[260];
	strcpy(szIP,pGI->szMasterServerIP[iMasterIdx]);
	
	SplitIPandPORT(szIP,pGI->dwMasterServerPORT);

	int len = 0;//(int)strlen(sendbuf);
	string query;
	query = pGI->szMasterQueryString;
	ReplaceStrInStr(query,"%PROTOCOL%",pGI->szGameProtocol[iMasterIdx]);
	len = UTILZ_ConvertEscapeCodes((TCHAR*)query.c_str(),sendbuf,sizeof(sendbuf));

	ConnectSocket = getsockudp(szIP,(unsigned short)pGI->dwMasterServerPORT); // etmaster.idsoftware.com"27950 master server
	if(INVALID_SOCKET==ConnectSocket)
	{

		KillTimer(g_hWnd,IDT_1SECOND);
		g_log.AddLogInfo(GS_LOG_ERROR,"Error connecting to server %s:%d!",szIP,pGI->dwMasterServerPORT);
		return 1;
	}

	WSAEVENT hEvent;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		KillTimer(g_hWnd,IDT_1SECOND);
		g_log.AddLogInfo(GS_LOG_ERROR,"WSACreateEvent() = WSA_INVALID_EVENT");
		closesocket(ConnectSocket);
		return 1;
	}

	//
	// Make the socket non-blocking and 
	// associate it with network events
	//
	int nRet;
	nRet = WSAEventSelect(ConnectSocket, hEvent,FD_READ|FD_CONNECT|FD_CLOSE);
	if (nRet == SOCKET_ERROR)
	{
		KillTimer(g_hWnd,IDT_1SECOND);
		g_log.AddLogInfo(GS_LOG_ERROR,"EventSelect() = SOCKET_ERROR");
		closesocket(ConnectSocket);
		WSACloseEvent(hEvent);
		return 5;
	}



	//
	// Handle async network events
	//

	WSANETWORKEVENTS events;
	while(1)
	{
		//
		// Wait for something to happen
		//
		//dbg_print("\nWaitForMultipleEvents()");
		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1, &hEvent, FALSE,4000,FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			g_log.AddLogInfo(GS_LOG_ERROR,"WSAWaitForMultipleEvents = WSA_WAIT_TIMEOUT");
			break;
		}

		//
		// Figure out what happened
		//
		//g_log.AddLogInfo(0,"\nWSAEnumNetworkEvents()");
		nRet = WSAEnumNetworkEvents(ConnectSocket, hEvent, &events);
		if (nRet == SOCKET_ERROR)
		{
			g_log.AddLogInfo(GS_LOG_ERROR,"WSAEnumNetworkEvents() = SOCKET_ERROR");
			break;
		}

		//				 //
		// Handle events //
		//				 //

		// Connect event?
		if (events.lNetworkEvents & FD_CONNECT)
		{
			//g_log.AddLogInfo(0,"\nFD_CONNECT: %d", events.iErrorCode[FD_CONNECT_BIT]);
			g_log.AddLogInfo(0,"Master server %s",pGI->szMasterServerIP[0]);
			g_log.AddLogInfo(0,"Sending packet string: [%s]  Length: %d",sendbuf,len);
			g_log.AddLogInfo(0,"xml config: %s ",pGI->szMasterQueryString);

			if(send(ConnectSocket, sendbuf, len , 0)==SOCKET_ERROR) 
			{
				KillTimer(g_hWnd,IDT_1SECOND);
				WSACloseEvent(hEvent);
				closesocket(ConnectSocket);		
				g_log.AddLogInfo(GS_LOG_ERROR,"Error sending master query packet!");
				return 2;
			}
		}

		// Read event?
		if (events.lNetworkEvents & FD_READ)
		{
			//g_log.AddLogInfo(0,"\nFD_READ: %d, %d",events.iErrorCode[FD_READ_BIT],i);
			// Read the data and write it to stdout
			packet[i]=(unsigned char*)ReadPacket(ConnectSocket, &packetlen);
			packet_len[i] = packetlen;
			i++;

			if(i>=MAX_PACKETS)
				break;
	
		}

		// Close event?
		if (events.lNetworkEvents & FD_CLOSE)
		{
			g_log.AddLogInfo(0,"\nFD_CLOSE: %d",events.iErrorCode[FD_CLOSE_BIT]);
			break;
		}
/*
		// Write event?
		if (events.lNetworkEvents & FD_WRITE)
		{
			g_log.AddLogInfo(0,"\nFD_WRITE: %d",events.iErrorCode[FD_WRITE_BIT]);
		}
*/
	}
	
	closesocket(ConnectSocket);
	WSACloseEvent(hEvent);

	DWORD Q3_dwNewTotalServers=0;
	for(i=0; i<MAX_PACKETS;i++)
	{
		if(packet[i] != NULL)
		{
		    Q3_dwNewTotalServers += Q3_ParseServers((char*)packet[i],packet_len[i],pGI);
			free(packet[i]);			
			packet[i]=NULL;
			SetStatusText(pGI->iIconIndex,g_lang.GetString("StatusReceivingMaster"),Q3_dwNewTotalServers,pGI->szGAME_NAME);
			if(g_bCancel)
			{
				//ensure to clean up
				for(i=0; i<MAX_PACKETS;i++)
				{
					if(packet[i] != NULL)
					{
						free(packet[i]);			
						packet[i]=NULL;
					}
				}			
				break;
			}
			Sleep(50); //give cpu sometime to breath
		}
	}

//	KillTimer(g_hWnd,IDT_TIMER_1SECOND);
	pGI->dwTotalServers += Q3_dwNewTotalServers;


	return 0;
}





