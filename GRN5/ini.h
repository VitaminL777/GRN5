// ini.h: interface for the ini class.
//
//////////////////////////////////////////////////////////////////////
/*
ini-����� ��� ������ � ������� ini-����
��� ������� (����� OpenParam, OpenLastString, OpenLastMultiString � GetStringBetween) ���������� 1 
� ������ ������ � 0 � ������ �������. OpenParam � ������ ������ ���������� ���������� ���� 
� �������� ��������� ��������� (0-���� ��������� �������� �� ���������) � -1 � ������ �������.
OpenLastString � ������ ������ ���������� ���-�� ���� � ��������� ������, 0 - � ������, ����
��������� ������ ������ ������������ ��� ����� ������ ������ � -1 � ������ ���� �� �������
����� ������ � ��������� ��������� ������.
OpenLastMultiString ���������� ����� ������ �� ������� �������, ������� ���� ������� ������, � ��������
������� ������ � -1 � ������ ����� �������

 ReadIni(char* NameIni, BOOL ReadOnly=0) � ReadOnly=0 ��������� ���� � ��������� ������ � �������� ��� 
���������� � ������, ��� ���������� �������� � ini ������������ ������ � ���� �������,(���� ���� ������� ���
���� ��������) � �� � ����� ini-������. ����� ������ SaveAndCloseIni() ���������� ������ ������ � ���� � 
��� ���������. ���� ReadOnly=1, �� ���� ����������� � ������� ������ �� ������, ����������� ��� ����� � ������
� ��� ���� ��� �� �����������, ���������� �������� ���� SaveAndCloseIni � Upadte* � ���� ������ ����������.
 
 SetSection(char* SectName) ������������� ������� ��������� ��� �������
OpenParam, UpdateParam, AddParam, OpenLastString(� NextSeek=0) ������ ������
� ������ SectName. ��� ������ ���������� ��� �������� "[" "]". SetSection(0)
������ ������� ��������� ����� �����.

 OpenParam(char* paramname,char* parambuf,int* value=0) ���� � ������ ini  - ��������
� ������ paramname � ��������� �������� ����� ��������� ������� � �����
parambuf(���� ��������� ��������� ��������). �������� �� �������� �����
���� ������� ��������� ������,"=" ��� �� ����������� � ����� ���������.
���� ������� ��������� �������� value(��������� �� ���������� int),��
������� ������� � ��� ���������� �������� 'atoi'�� parambuf

 OpenParam_n - ����, ��� � OpenParam �� ����������� ����, ��� ����������
�� ������ ������ ��������� �������� � ������ ������, � n-� ��������, �����������
�� ������ �������� ������ (������� �����, ��� ����� ���������� ���������).

 OpenLastString - ���������� ������, ������������ �� �������� paramname,(���������� ��� ����� ��������),
������ �������� ����� � ����� �������� ������, ������� �������� NextSeek ���������� ����� � ������ �����
�������� ������ � ����������� ������, � ��������� �������� NextSeek - ���������� ����� �����, ���� �� ���
��� �����, � ���� �� ��� �����, �� �������� ��� �� ��� � NextSeek=0. ���� IsParam=1, �� ������ ������������
��� �������� �� �������� � OpenParam, �.� ��� �������� � ������ "=" � ������ ������, ���� IsParam=0, - ��
������������ ������� ��� ����. ����� �� ����� ���������� ��������� ������ ������ BuffSize.
��� ���� ������� ��� �������������� Update-�������, ������� �������� ������ �� ������.
 
  OpenLastMultiString - ������ �� OpenLastString, �� ���� ������ ���������� ������ � �������� ������� ������
������������ �� ������ �� ����� �������� ����� �� ������� ������� strarr � �������� �������� sizearr
� ���������� ����� �������� ������ �� ������� ������� � ����� ������ ��������� ������ � parambuf.
 
  GetStringBetween - ���������� ������, ����������� ����� �������� Left � Right, Num - ����� ����� ���������
������ Left � ������� ������. Left � Right ����� ���� ������� 0 ��� ��������� �� ������ ������, ��� ��������
����� � ������ ������(Left=0) ��� ����� �� ����� ������ (Right=0). ���� Left=0, � Right-��������� ������, ��
� ���� ������ �������� Num ��������� �� ����� ��������� ������ Right. ��������� ��������� ���������� OpenLastString

 UpdateParam(char* parambuf) � ����� ���������� ��������� �������� ���������
�� parambuf,��� �������� ��������� ��� ���������� OpenParam.
 UpdateParam(char* parambuf, char* paramname=0) � ����� �����������
�������������� ��������� � ��������� �������� ��������� � ������
paramname. 
������� AddParam(char* paramname, char* separate="=", char* parambuf=0)
���������
*/

#if !defined(AFX_INI_H__6B59B70C_70B2_4BE5_8AA7_6A32343EAE68__INCLUDED_)
#define AFX_INI_H__6B59B70C_70B2_4BE5_8AA7_6A32343EAE68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define INI_CRYPT 1
#define INI_DECRYPT 2
#define INI_DECRYPT_IF_NEEDED 3

class ini  
{
public:
	ini();
	virtual ~ini();
	HANDLE hini;	
	LPVOID Pini;
	DWORD IniSize, IniFileSize, SectSize, SectSizePrev;
	int SeekSize;
	int SeekSizeM;
	char* pFind;
	char* pFindPrev;
	WORD Wnstr;
	char* PParam;
	char* Psect;
	char* PsectPrev;
	char* PRootSect;
	char* PSeek;
	char* EndIni;
	char CPT_ENC[4];
	char tmpstr[384];
	DWORD LenParam;
	DWORD SzAlloc;
	int ReadIni(char* NameIni, BOOL ReadOnly=false, int Cpt = 0);
	int ReadRegIni(char* NameKey, char* NameValue, int Cpt = 0);
	int SetSection(char* SectName);
	int SetPrevSection();
	int OpenParam(char* paramname, char* parambuf, int* value=0, UINT64* UI64val=0);
	int OpenParam_n(char* paramname, char* parambuf, int nparam, int* value=0);
	int OpenLastString(char* paramname, char* parambuf, int BuffSize, int NextSeek, BOOL IsParam);
	int OpenLastMultiString(char** strarr, int sizearr, char* parambuf, int BuffSize, int NextSeek, BOOL IsParam);
	int GetStringBetween(char* Left, char* Right, int Num, char* instr, char* outbuf, int BuffSize, BOOL IsParam);
	int UpdateParam(char* parambuf, char* paramname=0);
	int AddParam(char* paramname, char* separate="=", char* parambuf=0);
	int FindFirstStr(char* parambuf);
	int FindNextStr(char* parambuf);
	int FindFirstAnyStr(char* parambuf);
	int FindNextAnyStr(char* parambuf);
	int NormalizationStr(char* str, BOOL IsStrDir=false);
	int CloseIni();
	int SaveAndCloseIni();
	int Save2Reg(char* NameKey, char* NameValue, BOOL IsCpt = false);
	int Save2File(char* FileName, BOOL IsCpt = false);
	int CreateIni(char* NameIni);
	int CX;
	BOOL OpenFlag;
	BOOL Encrypt;
	BOOL OpenIni;
	BOOL RegIni;
	BOOL IsIniCrypt;
	SYSTEM_INFO  sysi;
        	


};

#endif // !defined(AFX_INI_H__6B59B70C_70B2_4BE5_8AA7_6A32343EAE68__INCLUDED_)
