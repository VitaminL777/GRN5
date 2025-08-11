// ini.h: interface for the ini class.
//
//////////////////////////////////////////////////////////////////////
/*
ini-класс для работы с файлами ini-типа
Все функции (кроме OpenParam, OpenLastString, OpenLastMultiString и GetStringBetween) возвращают 1 
в случае успеха и 0 в случае неудачи. OpenParam в случае успеха возвращает количество байт 
в значении открытого параметра (0-если параметру значение не присвоено) и -1 в случае неудачи.
OpenLastString в случае успеха возвращает кол-во байт в считанной строке, 0 - в случае, если
указанный размер буфера недостаточен для приёма данной строке и -1 в случае если не найдена
такая строка в указанном диапазоне поиска.
OpenLastMultiString возвращает номер строки во входном массиве, которая была найдена первой, с заданной
позиции поиска и -1 в случае любой неудачи

 ReadIni(char* NameIni, BOOL ReadOnly=0) с ReadOnly=0 открывает файл с указанным именем и копирует его 
содержимое в память, все последущие операции с ini производятся имеено с этим образом,(хотя файл остаётся при
этом открытым) а не с самим ini-файлом. После вызова SaveAndCloseIni() происходит запись образа в файл с 
его закрытием. Если ReadOnly=1, то файл открывается с правами только на чтение, считывается его образ в память
и сам файл тут же закрывается, последущие операции типа SaveAndCloseIni и Upadte* в этом случае невозможны.
 
 SetSection(char* SectName) устанавливает область видимости для функций
OpenParam, UpdateParam, AddParam, OpenLastString(с NextSeek=0) внутри секции
с именем SectName. Имя секции передается без символов "[" "]". SetSection(0)
задает область видимости всего файла.

 OpenParam(char* paramname,char* parambuf,int* value=0) ищет в образе ini  - параметр
с именем paramname и считанное значение этого параметра заносит в буфер
parambuf(если параметру присвоено значение). Параметр от значения может
быть отделен символами пробел,"=" или их комбинацией в любом сочетании.
Если указать ненулевое значение value(указатель на переменную int),то
функция запишет в эту переменную значение 'atoi'от parambuf

 OpenParam_n - тоже, что и OpenParam за исключением того, что возвращает
не просто первый найденный параметр с данным именем, а n-й параметр, отсчитанный
от начала заданной секции (имеется ввиду, что таких параметров несколько).

 OpenLastString - возвращает строку, начинающуюся со значения paramname,(возвращает без этого значения),
причём начинает поиск с конца активной секции, нулевый параметр NextSeek организует поиск с самого конца
активной секции в направлении начала, а ненулевой параметр NextSeek - продолжает поиск вверх, если он был
уже начат, а если не был начат, то начинает так же как с NextSeek=0. Если IsParam=1, то строка возвращается
как параметр по аналогии с OpenParam, т.е без пробелов и знаков "=" в начале строки, если IsParam=0, - то
возвращается целиком как есть. Также на входе необходимо указывать размер буфера BuffSize.
Для этой функции нет соответсвующих Update-функций, поэтому работает только на чтение.
 
  OpenLastMultiString - похожа на OpenLastString, но ищет первую попавшуюся строку с заданной позиции поиска
начинающуюся со строки из числа заданных строк во входном массиве strarr с заданным размером sizearr
и возвращает номер заданной строки во входном массиве и также полную найденную строку в parambuf.
 
  GetStringBetween - возвращает строку, находящуюся между строками Left и Right, Num - задаёт номер вхождения
строки Left в искомую строку. Left и Right могут быть равными 0 или указывать на пустые строки, что означает
поиск с начала строки(Left=0) или поиск до конца строки (Right=0). Если Left=0, а Right-ненулевая строка, то
в этом случае параметр Num указывает на номер вхождения строки Right. Остальные параметры аналогичны OpenLastString

 UpdateParam(char* parambuf) с одним параметром обновляет значение параметра
из parambuf,для которого последний раз вызывалась OpenParam.
 UpdateParam(char* parambuf, char* paramname=0) с двумя параметрами
самостоятельно открывает и обновляет значение параметра с именем
paramname. 
Функция AddParam(char* paramname, char* separate="=", char* parambuf=0)
добавляет
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
