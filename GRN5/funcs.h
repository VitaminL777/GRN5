#pragma once

int StriStr(char *Str, char *SubStr, char **SubCh = NULL);
int GetBaseName(char *FullName, char *BaseName, char *Ext = NULL);
void DeCPT(char *Data, int size);
void CPT(char *Data, int size);
int ToLowerStr(IN char *strsrc, OUT char *strlow);
BOOL IsFileExist(char *FileName);

