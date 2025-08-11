#include <stdio.h>
#include <windows.h>
#include "LogMsg.h"

static char LogDir[192] = { 0,0 };
int I, I2;
UINT64 UI, UI2;
char StrUI[64];
char StrUI2[64];
char *Str2, *Str3, *Str4;
DWORD DWSize;
DWORD StoredOffset = 0; //смещение в файле лога, данные после которого будут отправлены в стевой файл через функцию CreateNetLog
BOOL RemeberOffset = FALSE;
//LOG_MODE
#define STR_NO_TIME 0
#define STR 1
#define STR_INT 2
#define STR2 3
#define STR2_INT 4
#define STR3 5
#define STR3_INT 6
#define STR_INT_STR 7
#define STR_INT_STR_INT 8
#define STR2_INT_STR_INT 9
#define STR4 10
#define STR_UINT_STR_UINT 11
//LOG_MODE
int LOG_MODE;

//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage) {

if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
LOG_MODE = STR;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, int i) {
if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
I = i; LOG_MODE = STR_INT;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, char *str2) {
if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
LOG_MODE = STR2;
Str2 = str2;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, char *str2, int i) {
if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
LOG_MODE = STR2_INT;
Str2 = str2; I = i;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, char *str2, char *str3) {
if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
LOG_MODE = STR3;
Str2 = str2; Str3 = str3;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, int i, char *str2) {
	if (!LogDir[0]) {
		GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
		strcat(LogDir, ProfileDir);
	}
	LOG_MODE = STR_INT_STR;
	Str2 = str2; I = i;
	return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, int i, char *str2, int i2) {
if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
LOG_MODE = STR_INT_STR_INT;
Str2 = str2; I = i; I2 = i2;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, UINT64 ui, char *str2, UINT64 ui2) {
	if (!LogDir[0]) {
		GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
		strcat(LogDir, ProfileDir);
	}
	LOG_MODE = STR_UINT_STR_UINT;
	Str2 = str2; UI = ui; UI2 = ui2;
	return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, char *str2, char *str3, int i) {
if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
LOG_MODE = STR3_INT;
Str2 = str2; Str3 = str3; I = i;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, char *str2, int i, char *str3, int i2) {
if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
LOG_MODE = STR2_INT_STR_INT;
Str2 = str2; Str3 = str3; I = i; I2 = i2;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsg(char *LogMessage, char *str2, char *str3, char *str4) {
if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
LOG_MODE = STR4;
Str2 = str2; Str3 = str3; Str4 = str4;
return LogMsgWD(LogDir, LogMessage);
}
//-----------------------------------------------------------------------------------------
int LogMsgWD(LPSTR Wrkdir,LPSTR LogMessage){

SYSTEMTIME STLOG;
SYSTEMTIME ST;
HANDLE hFLog;
int ist;
int NOpen=5;
char FullLogName[256];
char StMsg[256];
DWORD NOBW_FLOG;

NOpen=5;
ZeroMemory(StMsg, sizeof(StMsg));
GetSystemTime(&ST);

switch (LOG_MODE)
{
case STR_NO_TIME:
	sprintf(StMsg, "%s\r\n", LogMessage);
	break;
case STR:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage);
	break;
case STR_INT:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%d\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, I);
	break;
case STR2:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%s\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, Str2);
	break;
case STR3:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%s%s\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, Str2, Str3);
	break;
case STR2_INT:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%s%d\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, Str2, I);
	break;
case STR_INT_STR:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%d%s\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, I, Str2);
	break;
case STR_INT_STR_INT:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%d %s%d\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, I, Str2, I2);
	break;
case STR3_INT:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%s %s%d\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, Str2, Str3, I);
	break;
case STR2_INT_STR_INT:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%s%d %s%d\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, Str2, I, Str3, I2);
	break;
case STR4:
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%s%s%s\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, Str2, Str3, Str4);
	break;
case STR_UINT_STR_UINT:
	_ui64toa(UI, StrUI, 10); _ui64toa(UI2, StrUI2, 10);
	sprintf(StMsg, "%02d.%02d.%4d %02d:%02d_%02d.%003d %s%s %s%s\r\n", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute,
		ST.wSecond, ST.wMilliseconds, LogMessage, StrUI, Str2, StrUI2);
	break;

default:
	break;
}

sprintf(FullLogName, "%s\\%s", Wrkdir, LogName);

openLog:
hFLog=CreateFile(FullLogName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0,
	OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
if(hFLog==INVALID_HANDLE_VALUE){
	Sleep(1);
	NOpen--;
	if(NOpen){
		goto openLog; }
	else{return 0;}
}
DWSize = SetFilePointer(hFLog, NULL, NULL, FILE_END);
if (RemeberOffset) {
	StoredOffset = DWSize; RemeberOffset = FALSE;
}
WriteFile(hFLog,StMsg,strlen(StMsg),&NOBW_FLOG,NULL);

CloseHandle(hFLog);
 return 1;
}
//-----------------------------------------------------------------------------------------
int LogMsgiWD(LPSTR Wrkdir, int i) {
	char buff[256];
	LogMsgWD(Wrkdir, itoa(i, buff, 10));
	return 1;
}
//-----------------------------------------------------------------------------------------
int LogMsgi(int i) {
char buff[256];
	
	if (!LogDir[0]) {
		GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
		strcat(LogDir, ProfileDir);
	}
	LOG_MODE = STR;
	return LogMsgWD(LogDir, itoa(i, buff, 10));
}

//-----------------------------------------------------------------------------------------
int LogMsg(WORD LogServiceCode) {
char LogMessage[256] = "================================================================================================================";
int ret = 0;

if (!LogDir[0]) {
	GetEnvironmentVariable("PROGRAMDATA", LogDir, sizeof(LogDir));
	strcat(LogDir, ProfileDir);
}
	if (LogServiceCode&REMEMBER_OFFSET) {
		RemeberOffset = TRUE;
		ret = 1;
	}
	if (LogServiceCode&NEW_BLOCK_LOG) {
		LOG_MODE = STR_NO_TIME;
		return LogMsgWD(LogDir, LogMessage);
	}
return ret;
}

