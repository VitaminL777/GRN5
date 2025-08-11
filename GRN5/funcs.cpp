
#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <stdio.h>
#include "funcs.h"

int StriStr(char *Str, char *SubStr, char **SubCh) {

int len;
char str1[256];
char str2[256];
char *pch;

if (!Str || !SubStr) {
	return 0;
}

len = strlen(Str);
if (len > 256 || !len) {return 0;}
str1[len] = 0;

while (len>=0) {
	str1[len] = tolower(Str[len]);
	len--;
}
len = strlen(SubStr);
if (len > 256 || !len) { return 0; }
str2[len] = 0;

while (len >= 0) {
	str2[len] = tolower(SubStr[len]);
	len--;
}
pch = strstr(str1, str2);
if (pch != 0) {
	if (SubCh) {
		*SubCh = Str + (pch - str1);
	}
	return 1;
}

return 0;
}

///////////////////int GetBaseName(char *FullName, char *BaseName, char *Ext)//////////////////////
int GetBaseName(char *FullName, char *BaseName, char *Ext) {
	char *pwav = 0;
	DWORD *DWwav = 0;

	strcpy(BaseName, FullName);
	if (Ext) {
		StriStr(BaseName, Ext, &pwav);
	}
	else {
		StriStr(BaseName, ".WAV", &pwav);
	}
	if (pwav) {
		DWwav = (DWORD*)pwav;
		*DWwav = 0;
		return 1;
	}
	return 0;
}

/////////////////////////void DeCPT(char *Data, int size)//////void CPT(char *Data, int size)///////////////////////////////////
void DeCPT(char *Data, int size) {
	extern BOOL IsCrypt;
	char KeyCPT[32] = "UjcgjlbCgfcbBCj[hfyb";
	int KeyLen;
	int i, j;
	char k2, ko;
	char diff;
	char CptPrePrev = 0, CptPrev = 0;
	char SavData;

	if (!IsCrypt || size == 0) {
		return;
	}
	KeyLen = strlen(KeyCPT);
	j = 0; k2 = 0; diff = 1;
	for (i = 0; i < size; i++) {
		
		if (1 & diff) {
			k2 += diff; }
		else {
			k2 -= diff; }
		
		if (1 & i) {
			k2 ^= (CptPrev+ CptPrePrev);
		}
		else {
			k2 += (CptPrev- CptPrePrev);
		}
		
		ko = k2^KeyCPT[j];
		
		CptPrev = Data[i];
		if (i > 0) {
			CptPrePrev = SavData;
		}
		SavData = Data[i];
		Data[i] ^= ko;
		
		j++; diff++;
		if (j >= KeyLen) {
			j = 0;}
	}
}

void CPT(char *Data, int size) {
	extern BOOL IsCrypt;
	char KeyCPT[32] = "UjcgjlbCgfcbBCj[hfyb";
	int KeyLen;
	int i, j;
	char k2, ko;
	char diff;
	char CptPrePrev=0, CptPrev=0;

	if (!IsCrypt || size == 0) {
		return;
	}
	KeyLen = strlen(KeyCPT);
	j = 0; k2 = 0; diff = 1;
	for (i = 0; i < size; i++) {

		if (1 & diff) {
			k2 += diff;
		}
		else {
			k2 -= diff;
		}
		
		if (1 & i) {
			k2 ^= (CptPrev+ CptPrePrev);
		}
		else {
			k2 += (CptPrev- CptPrePrev);
		}
		
		ko = k2 ^ KeyCPT[j];
		
		Data[i] ^= ko;
		CptPrev = Data[i];
		if (i > 0) {
			CptPrePrev = Data[i - 1];
		}
		j++; diff++;
		if (j >= KeyLen) {
			j = 0;
		}
	}
}

////////////////////////////////int ToLowerStr(char *strsrc, char *strlow)/////////////////////////////////
int ToLowerStr(IN char *strsrc, OUT char *strlow) {
int  i, len, low;

len = strlen(strsrc);
for (i = 0; i < len; i++) {
	strlow[i]=(char)tolower((int)strsrc[i]);
}
strlow[i] = 0;
	return len;
}

//////////////////////////////////BOOL IsFileExist(char *FileName)/////////////////////////////////
BOOL IsFileExist(char *FileName) {
	char FindFile[256];
	WIN32_FIND_DATA FD;
	HANDLE hf = INVALID_HANDLE_VALUE;

	if (!FileName[0]) {
		return FALSE;
	}
	if (!strstr(FileName, "\\")) {
		GetCurrentDirectoryA(sizeof(FindFile), FindFile);
		sprintf(FindFile, "%s\\%s", FindFile, FileName);
	}
	else {
		strcpy(FindFile, FileName);
	}
	ZeroMemory(&FD, sizeof(FD));
	hf = FindFirstFile(FindFile, &FD);
	if (hf != INVALID_HANDLE_VALUE) {
		FindClose(hf);
		return TRUE;
	}
	return FALSE;
}

