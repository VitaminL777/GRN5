#include "nowarnings.h"
#include "resource.h"
#include <windows.h>
#include "structs.h"
#include "framework.h"
#include "Grn5Dlg.h"

BOOL EndDlg = FALSE;
DWORD CRIT_STEP1, CRIT_STEP2;
UINT I_STEP1 = 0, I_STEP2 = 0;
extern HINSTANCE hInst; // текущий экземпл€р
WINDOWINFO WI;
int XSCR, YSCR, XPos, YPos;
static HICON hIconG = NULL;
static HICON hIconY = NULL;
static HICON hIconR = NULL;
BOOL IsSetRange = FALSE;
WPARAM WStep = 0;
WPARAM Step1, Step2, Step3;
WPARAM PrevStep1, PrevStep2, PrevStep3;
TIME_PROGRESS TP1, TP2, TP3;
PTIME_PROGRESS pTP[] = { &TP1, &TP2, &TP3 };
LRESULT lrez;
HWND hPr1, hPr2, hPr3;
HWND hStop, hStart, hStatus;
HWND hDLG = NULL;
BYTE C_R = 100;
BYTE C_G = 100;
BYTE C_B = 100;
int TimerID = 3007;
char TMP[128];
UINT_PTR UTIMER;
WORD PANEL_STATUS = 0;
extern WORD SS_STATUS;
extern WORD STATUS_FLAGS;
WORD COLOR_STATE = 0;
BOOL StatusStartConnect = false;

INT_PTR CALLBACK    Grn5Wnd(HWND, UINT, WPARAM, LPARAM);

unsigned long _stdcall ThDlg(void*) {

TP1.DEndPos = TP3.DEndPos = TP3.DEndPos = 0;
DialogBox(hInst, MAKEINTRESOURCE(IDD_REMAG), NULL, Grn5Wnd);
	
	return 0;

}

//========================================================================================
// ќбработчик сообщений дл€ окна "GRN5".
INT_PTR CALLBACK Grn5Wnd(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
		
	switch (message)
	{
	case WM_INITDIALOG:
		hIconY = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIconY);
		UTIMER=SetTimer(hDlg, TimerID, 100, NULL);
		hPr1 = GetDlgItem(hDlg, IDC_PROGRESS1);
		hPr2 = GetDlgItem(hDlg, IDC_PROGRESS2);
		hPr3 = GetDlgItem(hDlg, IDC_PROGRESS3);
		lrez = SendMessageA(hPr1, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
		lrez = SendMessage(hPr1, PBM_SETSTEP, (WPARAM)1, 0);
		lrez = SendMessageA(hPr2, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
		lrez = SendMessage(hPr2, PBM_SETSTEP, (WPARAM)1, 0);
		lrez = SendMessageA(hPr3, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
		lrez = SendMessage(hPr3, PBM_SETSTEP, (WPARAM)1, 0);
		SendMessage(hPr1, PBM_SETBARCOLOR, 0, RGB(0, 0, 0));
		SendMessage(hPr2, PBM_SETBARCOLOR, 0, RGB(255, 255, 0));
		SendMessage(hPr3, PBM_SETBARCOLOR, 0, RGB(0, 255, 0));
		//------------SetWindowPosition---------------------------------------
		WI.cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(hDlg, &WI);
		XSCR = GetSystemMetrics(SM_CXSCREEN); XPos = XSCR / 2 - (WI.rcWindow.right / 2);
		YSCR = GetSystemMetrics(SM_CYSCREEN); YPos = YSCR / 2 - (WI.rcWindow.bottom / 2);
		SetWindowPos(hDlg, 0, XPos, YPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		//------------SetWindowPosition---------------------------------------
		Step1 = Step2 = Step3 = PrevStep1 = PrevStep2 = PrevStep3 = 0;
		ZeroMemory(&TP1, sizeof(TIME_PROGRESS));
		ZeroMemory(&TP2, sizeof(TIME_PROGRESS));
		ZeroMemory(&TP3, sizeof(TIME_PROGRESS));
		hDLG = hDlg; EndDlg = FALSE;
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDABORT)
		{
			EndDialog(hDlg, LOWORD(wParam));
			hDLG = NULL; EndDlg = TRUE;
			ExitThread(0);
			return (INT_PTR)TRUE;
		}
		break;
	
	case WM_TIMER:
		if (TP1.DEndPos) {
			TP1.DCurPos += TP1.DStep;
			if (TP1.DCurPos > TP1.DEndPos) {
				ZeroMemory(&TP1, sizeof(TP1));
				goto WTS1;
			}
			Step1 = (WPARAM)TP1.DCurPos;
		}
		if (TP2.DEndPos) {
			TP2.DCurPos += TP2.DStep;
			if (TP2.DCurPos > TP2.DEndPos) {
				ZeroMemory(&TP2, sizeof(TP2));
				goto WTS1;
			}
			Step2 = (WPARAM)TP2.DCurPos;
		}
		if (TP3.DEndPos) {
			TP3.DCurPos += TP3.DStep;
			if (TP3.DCurPos > TP3.DEndPos) {
				ZeroMemory(&TP3, sizeof(TP3));
				goto WTS1;
			}
			Step3 = (WPARAM)TP3.DCurPos;
		}
WTS1:
		if (Step1 != PrevStep1) {
			SendMessage(hPr1, PBM_SETPOS, Step1, 0);
			PrevStep1 = Step1;
		}
		if (Step2 != PrevStep2) {
			SendMessage(hPr2, PBM_SETPOS, Step2, 0);
			PrevStep2 = Step2;
		}
		if (Step3 != PrevStep3) {
			SendMessage(hPr3, PBM_SETPOS, Step3, 0);
			PrevStep3 = Step3;
		}
		if (EndDlg) {
			EndDialog(hDlg, LOWORD(wParam));
			hDLG = NULL;
			ExitThread(0);
		}
		return (INT_PTR)TRUE;
		break;

	}
	return (INT_PTR)FALSE;
}


///////////////////int SetProgress(int NumProgr, DWORD Value, DWORD MaxValue)//////////////////////
int SetProgress(int NumProgr, DWORD Value, DWORD MaxValue){
int ret = 1;
DWORD Step;
UINT64 UStep, UValue, UMaxValue;

	if (hDLG == NULL) {
		return 0;
	}
	UValue = (UINT64)Value; UMaxValue = (UINT64)MaxValue;
	UStep = (UValue * 100) / UMaxValue;
	switch (NumProgr)
	{
	case 0:
		Step1 = Step2 = Step3 = 0;
		EndDlg = TRUE;
		break;
	case 1:
		if (MaxValue != 100) {
			Step1 = UStep;
		}
		else {
			Step1 = Value;
		}
		break;
	case 2:
		if (MaxValue != 100) {
			Step2 = UStep;
		}
		else {
			Step2 = Value;
		}
		break;
	case 3:
		if (MaxValue != 100) {
			Step3 = UStep;
		}
		else {
			Step3 = Value;
		}
		break;
	
	default:
		ret = 0;
		break;
	}
	
	return ret;
}

//////////int StartProgress(int NumProgr, DWORD ms, DWORD PercentStart, DWORD PercentEnd)//////////
int StartProgress(int NumProgr, DWORD ms, DWORD PercentStart, DWORD PercentEnd) {
PTIME_PROGRESS ptp;

if (hDLG == NULL || NumProgr < 1 || NumProgr > 3 || !ms || !PercentEnd){
	return 0;
}
ptp = pTP[NumProgr - 1];
if (NumProgr == 1) {
	SendMessage(hPr1, PBM_SETPOS, (WPARAM)PercentStart, 0);
	Step1 = PrevStep1 = (WPARAM)PercentStart;
	ptp->DCurPos = (DOUBLE)Step1;
}
else if (NumProgr == 2) {
	SendMessage(hPr2, PBM_SETPOS, (WPARAM)PercentStart, 0);
	Step2 = PrevStep2 = (WPARAM)PercentStart;
	ptp->DCurPos = (DOUBLE)Step2;
}
else {
	SendMessage(hPr3, PBM_SETPOS, (WPARAM)PercentStart, 0);
	Step3 = PrevStep3 = (WPARAM)PercentStart;
	ptp->DCurPos = (DOUBLE)Step3;
}
ptp->DStep = (DOUBLE)(PercentEnd-PercentStart)*100 / (DOUBLE)ms;
ptp->DEndPos = (DOUBLE)PercentEnd;
	
return 1;
}
