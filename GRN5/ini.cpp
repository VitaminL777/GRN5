// ini.cpp: implementation of the ini class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <stdio.h>
#include "ini.h"
#include "funcs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ini::ini()
{
hini=INVALID_HANDLE_VALUE;
SectSize=0;
SectSizePrev = 0;
PsectPrev = 0;
SeekSize=0;
pFind = 0;
pFindPrev = 0;
PSeek=0;
Pini=0;
OpenFlag=0;
Wnstr=0x0A0D;
Encrypt=0;
GetSystemInfo(&sysi);
OpenIni=0;
RegIni=0;
CPT_ENC[0] = 0x17;
CPT_ENC[1] = 0xD2;
CPT_ENC[2] = 0x91;
CPT_ENC[3] = 0;
IsIniCrypt = false;
}

ini::~ini()
{

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ini::ReadRegIni(char* NameKey, char* NameValue, int Cpt) {
DWORD PDWORD=0;
DWORD DWTYPE;
unsigned char tmp[16];
HKEY hkey = 0;
	if (OpenIni || (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, NameKey, &hkey)) || hkey==0) {
		return 0;
	}
	Pini = 0;
	RegQueryValueEx(hkey, NameValue, NULL, &DWTYPE, tmp, &PDWORD);
	if (PDWORD == 0) {
		goto CLEX;
	}
	IniFileSize = IniSize = SectSize = PDWORD;
	SzAlloc = (IniFileSize / sysi.dwPageSize + 1)*sysi.dwPageSize;
	Pini = (LPSTR)VirtualAlloc(0, SzAlloc,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (Pini == 0) {
		RegCloseKey(hkey);
		SectSize = 0;
		return 0;
	}
	if (ERROR_SUCCESS != RegQueryValueEx(hkey, NameValue, NULL, &DWTYPE, (unsigned char*)Pini, &PDWORD)) {
		goto VF;
	}
	if (Cpt == INI_CRYPT) {
		CPT((char*)Pini, IniSize);
	}
	if (Cpt == INI_DECRYPT) {
		DeCPT((char*)Pini, IniSize);
	}
	OpenIni = 1;
	RegIni = 1;
	Psect = (char*)Pini;
	EndIni = (char*)Pini + IniSize;
	RegCloseKey(hkey);
	return 1;

VF:
	if (Pini != 0) {
		VirtualFree(Pini, 0, MEM_RELEASE);
		Pini = 0;
	}
CLEX:
	RegCloseKey(hkey);
	SectSize = 0;
	return 0;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ini::ReadIni(char* NameIni, BOOL ReadOnly, int Cpt)
{
DWORD PDWORD;
PDWORD=0;
Encrypt=0;
OpenIni=0;
if(hini!=INVALID_HANDLE_VALUE){
	CloseHandle(hini);
	hini=INVALID_HANDLE_VALUE;
}
if(Pini!=0){
VirtualFree(Pini,0,MEM_RELEASE);
	Pini=0;
}
SectSize=0;
if(!ReadOnly){
hini=CreateFile( (LPCTSTR)NameIni,GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,0,
				OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,0);
}
else{//ReadOnly
hini=CreateFile( (LPCTSTR)NameIni,GENERIC_READ,
				FILE_SHARE_READ|FILE_SHARE_WRITE,0,
				OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,0);
}

if (INVALID_HANDLE_VALUE==hini)
return 0;

DWORD SFP=SetFilePointer(hini,0,0,FILE_BEGIN);
IniFileSize=IniSize=SectSize=SectSizePrev=GetFileSize(hini,0);

SzAlloc=(IniFileSize/sysi.dwPageSize+1)*sysi.dwPageSize;
Pini=(LPSTR) VirtualAlloc(0,SzAlloc,
						MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
if(Pini==0){
	CloseHandle(hini);
	hini=INVALID_HANDLE_VALUE;
	SectSize=0;
	return 0;
}

ReadFile(hini,Pini,IniSize,&PDWORD,0);

if (Cpt == INI_CRYPT) {
	CPT((char*)Pini, IniSize);
}
if (Cpt == INI_DECRYPT) {
	DeCPT((char*)Pini, IniSize);
}
if (Cpt == INI_DECRYPT_IF_NEEDED) {
	if (strncmp(CPT_ENC, (char*)Pini, 3) == 0) {
		DeCPT((char*)Pini, IniSize);
		IsIniCrypt = true;
	}
}

if(PDWORD==0){
	CloseHandle(hini);
	hini=INVALID_HANDLE_VALUE;
	BOOL rez=VirtualFree(Pini,0,MEM_RELEASE);
	Pini=0;
	SectSize=0;
	return 0;
}
OpenIni=1;
Psect = PsectPrev = PRootSect = (char*)Pini;
EndIni=(char*)Pini+IniSize;
DWORD* DPini=(DWORD*)Pini;
if(*DPini==0xF0F2F0F2)
Encrypt=1;
if(ReadOnly){
CloseHandle(hini);
hini=INVALID_HANDLE_VALUE;
}

return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ini::OpenParam(char* paramname, char* parambuf, int* value, UINT64* UI64val)
{
DWORD i,j,l;
int k;
char* pcur1;
char* pcur2;

if(!OpenIni){return -1;}

if(!SectSize||Encrypt)
	return -1;

i=j=k=0;
OpenFlag=1;
l=strlen(paramname);
while(i<SectSize){
wh1:		
		while(j<l){
			pcur1=paramname+j;
			pcur2=(char*)Psect+i+j;
			if(*pcur1!=*pcur2){
				j=0;
			    break;
			}
			j++;
		}
		
		if(*pcur2==0x0D || *pcur2==0x0A){
			i++;
			goto wh1;
		}
		
		pcur2++;
		if(j && (*pcur2==0x0D||*pcur2==0x20||*pcur2==0x3D))
		break;
		if(j)
		j=0;
	while(i<SectSize){
		i++;
		if(i>=SectSize)
		break;
		pcur2=(char*)Psect+i;
		if(*pcur2==0x0D && *(pcur2+1)==0x0A){
		i+=2;
		break;
		}
	}
		

}

if(j){
 i+=j;
 
 while(i<SectSize){
	pcur2=(char*)Psect+i;
	if(*pcur2!=0x20 && *pcur2!=0x3D)
		break;
		i++;
 }

 PParam=pcur2;
 if(i>=SectSize || *pcur2==0x0D){
 LenParam=0;
 return 0;
 }
 
 
 while(i<SectSize && *pcur2!=0x0D){
	*(parambuf+k)=*pcur2;
	i++;
	k++;
	pcur2=(char*)Psect+i;
			
 }
 *(parambuf+k)=0;
LenParam=k;
while (k>=0) {
	if ((*(parambuf + k) != 0) && (*(parambuf + k) != 0x20) && (*(parambuf + k) != 0x09)) {
		k++; break;
	}
	*(parambuf + k) = 0; k--;
}
if (k < 0) {
	return 0;
}
if(value){
*value=atoi(parambuf);
}
if (UI64val) {
*UI64val = strtoull(parambuf, NULL, 10);
}
 return k;

 
}
	OpenFlag=0;
	return -1;
}

/////////////////////////////////////////////////////////////
int ini::OpenParam_n(char* paramname, char* parambuf, int nparam, int* value)
{
DWORD i,j,k,l;
char* pcur1;
char* pcur2;

if(!OpenIni){return -1;}

if(!SectSize||Encrypt)
	return -1;

i=j=k=0;
OpenFlag=1;
l=strlen(paramname);
while(i<SectSize){
wh1:		
		while(j<l){
			pcur1=paramname+j;
			pcur2=(char*)Psect+i+j;
			if(*pcur1!=*pcur2){
				j=0;
			    break;
			}
			j++;
		}
		
		if(*pcur2==0x0D || *pcur2==0x0A){
			i++;
			goto wh1;
		}
		
		pcur2++;
		if(j && (*pcur2==0x0D||*pcur2==0x20||*pcur2==0x3D)){
			if(nparam==1){
				break;
			}
			nparam--;
			i+=j;
			j=0;
			goto wh1;
		}

		if(j)
		j=0;
	while(i<SectSize){
		i++;
		if(i>=SectSize)
		break;
		pcur2=(char*)Psect+i;
		if(*pcur2==0x0D && *(pcur2+1)==0x0A){
		i+=2;
		break;
		}
	}
		

}

if(j){
 i+=j;
 
 while(i<SectSize){
	pcur2=(char*)Psect+i;
	if(*pcur2!=0x20 && *pcur2!=0x3D)
		break;
		i++;
 }

 PParam=pcur2;
 if(i>=SectSize || *pcur2==0x0D){
 LenParam=0;
 return 0;
 }
 
 
 while(i<SectSize && *pcur2!=0x0D){
	*(parambuf+k)=*pcur2;
	i++;
	k++;
	pcur2=(char*)Psect+i;
			
 }
 *(parambuf+k)=0;
LenParam=k;
while (k >= 0) {
	if ((*(parambuf + k) != 0) && (*(parambuf + k) != 0x20) && (*(parambuf + k) != 0x09)) {
		k++; break;
	}
	*(parambuf + k) = 0; k--;
}
if (k < 0) {
	return 0;
}
if(value){
*value=atoi(parambuf);
}
 return k;

 
}
	OpenFlag=0;
	return -1;
}

/////////////////////////////////////////////////////////////
int ini::OpenLastString(char* paramname, char* parambuf, int BuffSize, int NextSeek, BOOL IsParam){

int i,j,k,l,len;
char* pcur1;
char* pcur2;
i=j=k=len=0;
OpenFlag=1;

if(!OpenIni){return -1;}

if(!NextSeek || PSeek==0){
	PSeek=Psect;
	SeekSize=SectSize;
}

l=strlen(paramname);

while(i<SeekSize){
wh1:
	pcur1=paramname+l-1;
	pcur2=(char*)PSeek+SeekSize-i;
	if(*pcur1==*pcur2){
		j=1;
		while(j<l){
			pcur1=paramname+l-1-j;
			pcur2=(char*)PSeek+SeekSize-i-j;
			if(*pcur1!=*pcur2){
				i+=j;
				j=0;
			    goto wh1;
			}
			j++;
		}
		
		pcur2=(char*)PSeek+SeekSize+1-i;
		while(k<i && IsParam){
			if(*(pcur2+k)!=0x20 && *(pcur2+k)!=0x3D )
				break;
			
			k++;
		}
	
		while(k<i && BuffSize){
			BuffSize--;
			if(*(pcur2+k)!=0x0D && *(pcur2+k)!=0x0A && BuffSize>0){
			*parambuf=*(pcur2+k);
			}
			else{
				*parambuf=0; goto exf;}
			parambuf++; k++; len++;
		}
		goto exf;

	}//fi(*pcur1==*pcur2)


i++;
}//end_while(i<SeekSize)
return -1;
exf:
SeekSize-=(i+l);
if(SeekSize<0){SeekSize=0;}

if(BuffSize>0)
return len;
else
return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
int ini::OpenLastMultiString(char** strarr, int sizearr, char* parambuf, int BuffSize, int NextSeek, BOOL IsParam){
int i,j,k,l,len;
char* pcur1;
char* pcur2;
char* paramname;
i=j=k=len=0;
OpenFlag=1;
int n;

if(!OpenIni){return -1;}

if(!NextSeek || PSeek==0){
	PSeek=Psect;
	SeekSizeM=SectSize;
}
n=0;


while(i<SeekSizeM){

 for(n=0;n<sizearr;n++){
wh1:
	paramname=strarr[n];
	l=strlen(paramname);

	pcur1=paramname+l-1;
	pcur2=(char*)PSeek+SeekSizeM-i;
	if(*pcur1==*pcur2){
		j=1;
		while(j<l){
			pcur1=paramname+l-1-j;
			pcur2=(char*)PSeek+SeekSizeM-i-j;
			if(*pcur1!=*pcur2){
				j=0;
				n++;
				if(n>=sizearr){
				i++;
				n=0;
				}
			  goto wh1;
			}
			j++;
		}
		
		pcur2=(char*)PSeek+SeekSizeM+1-i;
		while(k<i && IsParam){
			if(*(pcur2+k)!=0x20 && *(pcur2+k)!=0x3D )
				break;
			
			k++;
		}
	
		while(k<i && BuffSize){
			BuffSize--;
			if(*(pcur2+k)!=0x0D && *(pcur2+k)!=0x0A && BuffSize>0){
			*parambuf=*(pcur2+k);
			}
			else{
				*parambuf=0; goto exf;}
			parambuf++; k++; len++;
		}
		goto exf;

	}//fi(*pcur1==*pcur2)
 
 }//end_for(n=;n<sizearr;n++)


i++;
}//end_while(i<SeekSizeM)

return -1;
exf:
SeekSizeM-=(i+l);
if(SeekSizeM<0){SeekSizeM=0;}

if(BuffSize>0)
return n;
else
return -1;


}
///////////////////////////////////////////////////////////////////////////////////////////
int ini::GetStringBetween(char* Left, char* Right, int Num, char* instr, char* outbuf, int BuffSize, BOOL IsParam){

int i,j,k,l,len, outlen, lL, lR;
char* pL;
char* pR;
char* pcur;

i=j=k=l=len=lL=lR=0;
if(Num==0){Num=1;}
if(Left!=0) lL=strlen(Left);
if(Right!=0)lR=strlen(Right);
len=strlen(instr);
	if(len==0){return -1;}
pcur=instr;

if(lL==0){goto FindL2;}

whl:
while(i<len){
	if(*(Left+j) == *(pcur+i+j)){
		j++;
		if(j==lL){
			i+=j;
			j=0;
			goto FindL;}
	}
	else{
		j=0;
		i++;
	}
}//end_whl

return -1;

FindL:
Num--;
if(Num>0){
	goto whl;
}
FindL2:
pL=pcur+i;

if(lR==0){i=len; goto FindR2;}

whr:
while(i<len){
	if(*(Right+j) == *(pcur+i+j)){
		j++;
		if(j==lR){
			goto FindR;}
	}
	else{
		j=0;
		i++;
	}
}//end_whr

return -1;

FindR:
Num--;
if(Num>0){
i+=j;
j=0;
goto whr;
}
FindR2:
pR=pcur+i;
if(IsParam){
	while( pL<pR && *pL==0x20 ){
		 pL++;
	}
  if(pL>=pR){return -1;}
	while( pL<pR && *(pR-1)==0x20 ){
		 pR--;
	}

}
outlen=pR-pL;

if(BuffSize<outlen)
return 0;

k=outlen;

 while(l<k){
		*(outbuf+l)=*(pL+l);
		l++;
 }
*(outbuf+l)=0;
return k;

return 0;


}



/////////////////////////////////////////////////////////////
int ini::UpdateParam(char* parambuf, char* paramname){

DWORD lenpnew;
char* pcur1;
char* R_EDI;
char* R_ESI;
char pbuf[128];
DWORD R_ECX;
DWORD DiffLen;

if(!OpenIni){return -1;}

if(paramname){
	if(-1==this->OpenParam(paramname,pbuf))
		return 0;
}

if(!SectSize)
	return 0;
pcur1=PParam;
lenpnew=strlen(parambuf);

if(lenpnew==LenParam){ //новое значение параметра по длине как старое

CX=0;
while(CX<lenpnew){
	pcur1[CX]=parambuf[CX];
	CX++;
}
	OpenFlag=0;
	return 1;
}

if(lenpnew>LenParam){ //новое значение параметра длиннее старого

DiffLen=lenpnew-LenParam;

R_ESI=(char*)Pini+IniSize-1;
R_EDI=(char*)R_ESI+DiffLen;
R_ECX=R_ESI-pcur1+1;

CX=R_ECX;
while(CX){
	*R_EDI=*R_ESI;
	R_EDI--;
	R_ESI--;
	CX--;
}
CX=0;
while(CX<lenpnew){
pcur1[CX]=parambuf[CX];
CX++;
}

IniSize+=DiffLen;
SectSize+=DiffLen;
LenParam=lenpnew;
OpenFlag=0;
return 1;
}

if(lenpnew<LenParam){ //новое значение параметра короче старого

DiffLen=LenParam-lenpnew;

R_ESI=pcur1+LenParam;
R_EDI=R_ESI-DiffLen;
R_ECX=IniSize+(char*)Pini-pcur1;

CX=0;
while(CX<R_ECX){
	*R_EDI=*R_ESI;
	R_EDI++;
	R_ESI++;
	CX++;
}
CX=0;
while(CX<lenpnew){
pcur1[CX]=parambuf[CX];
CX++;
}


IniSize-=DiffLen;
SectSize-=DiffLen;
OpenFlag=0;
return 1;
}



return 0;
}
/////////////////////////////////////////
int ini::CloseIni(){


 if(hini!=INVALID_HANDLE_VALUE){
	CloseHandle(hini);
	hini=INVALID_HANDLE_VALUE;
 }
 if(OpenIni){
	BOOL rez=VirtualFree(Pini,0,MEM_RELEASE);
	Pini=0;
	OpenIni = 0;
	RegIni = 0;
	return 1;
 }

return 0;

}



/////////////////////////////////////////////////////////////////////////////////////////
int ini::Save2Reg(char* NameKey, char* NameValue, BOOL IsCpt) {
DWORD PDWORD = 0;
DWORD DWTYPE;
HKEY hkey = 0;
	
if (Pini==0 || (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, NameKey, &hkey))) {
return 0;
}
if (IsCpt) {
	CPT((char*)Pini, IniSize);
}
RegSetValueEx(hkey, NameValue, NULL, REG_BINARY, (unsigned char *)Pini, (DWORD)IniSize);
RegCloseKey(hkey);
if (IsCpt) {
	DeCPT((char*)Pini, IniSize);
}

return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
int ini::Save2File(char* FileName, BOOL IsCpt) {
HANDLE hsini;
DWORD DWBW = 0;

if (Pini == 0) {
	return 0;
}
hsini = CreateFile((LPCTSTR)FileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
if (hsini == INVALID_HANDLE_VALUE) {
	return 0;
}
if (IsCpt) {
	CPT((char*)Pini, IniSize);
}
WriteFile(hsini, Pini, IniSize, &DWBW, NULL);
CloseHandle(hsini);
if (IsCpt) {
	DeCPT((char*)Pini, IniSize);
}
if (DWBW == IniSize) {
	return 1;
}
return 0;
}

/////////////////////////////////////////
int ini::SaveAndCloseIni(){

DWORD PDWORD;

if(hini==INVALID_HANDLE_VALUE || RegIni)
return 0;


DWORD SFP=SetFilePointer(hini,0,0,FILE_BEGIN);
SetEndOfFile(hini);
WriteFile(hini,Pini,IniSize,&PDWORD,0);
CloseHandle(hini);
hini=INVALID_HANDLE_VALUE;
BOOL rez=VirtualFree(Pini,0,MEM_RELEASE);
Pini = 0;
OpenIni = 0;

return 1;
}
/////////////////////////////////////////
int ini::SetSection(char* SectName){

DWORD SectSizeOld;
char* PsectOld;
char FullSectName[128];
int OP;

if(hini == INVALID_HANDLE_VALUE && !RegIni)
return 0;

PsectOld=PsectPrev=Psect;
SectSizeOld=SectSizePrev=SectSize;
pFindPrev = pFind;
Psect=(char*)Pini;
SectSize=IniSize;

if (SectName == 0 || !strlen(SectName)) {
	Psect = PRootSect;
	SectSize = IniSize;
	pFind = 0;
	return 1;
}

strcpy(FullSectName,"[");
strcat(FullSectName,SectName);
strcat(FullSectName,"]");
OP=this->OpenParam(FullSectName,FullSectName);
if(OP==-1){
Psect=PsectOld;
SectSize=SectSizeOld;
return 0;
}
OpenFlag=0;
SectSize=0;
Psect=PParam;
pFind = 0;

wh2:	
	while(*(Psect+SectSize)!=0x5B){
		if((char*)(Psect+SectSize)>=EndIni){
		return 1;
		}
	
		SectSize++;
	}

if(*(Psect+SectSize-1)==0x0D || *(Psect+SectSize-1)==0x0A)
return 1;

SectSize++; 
goto wh2;
}

/////////////////////////////////////////////////////////
int ini::SetPrevSection() {

char *Pswap;
DWORD DSWAP;

	if (PsectPrev != 0 && SectSizePrev != 0) {
		Pswap = Psect; Psect = PsectPrev; PsectPrev = Pswap;
		Pswap = pFind; pFind = pFindPrev; pFindPrev = Pswap;
		DSWAP = SectSize; SectSize = SectSizePrev; SectSizePrev = DSWAP;
		return 1;
	}
	else {
		return 0;
	}

}

/////////////////////////////////////////////////////////
int ini::AddParam(char* paramname, char* separate, char* parambuf){

char* pcur1;
char* ppbuf;
char* R_EDI;
char* R_ESI;
char pbuf[128];
DWORD R_ECX;
DWORD DiffLen;

if(hini==INVALID_HANDLE_VALUE || !paramname || Encrypt)
return 0;

DiffLen=strlen(paramname);
if(DiffLen==0)
return 0;
pbuf[0]=0;
pcur1=Psect+SectSize;

if(*(pcur1-1)!=0x0A){
pbuf[0]=0x0D;
pbuf[1]=0x0A;
pbuf[2]=0;
}
strcat(pbuf,paramname);
strcat(pbuf,separate);
strcat(pbuf,"\r\n");
DiffLen=strlen(pbuf);

R_ESI=(char*)Pini+IniSize-1;
R_EDI=(char*)R_ESI+DiffLen;
R_ECX=R_ESI-pcur1+1;
ppbuf=pbuf;


while(pcur1>(Psect+2) && (*(pcur1-3)==0x0D || *(pcur1-3)==0x0A)){ //условие pcur1<Psect должно быть на первом месте, иначе может возникнуть исключение
pcur1--;
R_ECX++;
}
if(pcur1==(char*)Pini+2 && (*(WORD*)Pini)==0x0A0D){
pcur1-=2;
R_ECX+=2;
}

CX=R_ECX;
while(CX){
	*R_EDI=*R_ESI;
	R_EDI--;
	R_ESI--;
	CX--;
}
CX=0;
while(CX<DiffLen){
pcur1[CX]=ppbuf[CX];
CX++;
}

IniSize+=DiffLen;
SectSize+=DiffLen;
OpenFlag=0;
return 1;

	return 0;
}

int ini::CreateIni(char* NameIni){

DWORD PPDWORD;
	
	if(hini!=INVALID_HANDLE_VALUE)
	return 0;

hini=CreateFile( (LPCTSTR)NameIni,GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,0,
				OPEN_ALWAYS,FILE_ATTRIBUTE_ARCHIVE,0);

if(hini==INVALID_HANDLE_VALUE)
	return 0;

if(GetFileSize(hini,0)==0){
WriteFile(hini,&Wnstr,2,&PPDWORD,NULL);
}
CloseHandle(hini);
hini=INVALID_HANDLE_VALUE;
return 1;
}

/////////////////////////////////////////////////////////////
int ini::FindFirstStr(char* parambuf)
{
	int l;
	BOOL IsStrEntry;
	char* PsectEnd;

	if (!OpenIni || !SectSize || Encrypt) {
		return -1;
	}

	PsectEnd = Psect + SectSize;
	pFind = Psect;
	IsStrEntry = false;
	l = 0;
	OpenFlag = 1;

	//pcur2 = (char*)Psect + i + j;

	while (pFind < PsectEnd && *pFind != '[') {
		if (*pFind == 0x20 || *pFind == 0x0D || *pFind == 0x0A || *pFind == 0x09) {
			pFind++;
			continue;
		}
		if (*pFind != '#') {
			IsStrEntry = true;
			while (*pFind != 0x0D && *pFind != 0x0A && *pFind != '[' && pFind < PsectEnd) {
				*(parambuf + l) = *pFind;
				l++; pFind++;
			}
			break;
		}
		else {//*pFind == '#'
			while (*pFind != 0x0D && *pFind != 0x0A && *pFind != '[' && pFind < PsectEnd) {
				pFind++;
			}
		}
	}
	parambuf[l] = 0;
	OpenFlag = 0;
	return l;
}

/////////////////////////////////////////////////////////////
int ini::FindNextStr(char* parambuf) {
	int l;
	BOOL IsStrEntry;
	char* PsectEnd;

	if (!OpenIni || !SectSize || Encrypt) {
		return -1;
	}
	if (pFind == 0) {
		pFind = Psect;
	}
	PsectEnd = Psect + SectSize;
	if (pFind >= PsectEnd) {
		return -1;
	}
	IsStrEntry = false;
	l = 0;
	OpenFlag = 1;

	while (pFind < PsectEnd && *pFind != '[') {
		if (*pFind == 0x20 || *pFind == 0x0D || *pFind == 0x0A || *pFind == 0x09) {
			pFind++;
			continue;
		}
		if (*pFind != '#') {
			IsStrEntry = true;
			while (*pFind != 0x0D && *pFind != 0x0A && *pFind != '[' && pFind < PsectEnd) {
				*(parambuf + l) = *pFind;
				l++; pFind++;
			}
			break;
		}
		else {//*pFind == '#'
			while (*pFind != 0x0D && *pFind != 0x0A && *pFind != '[' && pFind < PsectEnd) {
				pFind++;
			}
		}
	}
	parambuf[l] = 0;
	OpenFlag = 0;
	return l;
}


/////////////////////////////////////////////////////////////
int ini::FindFirstAnyStr(char* parambuf)
{
	int l;
	BOOL IsStrEntry;
	char* PsectEnd;

	if (!OpenIni || !SectSize || Encrypt) {
		return -1;
	}

	PsectEnd = Psect + SectSize;
	pFind = Psect;
	IsStrEntry = false;
	l = 0;
	OpenFlag = 1;

	//pcur2 = (char*)Psect + i + j;

	while (pFind < PsectEnd) {
		if (*pFind == 0x20 || *pFind == 0x0D || *pFind == 0x0A || *pFind == 0x09) {
			pFind++;
			continue;
		}
		IsStrEntry = true;
		while (*pFind != 0x0D && *pFind != 0x0A && pFind < PsectEnd) {
			*(parambuf + l) = *pFind;
			l++; pFind++;
		}
		break;
	}
	parambuf[l] = 0;
	OpenFlag = 0;
	return l;
}


/////////////////////////////////////////////////////////////
int ini::FindNextAnyStr(char* parambuf) {
	int l;
	BOOL IsStrEntry;
	char* PsectEnd;

	if (!OpenIni || !SectSize || Encrypt) {
		return -1;
	}
	if (pFind == 0) {
		pFind = Psect;
	}
	PsectEnd = Psect + SectSize;
	if (pFind >= PsectEnd) {
		return -1;
	}
	IsStrEntry = false;
	l = 0;
	OpenFlag = 1;

	while (pFind < PsectEnd) {
		if (*pFind == 0x20 || *pFind == 0x0D || *pFind == 0x0A || *pFind == 0x09) {
			pFind++;
			continue;
		}
		IsStrEntry = true;
		while (*pFind != 0x0D && *pFind != 0x0A && pFind < PsectEnd) {
			*(parambuf + l) = *pFind;
			l++; pFind++;
		}
		break;
		
	}
	parambuf[l] = 0;
	OpenFlag = 0;
	return l;
}


/////////////////////int NormalizationStr(char* str, BOOL IsStrDir=false)////////////////////////
int ini::NormalizationStr(char* str, BOOL IsStrDir ) {
int i, len;
char *pch;
char *pend;
BOOL IsBeginStr;

len = strlen(str);
i = len-1;
while (i != 0 && str[i] == ' ') {
	i--;
}
len = i;

if (len >= sizeof(tmpstr)) {
	return 0;
}
pch = str; pend = pch + len + 1;
i = 0;
IsBeginStr = true;
while (pch < pend) {
	if (*pch == ' ' && IsBeginStr) {
		pch++; continue;
	}
	IsBeginStr = false;
	tmpstr[i] = *pch;
	i++; pch++;
}
if (!IsStrDir) {
	tmpstr[i] = 0;
	strcpy(str, tmpstr);
	return i;
}
if (tmpstr[i - 1] != 0x5C) {
	tmpstr[i] = 0x5C; i++;
}
tmpstr[i] = 0;
strcpy(str, tmpstr);
return i;
	
}