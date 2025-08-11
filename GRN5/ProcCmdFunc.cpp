#include "nowarnings.h"
#include <windows.h>
#include <mbstring.h>
#include "cpau.h"
#include "ProcCmdFunc.h"
#include "psapi.h"
#include "funcs.h"
#include "tlhelp32.h"


namespace PCF{
DWORD PIDS[512];
DWORD PDS=512;
DWORD rz;
int irz;
int i;
char len;
char pn[255];
char ShortPN[128];
char KeepIFEO[256];
char StrIFEO[128];
char RegIFEO[128]="SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\";
char FullRegIFEO[128];
char temp[384];
HKEY hkey;
DWORD DataType,lenreg;
}

//#ifdef MSTSC_FIND
//#include "mstsc.hpp"
//#endif // MSTSC_FIND


//////////////////////////////////////////////////////////////////////////////////
int GetCorrectCmdLine(LPSTR Cmd, LPSTR CmdLine){//с двумя одинаковыми указателями тоже работает
using namespace PCF;

	while((char)Cmd[0]==0x20){
		Cmd++;
	}

strcpy(CmdLine,Cmd);

len=strlen(CmdLine)-1;


while((CmdLine[len]==0x20)&&(len!=0)){
	CmdLine[len]=0;
	len--;
}

return 1;

}



/////////////////////////////////////////////////////////////////////////////////
int GetShortModuleName(LPSTR Full, LPSTR Short){
using namespace PCF;
char tmp[256];
char* pshort;
int i, len;
i=1;
strcpy(tmp,Full);
pshort=tmp;
len=strlen(tmp);
	if(*pshort==0x22){
		pshort++;
		while(i<len){
			if(tmp[i]==0x22){
				tmp[i]=0;
				break;
			}
			if(tmp[i]==0x5C){
				pshort=&tmp[i+1];
			}
		i++;
		}
	strcpy(Short,pshort);
	return 1;
	}
	
	else{//*pshort!=0x22
		while(i<len){
			if(tmp[i]==0x20||tmp[i]==0){
				tmp[i]=0;
				break;
			}
			if(tmp[i]==0x5C){
				pshort=&tmp[i+1];
			}
		i++;
		}
	strcpy(Short,pshort);
	return 1;

	}

}

////////////////////////////////////////////////////////////////////////////////
int SetProcDirectory(LPSTR Full){
using namespace PCF;

int i, len;


strcpy(temp,Full);
len=strlen(temp);
i=len-1;
	while(i>0){
		if(temp[i]==0x5C){
			break;
		}
	i--;	
	}
temp[i]=0;
	if(strlen(temp)==0){
	return 0;
	}
	else{
		if(SetCurrentDirectory(temp)==0){
			return 0;
		}
	return 1;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
int GetFullModuleNameAndParams(LPSTR Full, LPSTR FullModuleName, LPSTR Params, LPSTR SelfParams){
using namespace PCF;
char tmp[256];
char* pfname;
char* pparam;
char* psparam;
int i, len;
i=1;
*FullModuleName=0;
*Params=0;
*SelfParams=0;
strcpy(tmp,Full);
pfname=tmp;
len=strlen(tmp);
if(*pfname==0x22){
		pfname++;
		while(i<len){
			if(tmp[i]==0x22){
				tmp[i]=0;
				i++;
				break;
			}
		i++;	
		}
FMN:		
strcpy(FullModuleName,pfname);
pparam=&tmp[i];
len=strlen(pparam); 
	if(len>=1){
		i=1;
		while(*pparam==0x20 && i<=len){//очистка строки параметров от пробелов впереди
			*pparam=0;
			pparam++; i++;
		}

	}
strcpy(Params,pparam);
		len=strlen(pparam);
		i=1;
		psparam=pparam+len-1;
		while(*psparam!=0x20 && i<=len){
			if(*psparam==0x40 && *(psparam-1)==0x40){
				if((len-i-2)>0){//строка параметров программы + своя строка @@
				strncpy(Params,pparam,(len-i-2));
				*(Params+len-i-1)=0;
				GetCorrectCmdLine(Params,Params);
				}
				else{//len-i-2)<=0 только своя строка @@
				*Params=0;
				}
				strcpy(SelfParams, psparam+1);
				return 1;

			}

		psparam--; i++;	

		}


	return 1;
}

else{//*pfname!=0x22
	while(i<len){
			if(tmp[i]==0x20||tmp[i]==0){
				tmp[i]=0;
				i++;
				break;
			}
		i++;	
	}

}
goto FMN;
}
////////////////////////////////////////////////////////////////////////////////////////////
int ClearIFEO(LPSTR Cmd){
using namespace PCF;
GetShortModuleName(Cmd,StrIFEO);
strcpy(FullRegIFEO,RegIFEO);
strcat(FullRegIFEO,StrIFEO);
if(RegOpenKey(HKEY_LOCAL_MACHINE,FullRegIFEO,&hkey)!=ERROR_SUCCESS){
KeepIFEO[0]=0;
return 0;
}
lenreg=256;
RegQueryValueEx(hkey,"Debugger",0,&DataType,(unsigned char *)KeepIFEO,&lenreg);
lenreg=0;
RegSetValueEx(hkey,"Debugger",0,REG_SZ,(unsigned char *)"",lenreg);
RegCloseKey(hkey);
return 1;
}
///////////

int RestoreIFEO(){
using namespace PCF;
if(!KeepIFEO[0]){return 0;}

if(RegOpenKey(HKEY_LOCAL_MACHINE,FullRegIFEO,&hkey)!=ERROR_SUCCESS){
KeepIFEO[0]=0;
return 0;
}
lenreg=strlen(KeepIFEO);
RegSetValueEx(hkey,"Debugger",0,REG_SZ,(unsigned char *)KeepIFEO,lenreg);
RegCloseKey(hkey);
KeepIFEO[0]=0;
return 1;
}

//////////////////////////////////////////////////////////////////////////////////
int GetFlagAndCorrect(LPSTR Cmd, int* POperFlag){
using namespace PCF;

#define NUM_FLAGS 9
const char *PFlags[]={"`close`",	//=1 Close only
				"`close&`", //=2 Close And Run
				"`kill`",	//=3 Kill only
				"`kill&`",	//=4 Kill And Run
				"`wait`",	//=5 Ждать завершения данного процесса
				"`&wait`",  //=6 Запустить данный процесс и ждать его завершения
				/*7 и 8 - операции по установке флага FlagIs, единичное значение которого является условием для выполнения любой из первых шести операций,
				 по умолчанию флаг всегда в 1, и после каждой из первых шести операций, вне зависимости выполнилась она или нет, флаг FlagIs также устанавливается в 1*/
				"`is`",		//=7 проверка наличия процесса: 1 -есть, 0 -нет
				"`is!`",	//=8 проверка отсутствия процесса: 1 - отсутствует, 0 -присутсвует
				"`sys`",    //=9 запуск процесса от имени system
				};		
LPSTR CmdOrig;
char Sec[64];
int TimeOut;
int i,j,l;

CmdOrig=Cmd;
TimeOut=0;
i=j=l=0;
*POperFlag=0;
GetCorrectCmdLine(Cmd, Cmd);

if(Cmd[0]!='`') //нет флагов
goto CO;

while(i<NUM_FLAGS){
l=strlen(PFlags[i]);
	if(_mbsnbicmp((const unsigned char *)PFlags[i],(const unsigned char *)Cmd,l)==0){
	*POperFlag=i+1;
	Cmd+=l;

			if(Cmd[0]==0x20 && (i==4||i==5)){TimeOut=-1; break;}
			if(Cmd[0]==0x20 && i<2){TimeOut=40; break;}
		
			if(Cmd[0]!=0x20){
				while(j<3){
					Sec[j]=Cmd[0];
					j++; Cmd++;
					if(Cmd[0]==0x20){
					Sec[j]=0;
					break;
					}
				}
				TimeOut=atoi(Sec);
			}//endif(Cmd[0]!=0x20
		break;


	}//enfif _mbsnbicmp

	i++;
}

//from break

GetCorrectCmdLine(Cmd, Cmd);
CO:
strcpy(CmdOrig,Cmd);

return TimeOut;
}

//////////////////////////////////////////////////////////////////////////////////
DWORD GetProcessID(char* ProcStr){
using namespace PCF;
	GetShortModuleName(ProcStr,ShortPN);
	ZeroMemory(PIDS,sizeof(PIDS));
	EnumProcesses(PIDS,sizeof(PIDS),&PDS);
	
	for(i=0;i<512;i++){
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ|SYNCHRONIZE,
                                   FALSE, PIDS[i] );
		if ( hProcess )
		{
			HMODULE hMod;
			DWORD cbNeeded;
			if (EnumProcessModules( hProcess, &hMod, sizeof(hMod),&cbNeeded))
			{
			 if (!GetModuleBaseName( hProcess, hMod, pn,sizeof(pn) )){
				 
				 CloseHandle( hProcess );
				return 0;
			 }
			 if(!stricmp(pn,ShortPN)){
					 CloseHandle( hProcess );
					 return PIDS[i];
				 }
			CloseHandle( hProcess );
			}
			else{//(!EnumProcessModules
			CloseHandle( hProcess );
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
int WaitProcess(char* ProcStr, DWORD TimeOut){
using namespace PCF;
GetShortModuleName(ProcStr,ShortPN);

	ZeroMemory(PIDS,sizeof(PIDS));
	EnumProcesses(PIDS,sizeof(PIDS),&PDS);
	
	for(i=0;i<512;i++){
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ|SYNCHRONIZE,
                                   FALSE, PIDS[i] );
		if ( hProcess )
		{
			HMODULE hMod;
			DWORD cbNeeded;
			if (EnumProcessModules( hProcess, &hMod, sizeof(hMod),&cbNeeded))
			{
			 if (!GetModuleBaseName( hProcess, hMod, pn,sizeof(pn) )){
				 
				 CloseHandle( hProcess );
				return 0;
			 }
			 if(!stricmp(pn,ShortPN)){
					 rz=WaitForSingleObject(hProcess,TimeOut);
					 CloseHandle( hProcess );
					 if(rz==WAIT_TIMEOUT)
					 return -1;
					 
					 else
					 return 1;

				 }
			CloseHandle( hProcess );
			}
			
			else{//(!EnumProcessModules
			CloseHandle( hProcess );
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
int IsProcessExist(char* ProcStr, HANDLE *hProc){
using namespace PCF;

	GetShortModuleName(ProcStr,ShortPN);
	ZeroMemory(PIDS,sizeof(PIDS));
	EnumProcesses(PIDS,sizeof(PIDS),&PDS);
	
	for(i=0;i<512;i++){
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ|SYNCHRONIZE,
                                   FALSE, PIDS[i] );
		if ( hProcess )
		{
			HMODULE hMod;
			DWORD cbNeeded;
			if (EnumProcessModules( hProcess, &hMod, sizeof(hMod),&cbNeeded))
			{
			 if (!GetModuleBaseName( hProcess, hMod, pn,sizeof(pn) )){
				 
				 CloseHandle( hProcess );
				return 0;
			 }
				if(!stricmp(pn,ShortPN)){
					if (hProc == NULL) {
					 CloseHandle(hProcess);
					}
					else {
					 *hProc = hProcess;
					}
					 return 1;
				}
			CloseHandle( hProcess );
			}
			
			else{//(!EnumProcessModules
			CloseHandle( hProcess );
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
int KillProcess(char* ProcStr){
using namespace PCF;
int ret;
ret=0;
	GetShortModuleName(ProcStr,ShortPN);
ZM:
	ZeroMemory(PIDS,sizeof(PIDS));
	EnumProcesses(PIDS,sizeof(PIDS),&PDS);
	
	for(i=0;i<512;i++){
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ|SYNCHRONIZE|PROCESS_TERMINATE,
                                   FALSE, PIDS[i] );
		if ( hProcess )
		{
			HMODULE hMod;
			DWORD cbNeeded;
			if (EnumProcessModules( hProcess, &hMod, sizeof(hMod),&cbNeeded))
			{
			 if (!GetModuleBaseName( hProcess, hMod, pn,sizeof(pn) )){
				 
				 CloseHandle( hProcess );
				return ret;
			 }
			 if(!stricmp(pn,ShortPN)){
					 TerminateProcess(hProcess,0);
					 WaitForSingleObject(hProcess,-1);
					 CloseHandle( hProcess );
					 ret++;
					 goto ZM;
				 }
			CloseHandle( hProcess );
			}
			
			else{//(!EnumProcessModules
			CloseHandle( hProcess );
			}
		}
	}

	return ret;
}
//////////////////////////////////////////////////////////////////////////////////
int CloseProcess(char* ProcStr, DWORD TimeWait){
using namespace PCF;
HANDLE hProcess;
DWORD PID;

PID=GetProcessID(ProcStr);
hProcess=OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE,FALSE,PID);
if(hProcess==0){
	return 0;}

EnumWindows(CloseAppEnumWindows,PID);

TimeWait*=1000;
irz=WaitProcess(ProcStr,TimeWait);
CloseHandle( hProcess );
if(irz==-1){
KillProcess(ProcStr);
return -1;
}
if(irz==1)
return 1;

if(irz==0)
return 0;

return 0;
}


static
BOOL CALLBACK CloseAppEnumWindows(HWND hWnd, LPARAM lParam){

DWORD dwProcessID;

GetWindowThreadProcessId(hWnd,&dwProcessID);

if(IsWindowVisible(hWnd)&&
   dwProcessID==(DWORD)lParam)
   PostMessage(hWnd,WM_CLOSE,0,0);

return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
int StartAndWaitProcess(HANDLE hToken,char* Desk,char* ProcStr, DWORD TimeOut, char* PRN, int FlagIFEO){
using namespace PCF;
STARTUPINFOW si;
PROCESS_INFORMATION pi;
char RunStr[256];
char ProgParams[128];
char SelfParams[128];
LPWSTR PWCmd = NULL;

if(FlagIFEO)
ClearIFEO(ProcStr);

GetFullModuleNameAndParams(ProcStr,RunStr,ProgParams,SelfParams);
rz=SetProcDirectory(RunStr);
if(strlen(ProgParams)!=0){
	if(strcmp(ProgParams,"%%%%")==0) //признак взять оригинальные параметры запуска
	strcpy(ProgParams,PRN);
strcat(RunStr," ");
strcat(RunStr,ProgParams);
}

ZeroMemory(&si,sizeof(STARTUPINFOW));
ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
si.dwFlags = STARTF_USESHOWWINDOW;
	if(strlen(SelfParams)!=0){
		si.wShowWindow=atoi(SelfParams);
	}
	else{
		si.wShowWindow=SW_SHOWNORMAL;
	}

PWCmd = AnsiToUnicode(RunStr);
rz=CreateProcessW(NULL,PWCmd,NULL,NULL,0,0,NULL,NULL,&si,&pi);
//rz=CPAU(hToken,Desk,NULL,NULL,NULL,RunStr,&si,&pi);


if(FlagIFEO)
RestoreIFEO();

if(rz!=0 && TimeOut!=0)
rz=WaitForSingleObject(pi.hProcess,TimeOut*1000);
//ещё хэндлы здесь от процесса закрывать будем
if (pi.hThread != 0) {
	CloseHandle(pi.hThread);
	pi.hThread = 0;
}
if(pi.hProcess != 0) {
	CloseHandle(pi.hProcess);
	pi.hProcess = 0;
}

return 1;
}

//////////////////////////////////////////////////////////////////////////////////
int StartAndWaitProcess0(HANDLE hToken,char* ProcStr, DWORD TimeOut, int HS){
using namespace PCF;
STARTUPINFOW si;
PROCESS_INFORMATION pi;
char RunStr[256];
char ProgParams[128];
char SelfParams[128];
char Desk[32]="winsta0\\Default";

GetFullModuleNameAndParams(ProcStr,RunStr,ProgParams,SelfParams);
rz=SetProcDirectory(RunStr);
if(strlen(ProgParams)!=0){
strcat(RunStr," ");
strcat(RunStr,ProgParams);
}

ZeroMemory(&si,sizeof(STARTUPINFOW));

	si.wShowWindow=HS;

//rz=CreateProcess(NULL,RunStr,NULL,NULL,0,0,NULL,NULL,&si,&pi);
rz=CPAU(hToken,Desk,NULL,NULL,NULL,RunStr,&si,&pi);



if(rz!=0 && TimeOut!=0)
rz=WaitForSingleObject(pi.hProcess,TimeOut*1000);
//ещё хэндлы здесь от процесса закрывать будем
if(rz!=0){
CloseHandle(pi.hThread);
CloseHandle(pi.hProcess);
}


return 1;
}

///////////////////int CreateProcessAndWait(char* ProcStr, int HS, DWORD TimeOut = 0)///////////////////////
int CreateProcessAndWait(char* ProcStr, int HS, DWORD TimeOut, DWORD *PDWID) {
STARTUPINFO si;
PROCESS_INFORMATION pi;
char RunStr[256];
char ProgParams[128];
char SelfParams[128];
DWORD DWWait;
int rez, ret;

rez = ret = 0;
/*
GetFullModuleNameAndParams(ProcStr, RunStr, ProgParams, SelfParams);
SetProcDirectory(RunStr);
if (strlen(ProgParams) != 0) {
	strcat(RunStr, " ");
	strcat(RunStr, ProgParams);
}
*/
memset(&si, 0, sizeof(STARTUPINFO));
memset(&pi, 0, sizeof(PROCESS_INFORMATION));
si.cb = sizeof(STARTUPINFO);
si.dwFlags = STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW;
if (!HS) {
	si.wShowWindow = SW_HIDE;
}
else {
	si.wShowWindow = SW_SHOWNORMAL;
}
DWWait = TimeOut;
if (PDWID != 0) {
	*PDWID = 0;
}
	if (CreateProcess(NULL, ProcStr, NULL, NULL, 0, 0, NULL, NULL, &si, &pi)) {
		if (DWWait != 0) {
			rez=WaitForSingleObject(pi.hProcess, DWWait);
		}
		if (rez == WAIT_OBJECT_0) {
			ret = 1;
		}
		if (rez == WAIT_TIMEOUT) {
			ret = 2;
		}
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	if (PDWID != 0) {
		*PDWID = pi.dwProcessId;
	}
	return ret;
}


////int CreateProcAndWaitExitCode(char* ProcStr, int HS, DWORD TimeOut, DWORD *PEC)////////
int CreateProcAndWaitExitCode(char* ProcStr, int HS, DWORD TimeOut, DWORD *PEC) {
STARTUPINFO si;
PROCESS_INFORMATION pi;
char RunStr[256];
char ProgParams[128];
char SelfParams[128];
DWORD DWWait;
int rez, ret;

	rez = ret = 0;

	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW;
	if (!HS) {
		si.wShowWindow = SW_HIDE;
	}
	else {
		si.wShowWindow = SW_SHOWNORMAL;
	}
	DWWait = TimeOut;
	
	if (CreateProcess(NULL, ProcStr, NULL, NULL, 0, 0, NULL, NULL, &si, &pi)) {
		if (DWWait != 0) {
			rez = WaitForSingleObject(pi.hProcess, DWWait);
		}
		if (rez == WAIT_OBJECT_0) {
			ret = 1;
		}
		if (rez == WAIT_TIMEOUT) {
			ret = 2;
		}
		if (PEC != 0 && DWWait != 0) {
			GetExitCodeProcess(pi.hProcess, PEC);
		}
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	
	return ret;
	
}

////////////////////////////int WaitChildProcesses(DWORD PID)///////////////////////////
int WaitChildProcesses(DWORD PID) {
PROCESSENTRY32 PE32 = { 0 };
BOOL brez;
DWORD WaitPID = 0;
HANDLE hChProc;
int ProcCount = 0;

WCP_CTHS:
HANDLE HCTHS = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

brez = Process32First(HCTHS, &PE32);
PE32.dwSize = sizeof(PROCESSENTRY32);
	do {
		PE32.dwSize = sizeof(PROCESSENTRY32);
		brez = Process32Next(HCTHS, &PE32);
		if (PE32.th32ParentProcessID == PID) {
			WaitPID = PE32.th32ProcessID;
			break;
		}
	} while (brez == true);

	CloseHandle(HCTHS);
	if (WaitPID == 0 && !ProcCount) {
		return 0;
	}
	if (WaitPID == 0 && ProcCount) {
		return 1;
	}
	else {
		hChProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE,
			FALSE, WaitPID);
		if (hChProc != NULL) {
			WaitForSingleObject(hChProc, INFINITE);
			CloseHandle(hChProc);
			ProcCount++;
		}
		else {//hChProc == NULL)
			return 0;
		}
		WaitPID = 0;
		goto WCP_CTHS;
	}
	return 1;
}




