#pragma once

#define ERR_GET_DIR_SIZE (UINT64)(~0)

UINT64 GetDirectorySize(char *DirPath, BOOL RCall = FALSE);
int DeleteDirectory(char *DirPath, BOOL RCall = FALSE);
int FlushDirectory(char *DirPath, BOOL RCall = FALSE);
int ClearDirectory(char *DirPath, BOOL RCall = FALSE);
typedef BOOL(CALLBACK* DIRFILESENUMPROC)(WIN32_FIND_DATA *PFD, char *DirPath, BOOL AsPreviousDir, void *AddPar);
int EnumDirFiles(char *DirPath, DIRFILESENUMPROC DFEnumProc, void *AddPar = NULL);