#include "nowarnings.h"
#include <iostream>
#include <Windows.h>
#include <string.h>
#include "directory.h"
#include "funcs.h"


UINT64 GetDirectorySize(char *DirPath, BOOL RCall) {
UINT64 DirSize = 0;
UINT64 DSumm;
WIN32_FIND_DATA FD;
HANDLE hf = INVALID_HANDLE_VALUE;
char FindEl[256];
char DirPathR[256];

sprintf(FindEl, "%s\\%s", DirPath, "*");
hf = FindFirstFile(FindEl, &FD);
if (hf != INVALID_HANDLE_VALUE) {
	do {
		if (FD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {//директория
			if (strcmp(FD.cFileName, ".") && strcmp(FD.cFileName, "..")) {
				sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
				DirSize += GetDirectorySize(DirPathR, 1);
			}
		}
		else { //файл
			DSumm = FD.nFileSizeHigh;
			DSumm <<= 32;
			DSumm += FD.nFileSizeLow;
			DirSize += DSumm;
		}

	} while (FindNextFile(hf, &FD) != 0);
	FindClose(hf);
	return DirSize;
}
else {//hf == INVALID_HANDLE_VALUE
	if (!RCall) {
		return ERR_GET_DIR_SIZE;
	}
	else {//Recursive Call
		return 0;
	}
}
	
return 0;
}

//////////////////////////int DeleteDirectory(char *DirPath)//////////////////////////
int DeleteDirectory(char *DirPath, BOOL RCall) {
WIN32_FIND_DATA FD;
HANDLE hf = INVALID_HANDLE_VALUE;
char FindEl[256];
char DirPathR[256];

	sprintf(FindEl, "%s\\%s", DirPath, "*");
	hf = FindFirstFile(FindEl, &FD);
	if (hf != INVALID_HANDLE_VALUE) {
		do {
			if (FD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {//директория
				if (strcmp(FD.cFileName, ".") && strcmp(FD.cFileName, "..")) {
					sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
					DeleteDirectory(DirPathR, 1);
					RemoveDirectoryA(DirPathR);
				}
			}
			else { //файл
				sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
				if (!DeleteFileA(DirPathR)) {
					if (GetLastError() == ERROR_ACCESS_DENIED) {
						SetFileAttributesA(DirPathR, FILE_ATTRIBUTE_NORMAL);
						DeleteFileA(DirPathR);
					}
				}
			}

		} while (FindNextFile(hf, &FD) != 0);
		FindClose(hf);
		if (!RCall) {
			RemoveDirectoryA(DirPath);
		}
		return 1;
	}
	else {//hf == INVALID_HANDLE_VALUE
		if (!RCall) {
			return 0;
		}
		else {//Recursive Call
			return 1;
		}
	}

	return 0;

}

///////////////////////int FlushDirectory(char *DirPath, BOOL RCall = FALSE)///////////////////////
int FlushDirectory(char *DirPath, BOOL RCall) {
WIN32_FIND_DATA FD;
HANDLE hf = INVALID_HANDLE_VALUE;
char FindEl[256];
char DirPathR[256];
HANDLE hFile;
BOOL FB;

	sprintf(FindEl, "%s\\%s", DirPath, "*");
	hf = FindFirstFile(FindEl, &FD);
	if (hf != INVALID_HANDLE_VALUE) {
		do {
			if (FD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {//директория
				if (strcmp(FD.cFileName, ".") && strcmp(FD.cFileName, "..")) {
					sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
					FlushDirectory(DirPathR, 1);
				}
			}
			else { //файл
				sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
				hFile = CreateFile(DirPathR, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
				if (hFile != INVALID_HANDLE_VALUE) {
					FB=FlushFileBuffers(hFile);
					CloseHandle(hFile);
				}
			}
		} while (FindNextFile(hf, &FD) != 0);
		FindClose(hf);
		return 1;
	}
	else {//hf == INVALID_HANDLE_VALUE
		if (!RCall) {
			return 0;
		}
		else {//Recursive Call
			return 1;
		}
	}
	return 0;
}

///////////////////////int ClearDirectory(char *DirPath, BOOL RCall = FALSE)///////////////////////
int ClearDirectory(char *DirPath, BOOL RCall){
	WIN32_FIND_DATA FD;
	HANDLE hf = INVALID_HANDLE_VALUE;
	char FindEl[256];
	char DirPathR[256];
	HANDLE hFile;
	BOOL FB;

	sprintf(FindEl, "%s\\%s", DirPath, "*");
	hf = FindFirstFile(FindEl, &FD);
	if (hf != INVALID_HANDLE_VALUE) {
		do {
			if (FD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {//директория
				if (strcmp(FD.cFileName, ".") && strcmp(FD.cFileName, "..")) {
					sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
					ClearDirectory(DirPathR, 1);
				}
			}
			else { //файл
				sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
				if (StriStr(DirPathR, ".wav") || StriStr(DirPathR, ".mp3")) {
					DeleteFile(DirPathR);
				}
			}
		} while (FindNextFile(hf, &FD) != 0);
		FindClose(hf);
		return 1;
	}
	else {//hf == INVALID_HANDLE_VALUE
		if (!RCall) {
			return 0;
		}
		else {//Recursive Call
			return 1;
		}
	}
	return 0;
}

////////////int EnumDirFiles(char *DirPath, DIRFILESENUMPROC DFEnumProc, void *AddPar)/////////////
int EnumDirFiles(char *DirPath, DIRFILESENUMPROC DFEnumProc, void *AddPar) {
	WIN32_FIND_DATA FD;
	HANDLE hf = INVALID_HANDLE_VALUE;
	char FindEl[256];
	char DirPathR[256];
	char DirPathPrev[256] = { 0,0 };
	BOOL AsPrevDir;

	sprintf(FindEl, "%s\\%s", DirPath, "*");
	hf = FindFirstFile(FindEl, &FD);
	if (hf != INVALID_HANDLE_VALUE) {
		do {
			if (FD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {//директория
				if (strcmp(FD.cFileName, ".") && strcmp(FD.cFileName, "..")) {
					sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
					EnumDirFiles(DirPathR, DFEnumProc, AddPar);
				}
			}
			else { //файл
				AsPrevDir = DirPathPrev[0] && !strcmp(DirPath, DirPathPrev);
				strcpy(DirPathPrev, DirPath);
				DFEnumProc(&FD, DirPath, AsPrevDir, AddPar);
			}

		} while (FindNextFile(hf, &FD) != 0);
		FindClose(hf);
		return 1;
	}
	else {//hf == INVALID_HANDLE_VALUE
		return 0;
	}

	return 0;
}