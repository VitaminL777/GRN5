#include "nowarnings.h"
#include <windows.h>
#include <stdio.h>
#include "ini.h"
#include "funcs.h"
#include "structs.h"
#include "Grn5Dlg.h"
#include "ProcCmdFunc.h"
#include "LogMsg.h"
#include "directory.h"

#define WAIT_AND_AGAIN CloseHandle(hEv);Sleep(10000);goto WM;

HANDLE hThDlg = NULL;
DWORD ThDlgID;
HINSTANCE hInst;
BOOL IsCrypt = TRUE;
BOOL brez;
int rez;
extern HWND hDLG;
extern BOOL EndDlg;
ini Cini;
char tmp[1024];
char CFG_FILE[] = "GRN5.cfg";
char CFG_DIR[] = "\\GRN5";
char ConfFile[256];
char ConfDir[192];
char SubstPoint[192] = "B:";
char FragDir[256] = "C:\\A";
DWORD SilenceTimeThreshold = 1700; //длина одинаковых семплов в ms, считающейся началом паузы для вырезания
WORD SilenceAmplThreshold = 50; //порог амплитуды в дискретах, ниже которого на протяжении SilenceTimeThreshold и более - считается паузой
UINT MP3Bitrate = 24;
NETRES NetRes;
char PwdCrypt[192];
HANDLE hEv;
HANDLE hNot;
char EventName[] = "Global\\GRN5_G-Event";
char SubstCmd[192];
char WAVHead[64];
const char WAVfmt[] = "WAVEfmt ";
const char RIFF[] = "RIFF";
const char data[] = "data";
char oWAVPath[256];
char ocWAVPath[256];
char oWAVBasePath[256];
char WAVShort[128];
char MP3Short[128];
char Mp3Path[256];
char cMp3Path[256];
char Mp3Cmd[512];
UINT Mp3SecProgress;
HANDLE hWAV = INVALID_HANDLE_VALUE;
HANDLE hoWAV = INVALID_HANDLE_VALUE;
DWORD BytesPerSample;
UINT64 oWAVBytesWrite;
WAVBLOCK WavBlock = { NULL, NULL };

int InitParams();
unsigned long _stdcall ThDlg(void*);
int CreateProgress();
int CloseProgress();
UINT64 DirHandler(char *DirPath, BOOL RCall = 0);
UINT64 WAVHandler(char *WAVPath);
UINT64 WriteWavBlock(/*WAVBLOCK WavBlock*/);
inline BOOL IsSilencePair(UINT64* pW1, UINT64* pW2);
int NetCopyFile(char *FileName/*,NETRES NetRes*/);
//LPPROGRESS_ROUTINE LpprogressRoutine;
DWORD CopyProgress(
	LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred,
	LARGE_INTEGER StreamSize,
	LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber,
	DWORD dwCallbackReason,
	HANDLE hSourceFile,
	HANDLE hDestinationFile,
	LPVOID lpData
);


int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow){
WM:
	
hInst = hInstance;
hEv = CreateEvent(NULL, 0, 0, EventName);
if (GetLastError() == ERROR_ALREADY_EXISTS || hEv == NULL || !InitParams()) {
	return 0;
}

sprintf(SubstCmd, "%s %s %s", "subst.exe", SubstPoint, "/D");
CreateProcessAndWait(SubstCmd, SW_HIDE, 10000);
sprintf(SubstCmd, "%s %s %s", "subst.exe", SubstPoint, FragDir);
CreateProcessAndWait(SubstCmd, SW_HIDE, 10000);

ClearDirectory(SubstPoint);

hNot = FindFirstChangeNotification(
	FragDir,				// директория для просмотра
	TRUE,					// просматривать поддиректории
	FILE_NOTIFY_CHANGE_FILE_NAME); // отслеживать изм. имён файлов
if (hNot == INVALID_HANDLE_VALUE) {
	WAIT_AND_AGAIN
}
while (1) {
	/*//U
	DirHandler(FragDir);
	Sleep(2000);
	CloseProgress();
	//U*/
	WaitForSingleObject(hNot, INFINITE);
	if (FindNextChangeNotification(hNot) == FALSE) {
		WAIT_AND_AGAIN
	}
	DirHandler(FragDir);
	CloseProgress();
}


/*
rez=CreateProgress();
StartProgress(1, 20000, 10, 95);
StartProgress(2, 2000,50, 80);
StartProgress(3, 10000, 0, 77);
SetProgress(2, Val, 1000000);
CloseProgress();
*/


	
	return 0;

}


//////////////////////////////////int CreateProgress()///////////////////////////////////
int CreateProgress() {
int CreateAtt = 20;
	if (hThDlg != NULL || hDLG != NULL) {
		return 0;
	}
	hThDlg = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThDlg, NULL, 0, &ThDlgID);
	while (hDLG == NULL && CreateAtt--) {
		Sleep(50);
	}
	if (hDLG == NULL) {
		return 0;
	}
	return 1;
}

///////////////////////////////////int CloseProgress()////////////////////////////////////
int CloseProgress() {
int CloseAtt = 15;
	if (hThDlg == NULL && hDLG == NULL) {
		return 0;
	}
	SetProgress(0);
	while (hDLG != NULL && CloseAtt--) {
		Sleep(100);
	}
	if (hThDlg != NULL) {
		CloseHandle(hThDlg); hThDlg = NULL;
	}
	if (hDLG != NULL) {
		return 0;
	}
	return 1;
}

//////////////////////////int InitParams()////////////////////////////
int InitParams() {
int i, j, k;
UINT SYM;

	GetEnvironmentVariable("PROGRAMDATA", ConfDir, sizeof(ConfDir));
	strcat(ConfDir, CFG_DIR);
	sprintf(ConfFile, "%s\\%s", ConfDir, CFG_FILE);

	if (Cini.ReadIni(ConfFile, 0) < 1) {
		return 0;
	}
	Cini.OpenParam("SubstPoint", SubstPoint);
	Cini.OpenParam("FragDir", FragDir);
	Cini.OpenParam("SilenceTimeThreshold", tmp, (int*)&SilenceTimeThreshold);
	Cini.OpenParam("SilenceAmplThreshold", tmp, (int*)&i); SilenceAmplThreshold = (WORD)i;
	Cini.OpenParam("MP3Bitrate", tmp, (int*)&MP3Bitrate);
	Cini.OpenParam("Share", NetRes.Share);
	Cini.OpenParam("User", NetRes.User);
	strcpy(NetRes.MountPoint, "Y:");
	Cini.OpenParam("PwdCrypt", PwdCrypt);
	i = j = k = 0;
	while (PwdCrypt[i] && PwdCrypt[i] != '\r' && PwdCrypt[i] != '\n') {
		tmp[j] = PwdCrypt[i];
		if (j == 1) {
			tmp[j + 1] = 0;
			SYM = strtoul(tmp, 0, 16);
			NetRes.Password[k] = SYM; k++;
			j = -1;
		}
		i++; j++;
	}
	NetRes.Password[k] = 0;
	DeCPT(NetRes.Password, k);
	Cini.CloseIni();
	return 1;
}


/////////////////////////////////void DirHandler()///////////////////////////////////////
UINT64 DirHandler(char *DirPath, BOOL RCall) {
UINT64 DirSize = 0;
UINT64 DSumm;
WIN32_FIND_DATA FD;
HANDLE hf = INVALID_HANDLE_VALUE;
char FindEl[256];
char DirPathR[256];
char WAVPath[256];

		sprintf(FindEl, "%s\\%s", DirPath, "*");
		hf = FindFirstFile(FindEl, &FD);
		if (hf != INVALID_HANDLE_VALUE) {
			do {
				if (FD.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {//директория
					if (strcmp(FD.cFileName, ".") && strcmp(FD.cFileName, "..")) {
						sprintf(DirPathR, "%s\\%s", DirPath, FD.cFileName);
						DirSize += DirHandler(DirPathR, 1);
					}
				}
				else { //файл
					if (StriStr(FD.cFileName, ".wav")) {
						sprintf(WAVPath, "%s\\%s", DirPath, FD.cFileName);
						strcpy(WAVShort, FD.cFileName);
						WAVHandler(WAVPath);
					}
				}

			} while (FindNextFile(hf, &FD) != 0);
			FindClose(hf);
			return DirSize;
		}
		else {//hf == INVALID_HANDLE_VALUE
			if (!RCall) {
				return 0;
			}
			else {//Recursive Call
				return 0;
			}
		}

		return 0;
}

/////////////////////////////UINT64 WAVHandler(char *WAVPath)////////////////////////////
UINT64 WAVHandler(char *WAVPath) {
UINT64 ret = 0;
DWORD WavSize = 0;
DWORD PrevWavSize = 0;
#define WWSZ 6000; 
DWORD WaitWavSize = WWSZ; //время ожидания создания WAV-файла - 10 минут(если не меняется размер файла при ожидании)
DWORD WaitWavReady = 6000; //время ожидания завершения операций с WAV-файлом "Гранитом" - 600 сек.
DWORD BR, BW;
DWORD dr;
int Att = 100;
SIZE_T rM = 1024 * 1024;
char *iWAV = NULL;
PWAVHEAD pWavHead;
PWAVDATA pWavData;
DWORD WavHeadSz;
UINT64 SilenceByteThreshold, SQT;
UINT64 *pW, *pW2, *pWEnd;
UINT64 SilenceLength = 0;
BOOL IsSilenceArea = FALSE;
BOOL ISP;

while (Att--) {
	hWAV = CreateFile(WAVPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hWAV != INVALID_HANDLE_VALUE) {
		break;
	}
	Sleep(100);
}
if (hWAV == INVALID_HANDLE_VALUE) {
	return 0;
}
Att = 100;
while (Att--) {
	WavSize=GetFileSize(hWAV, NULL);
	if (WavSize >= 200) {
		break;
	}
	Sleep(100);
}
ReadFile(hWAV, WAVHead, 20, &BR, NULL);
pWavHead = (PWAVHEAD)WAVHead;
if (memcmp(pWavHead->RIFF, RIFF, 4) || memcmp(pWavHead->WAVEfmt, WAVfmt, 8) || pWavHead->FmtSz < 0x10) {//нет нормального заголовка
	goto WH_EX;
}
WavHeadSz = 20;
ReadFile(hWAV, &pWavHead->wFormatTag, pWavHead->FmtSz+8, &BR, NULL);
pWavData =(PWAVDATA)((char*)&pWavHead->wFormatTag + pWavHead->FmtSz);
if (memcmp(pWavData->DATA, data, 4)) {//нет нормального поля "data"
	goto WH_EX;
}
WavHeadSz += pWavHead->FmtSz + 8;
CreateProgress();
while(WaitWavSize){//время ожидания создания WAV-файла - 10 минут(если не меняется размер файла при ожидании)
	WavSize = GetFileSize(hWAV, NULL);
	if (PrevWavSize == WavSize) {
		WaitWavSize--;
	}
	else {//PrevWavSize != WavSize
		WaitWavSize = WWSZ;
	}
	PrevWavSize = WavSize;
	if (WavSize >= pWavData->DataSz + WavHeadSz) {
		SetProgress(1, 100);
		break;
	}
	SetProgress(1, WavSize, pWavData->DataSz);
	Sleep(100);
}
if (!WaitWavSize) {//WAV-файл не был корректно создан
	goto WH_EX;
}
CloseHandle(hWAV);//закрываем дескриптор WAV, чтобы открыть в READ-ONLY - как признак завершения записи WAV "Гранитом"
while (WaitWavReady) {
	hWAV = CreateFile(WAVPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hWAV != INVALID_HANDLE_VALUE) {
		break;
	}
	Sleep(100); WaitWavReady--;
}
if (!WaitWavReady) {//создание WAV-файла не было корректно завершено
	goto WH_EX;
}
GetBaseName(WAVPath, oWAVBasePath, ".wav");
sprintf(oWAVPath, "%s_%s", oWAVBasePath, ".wav");
sprintf(ocWAVPath, "\"%s_%s\"", oWAVBasePath, ".wav");
sprintf(Mp3Path, "%s%s", oWAVBasePath, ".mp3");
sprintf(cMp3Path, "\"%s%s\"", oWAVBasePath, ".mp3");
GetBaseName(WAVShort, MP3Short, ".wav");
strcat(MP3Short, ".mp3");
hoWAV = CreateFile(oWAVPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
if (hoWAV == INVALID_HANDLE_VALUE) {
	goto WH_EX;
}
ReadFile(hWAV, WAVHead, WavHeadSz, &BR, NULL);
WriteFile(hoWAV, WAVHead, WavHeadSz, &BW, NULL);
oWAVBytesWrite = BW;
iWAV = (char*)VirtualAlloc(0, rM, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
BytesPerSample = pWavHead->nBlockAlign;
SilenceByteThreshold = pWavHead->nSamplesPerSec*BytesPerSample*SilenceTimeThreshold / 1000;
SQT = SilenceByteThreshold / 8;
IsSilenceArea = FALSE;
WavBlock.WStart = WavBlock.WEnd = 0;
int RF = 0;
DWORD ReadSize = 0;
while (ReadFile(hWAV, iWAV, rM, &BR, NULL)) {
	RF++;
	ReadSize += BR/100;
	SetProgress(2, ReadSize, WavSize/75);
	if (BR < 8) {
		SetProgress(2, 75, 100);
		break;
	}
	pWEnd = (UINT64*)((char*)(iWAV + BR - 8));
	if (!IsSilenceArea) {
		WavBlock.WStart = iWAV;
		SilenceLength = 0;
	}
	pW = (UINT64*)iWAV; pW2 = pW + 1;
	while (1) {
		ISP = IsSilencePair(pW, pW2);
		if (ISP){
			while (ISP) {
				if (!IsSilenceArea) {
					SilenceLength++;
					if (SilenceLength >= SQT) {//кол-во пустоты набралось до точки начала отсечки пустоты
						WavBlock.WEnd = (char*)pW2;
						WriteWavBlock(/*WavBlock*/);
						IsSilenceArea = TRUE;
					}
				}
				pW++; pW2++;
				if (pW2 > pWEnd) {
					if (!IsSilenceArea) {
						WavBlock.WEnd = (char*)pW2 - 1;
						WriteWavBlock(/*WavBlock*/);
					}
					goto WH_W1;
				}
				ISP = IsSilencePair(pW, pW2);
			}
			
		}
		SilenceLength = 0;
		IsSilenceArea = FALSE;
		if (!WavBlock.WStart) {
			WavBlock.WStart = (char*)pW;
		}
		pW++; pW2++;
		if (pW2 > pWEnd) {
			WavBlock.WEnd = (char*)pW2 - 1;
			WriteWavBlock(/*WavBlock*/);
			break;
		}
	}
WH_W1:
	WavBlock.WStart = WavBlock.WEnd = 0;
}
//oWAVBytesWrite
//pWavHead->RiffSz
//WavSize >= pWavData->DataSz + WavHeadSz)
pWavData->DataSz = oWAVBytesWrite - WavHeadSz;
pWavHead->RiffSz = pWavData->DataSz + 0x20;
SetFilePointer(hoWAV, 0, 0, FILE_BEGIN);
WriteFile(hoWAV, WAVHead, WavHeadSz, &BW, NULL);
CloseHandle(hoWAV); hoWAV = INVALID_HANDLE_VALUE;
Mp3SecProgress = pWavData->DataSz / (1024 * 1024 * 14) + 1;
sprintf(Mp3Cmd, "lame.exe -b %d %s %s", MP3Bitrate, ocWAVPath, cMp3Path);
DeleteFile(Mp3Path);
	StartProgress(2, Mp3SecProgress * 1000, 75, 100);
CreateProcessAndWait(Mp3Cmd, SW_HIDE, INFINITE);
DeleteFile(oWAVPath);
SetProgress(2, 100, 100);
NetCopyFile(Mp3Path);
Sleep(1000);
DeleteFile(Mp3Path);
ret = 1;

WH_EX:
if (iWAV) {
	VirtualFree(iWAV, 0, MEM_RELEASE);
}
if (hWAV != INVALID_HANDLE_VALUE) {
	CloseHandle(hWAV);
}
if (hoWAV != INVALID_HANDLE_VALUE) {
	CloseHandle(hoWAV);
}
if (ret) {
	DeleteFileA(WAVPath);
}
	return ret;
}

//////////////////////UINT64 WriteWavBlock(/*WAVBLOCK WavBlock*/)////////////////////////
UINT64 WriteWavBlock(/*WAVBLOCK WavBlock*/) {
DWORD BW;
DWORD DWW;

	if (!WavBlock.WEnd || !WavBlock.WStart || hoWAV == INVALID_HANDLE_VALUE || hoWAV == NULL) {
		WavBlock.WStart = WavBlock.WEnd = 0;
		return 0;
	}
	DWW = ((WavBlock.WEnd - WavBlock.WStart + 1) / BytesPerSample) * BytesPerSample;
	WriteFile(hoWAV, WavBlock.WStart, DWW, &BW, NULL);
	WavBlock.WStart = WavBlock.WEnd = 0;
	oWAVBytesWrite += BW;
	return BW;
}

/////////////////inline BOOL IsSilencePair(UINT64* pW1, UINT64* pW2)/////////////////////
inline BOOL IsSilencePair(UINT64* pW1, UINT64* pW2) {//WORD SilenceAmplThreshold
int i;
short *ps1, *ps2;
short *p2s1, *p2s2;
short diff, diff2;
	if (BytesPerSample == 4) {
		ps1 = (short*)pW1; p2s1 = (short*)pW1 + 1;
		for (i = 2; i <= 6; i+=2) {
			ps2 = ps1 + i; diff = *ps2 - *ps1;
			p2s2 = p2s1 + i; diff2 = *p2s2 - *p2s1;
			diff = diff >= 0 ? diff : ~diff + 1;
			diff2 = diff2 >= 0 ? diff2 : ~diff2 + 1;
			if (diff > SilenceAmplThreshold || diff2 > SilenceAmplThreshold) {
				return FALSE;
			}
		}
		return TRUE;
	}
	else {//as BytesPerSample == 2
		ps1 = (short*)pW1;
		for (i = 1; i <= 7; i++) {
			ps2 = ps1 + i; diff = *ps2 - *ps1;
			diff = diff >= 0 ? diff : ~diff + 1;
			if (diff > SilenceAmplThreshold) {
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////int NetCopyFile(char *FileName)///////////////////////////////
int NetCopyFile(char *FileName) {/*NETRES NetRes, char *MP3Short*/
	NETRESOURCE NETRes;
	DWORD DWREZ;
	int AttConn = 2;
	char NetName[256]; //сетевое имя
	char ShortName[128];

	ZeroMemory(&NETRes, (SIZE_T)sizeof NETRESOURCE);
	NETRes.dwType = RESOURCETYPE_DISK;
	if (NetRes.MountPoint[0]) {
		NETRes.lpLocalName = NetRes.MountPoint;
	}
	else {
		return 0;
	}
	NETRes.lpRemoteName = NetRes.Share;
WAC:
	DWREZ = WNetAddConnection2(&NETRes, NetRes.Password, NetRes.User, CONNECT_TEMPORARY);
	if (DWREZ != NO_ERROR) {
		if (AttConn) {
			WNetCancelConnection2(NETRes.lpLocalName, NULL, TRUE);
			Sleep(1000); AttConn--;
			goto WAC;
		}
		else {
			return 0;
		}
	}
	sprintf(NetName, "%s\\%s", NETRes.lpLocalName, MP3Short);
	CopyFileExA(FileName, NetName, (LPPROGRESS_ROUTINE)CopyProgress, NULL, NULL, NULL);

	DWREZ = WNetCancelConnection2(NETRes.lpLocalName, NULL, TRUE);
	return 1;
}

/////////////////////////////////DWORD CopyProgress//////////////////////////////////////////
DWORD CopyProgress(
	LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred,
	LARGE_INTEGER StreamSize,
	LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber,
	DWORD dwCallbackReason,
	HANDLE hSourceFile,
	HANDLE hDestinationFile,
	LPVOID lpData
)
{
	DWORD Val, MaxVal;
	MaxVal = (DWORD)TotalFileSize.QuadPart/1024;
	Val = (DWORD)TotalBytesTransferred.QuadPart/1024;
	SetProgress(3, Val, MaxVal);

	return PROGRESS_CONTINUE;
}