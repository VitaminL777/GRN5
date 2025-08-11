#ifndef _SELFSTRUCTS_
#define _SELFSTRUCTS_
#define SOFSIZE 192

//STATUS_FLAGS
#define SRV_CONN_OK 0b1000000000000000
#define HASH_OK		0b0100000000000000
#define PASSW_OK	0b0010000000000000
#define SRV_FULL_OK 0b1110000000000000
#define REM_PC_OK	0b0001000000000000
#define FULL_OK		0b1111000000000000
//STATUS_FLAGS

//COLOR_FLAGS
#define COLOR_R		1 
#define COLOR_G		2
#define COLOR_Y		3 
#define COLOR_G		4
//COLOR_FLAGS

//SS_STATUS
#define STATUS_NONE  0
#define STATUS_STOP  1
#define STATUS_START 2
//SS_STATUS

union UFileTime{
	FILETIME FileTime;
	__int64 int64;
};

typedef struct _WLX_NOTIFICATION_INFO 
{
  ULONG  Size ;
  ULONG  Flags ;
  PWSTR  UserName ;
  PWSTR  Domain ;
  PWSTR  WindowStation ;
  HANDLE hToken ;
  HDESK  hDesktop ;
  void* pStatusCallback;
} WLX_NOTIFICATION_INFO, * PWLX_NOTIFICATION_INFO ;

typedef struct _SO_FLAGS 
{
  WORD Atm;
  DWORD Len;
  char Cmd[384];

} SO_FLAGS, *PSO_FLAGS ;

typedef struct _SID_HLP 
{
  char head[8];
  DWORD DWSID[100];

} SID_HLP, *PSID_HLP ;

typedef struct _TIME_PROGRESS
{
	DOUBLE  DStep;
	DOUBLE  DCurPos;
	DOUBLE  DEndPos;
} TIME_PROGRESS, *PTIME_PROGRESS;

typedef struct _WAVHEAD {
	char	RIFF[4];
	DWORD	RiffSz;
	char	WAVEfmt[8];
	DWORD	FmtSz;
	WORD	wFormatTag;         /* format type */
	WORD	nChannels;          /* number of channels (i.e. mono, stereo...) */
	DWORD	nSamplesPerSec;     /* sample rate */
	DWORD	nAvgBytesPerSec;    /* for buffer estimation */
	WORD	nBlockAlign;        /* block size of data */
	WORD	wBitsPerSample;
}WAVHEAD, *PWAVHEAD;

typedef struct _WAVDATA {
	char	DATA[4];
	DWORD	DataSz;
}WAVDATA, *PWAVDATA;

typedef struct _WAVBLOCK {
	char	*WStart;
	char	*WEnd;
}WAVBLOCK, *PWAVBLOCK;

typedef struct _NETRES {
	char Share[128];
	char MountPoint[64];
	char User[64];
	char Password[96];
}NETRES, *PNETRES;

#endif /* _SELFSTRUCTS_ */
