#include <windows.h>
#include <Winnetwk.h>
#include "Structs.h"

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Mpr.lib")
//LogServiceCode
#define NEW_BLOCK_LOG (WORD)1
#define REMEMBER_OFFSET (WORD)2
//LogServiceCode

const char ProfileDir[] = "\\GRN5";
const char LogName[] = "GRN5.log";

int LogMsg(char *LogMessage);
int LogMsg(char *LogMessage, int i);
int LogMsg(char *LogMessage, char *str2);
int LogMsg(char *LogMessage, char *str2, char *str3);
int LogMsg(char *LogMessage, char *str2, int i);
int LogMsg(char *LogMessage, int i, char *str2);
int LogMsg(char *LogMessage, int i, char *str2, int i2);
int LogMsg(char *LogMessage, UINT64 ui, char *str2, UINT64 ui2);
int LogMsg(char *LogMessage, char *str2, char *str3, int i);
int LogMsg(char *LogMessage, char *str2, int i, char *str3, int i2);
int LogMsg(char *LogMessage, char *str2, char *str3, char *str4);
int LogMsg(WORD LogServiceCode);
int LogMsgi(int i);
int LogMsgWD(LPSTR Wrkdir,LPSTR LogMessage);
int LogMsgiWD(LPSTR Wrkdir, int i);
