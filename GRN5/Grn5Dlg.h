#pragma once
int SetProgress(int NumProgr, DWORD Value = 0, DWORD MaxValue = 100);
int StartProgress(int NumProgr, DWORD ms, DWORD PercentStart, DWORD PercentEnd);
