int GetCorrectCmdLine(LPSTR Cmd, LPSTR CmdLine);
int GetShortModuleName(LPSTR Full, LPSTR Short);
int GetFullModuleNameAndParams(LPSTR Full, LPSTR FullModuleName, LPSTR Params, LPSTR SelfParams);
int ClearIFEO(LPSTR Cmd);
int RestoreIFEO();
int GetFlagAndCorrect(LPSTR Cmd, int* POperFlag);
int WaitProcess(char* ProcStr, DWORD TimeOut);
int IsProcessExist(char* ProcStr, HANDLE *hProc=NULL);
int KillProcess(char* ProcStr);
int CloseProcess(char* ProcStr, DWORD TimeWait);//TimeWait in seconds
int StartAndWaitProcess(HANDLE hToken,char* Desk,char* ProcStr, DWORD TimeOut, char* PRN, int FlagIFEO);
int StartAndWaitProcess0(HANDLE hToken,char* ProcStr, DWORD TimeOut, int HS);
int CreateProcessAndWait(char* ProcStr, int HS, DWORD TimeOut = 0, DWORD *PDWID = 0);
int CreateProcAndWaitExitCode(char* ProcStr, int HS, DWORD TimeOut = 0, DWORD *PEC = 0);
int WaitChildProcesses(DWORD PID);
int SetProcDirectory(LPSTR Full);
DWORD GetProcessID(char* ProcStr);
BOOL CALLBACK CloseAppEnumWindows(HWND hWnd, LPARAM lParam);


#define MSTSC_FIND

#ifdef MSTSC_FIND
int CreateSnapshot1();
int CreateSnapshot2();
HANDLE FindMstscEmbedding();
#endif