#pragma once
#define SUCCESS 1
#define FAILURE -1
#define RECURSE -2
#define NO_CHANGE -3
#define MAX_PATH_SIZE 33000
#define INITIAL_SIZE 1024
#define MAX_BASE_10_LEN 6
#define MEMCPY_S(a, b, c, d) if(memcpy_s(a, b, c, d) != 0){printf_s("memcpy failure\n"); return FAILURE;}
#define MEMCPY_S_FREE(a, b, c, d, e) if(memcpy_s(a, b, c, d) != 0){printf_s("memcpy failure\n");free(e); return FAILURE;}
#define IS_NULL_TERMINATED(pChar, size) if(strnlen_s(pChar, size) == size){printf_s("string not null terminated\n"); return FAILURE;}
#define SEC_LEN(str, size, store) if((store = strnlen_s(str, size)) == size){printf_s("string too big\n"); return FAILURE;}
#define CLOSE_HANDLE(h) if(CloseHandle(h) == 0){return FAILURE;}


DWORD setEvilDetails();

DWORD RunProgram();

int getFilePath(char* cpPath, char* cpNewPath);

DWORD WritetoFile(HANDLE hFile);

DWORD ReadFromFile(char* cpPath);

HANDLE CeateFileProcess(char* cpPath, int* error_code);

char* RemoveLastPart(char* cpPath);

char* AddBlock(char* cpCurrent, char* cpFull);

BOOL IsEvilRequest(char* cpPath);

BOOL IsEvilRequest(char* cpPath);

BOOL IsSamePath(char* cpPath2);

BOOL IsSameHandle(HANDLE hFile2);

BOOL IsExecutableExtension(char* cpPath);

char* GetPointerToExtension(char* cpPath);

char* ToSearchableFormat(char* str);

void ToUpper(char* str, size_t dwMax);

DWORD getActualPath(char* cpPath, char* cpNewPath);

BOOL IsExecutableHeader(char* content);

DWORD CreateAllSubDirs(char* cpPath);

DWORD BuildDirs(char* cpWorkingPath, char* cpFullPath);

BOOL ResolveSymbolics(char* cpPath);

BOOL GetFirstSymbolic(char* cpPath);

BOOL IsSymbolic(char* cpPath);

BOOL GetCBasedDrive(char* cpPath, char* cpPathOut);

char* strsub(char* input, char* substring, char* replace);

BOOL substr(char* org, char* what, char* with);

BOOL strfixsmall(char* old, size_t start, size_t end, char* middle);

BOOL HandleOffsetExpression(char* cpPath, size_t start, size_t end, char* varVal);

size_t find_char(char* str, char val, size_t max_size);

BOOL ParseComplexExpresions(char* cpPath);

BOOL parseInnerPart(char* cpPath, size_t start, size_t end, int* piOffset);

BOOL IsOffsetExpression(char* cpStr, size_t start, size_t end);

BOOL HandleReplaceExpression(char* cpPath, char* copy, char* varVal, size_t start, size_t end, size_t current, size_t ColIdx);

int runCommand();

BOOL ResolveWildCard(char* cpPath, size_t idx, int* offset, size_t path_len);

BOOL FindAndResolveWildCards(char* path);

BOOL substrUpTo(char* org, char* what, char* with);

BOOL RunCmd(char* exp, char* output);

BOOL IsEscapable(char a);

BOOL escapeEscapables(char* cmd, size_t sz);

BOOL addEscape(char* cmd, int i, size_t sz);

BOOL getFromCMD(char* cpPath, size_t start, size_t end, int* piOffset, char* output);
