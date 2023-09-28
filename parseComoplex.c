#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include"main.h"
#include<fileapi.h>
#include<ctype.h>
#include<processenv.h>
#include<string.h>


BOOL ParseComplexExpresions(char* cpPath) {
	DWORD first = -1;
	BOOL escaped = FALSE;
	int offset = 0;
	for (DWORD i = 0;i < strlen(cpPath); i++) {
		if (cpPath[i] == '%') {
			if (first == -1) {
				first = i;

			}
			else
			{
				BOOL bRetVal = parseInnerPart(cpPath, first, i, &offset);
				if (bRetVal == FAILURE) {
					puts("failed while parsing complex enviroment variables");
					return FAILURE;
				}
				else if (bRetVal == NO_CHANGE) {
					first = i;
				}
				else
				{
				    i += offset;
					first = -1;
					continue;
				}

			}
		}
	}
	escaped = FALSE;
	for (DWORD i = 0;i < strlen(cpPath); i++) {
		if (cpPath[i] == '^' && escaped == FALSE) {
			escaped = TRUE;
				RemoveCharFromString(cpPath, i, MAX_PATH_SIZE - i);
				i--;
		}
		else
		{
			escaped = FALSE;
		}
		
	
	
	}


	return SUCCESS;
}

BOOL getFromCMD(char* cpPath, size_t start, size_t end, int* piOffset, char* output) {
	char varName[MAX_PATH_SIZE] = { 0 };
	memset(varName, 0, MAX_PATH_SIZE);
	memset(output, 0, MAX_PATH_SIZE);
	MEMCPY_S(varName, MAX_PATH_SIZE, cpPath + start, end-start+1);
	if (RunCmd(varName, output) == FAILURE)
		return FAILURE;

	if (strncmp(varName, output, MAX_PATH_SIZE) == 0)
		return NO_CHANGE;

	size_t newlen = 0;
	SEC_LEN(output, MAX_PATH_SIZE, newlen);

	int offset = newlen - strlen(varName);
	char part1[MAX_PATH_SIZE] = { 0 };
	MEMCPY_S(part1, MAX_PATH_SIZE, cpPath, MAX_PATH_SIZE);

	if (strfixsmall(part1, start + 1, end + 1, output) == FAILURE) {
		return FAILURE;
	} // one added to account for the % which is being removed
	memset(cpPath, 0, strlen(cpPath));
	MEMCPY_S(cpPath, MAX_PATH_SIZE, part1, MAX_PATH_SIZE);
	*piOffset = offset;

	return SUCCESS;
}


BOOL parseInnerPart(char* cpPath, size_t start, size_t end, int* piOffset) {
	BOOL bOffsetExp = FALSE;
	BOOL envEnd = FALSE;
	BOOL escaped = FALSE;
	size_t stColIdx = 0;
	char temp[MAX_PATH_SIZE] = { 0 };
	char varVal[MAX_PATH_SIZE] = { 0 };
	char copy[MAX_PATH_SIZE] = { 0 };
	for (size_t i = start; i < end; i++) {
		if ((cpPath[i] == ':') && (envEnd == FALSE)) {
			
			envEnd = TRUE;
			stColIdx = i;
			memset(temp, 0, MAX_PATH_SIZE);
			MEMCPY_S(temp, MAX_PATH_SIZE, cpPath + start + 1, i - start - 1);
			if (GetEnvironmentVariableA(temp, varVal, MAX_PATH_SIZE) == 0) {
				errno_t e = GetLastError();
				if (e == ERROR_ENVVAR_NOT_FOUND) {
					//*piOffset = 0;
					return getFromCMD(cpPath, start, end, piOffset, varVal);
				}
				else
				{
					printf_s("get env variable has failed with error code %d\n", e);
					return FAILURE;
				}
			}
	
			if (cpPath[i + 1] == '~' && IsOffsetExpression(cpPath, i + 2, end)) {
				MEMCPY_S(copy, MAX_PATH_SIZE, cpPath, MAX_PATH_SIZE);
				HandleOffsetExpression(cpPath, stColIdx+2, end, varVal);
				size_t newlen = 0;
				SEC_LEN(varVal, MAX_PATH_SIZE, newlen);

				int offset = newlen - (end - start) - 1;
				char part1[MAX_PATH_SIZE] = { 0 };
				if (strfixsmall(copy, start + 1, end + 1, varVal) == FAILURE) {
					return FAILURE;
				} // one added to account for the % which is being removed
				memset(cpPath, 0, strlen(cpPath));
				MEMCPY_S(cpPath, MAX_PATH_SIZE, copy, MAX_PATH_SIZE);
				*piOffset = offset;
				return SUCCESS;
			}
		}
		/*
		* if it gets here its either a replacement case or nothing
		*/
		if ((cpPath[i] == '=') && envEnd == TRUE && escaped == FALSE) {
				if (i == stColIdx+1)
				{
					printf_s("invalid expresion\n");
					return FAILURE;

				}

				if (HandleReplaceExpression(cpPath, copy, varVal, start, end, i, stColIdx) == FAILURE) {
					printf("failed while resolving complex expression\n");
					return FAILURE;
				}

				size_t newlen = 0;
				SEC_LEN(varVal, MAX_PATH_SIZE, newlen);

				int offset = newlen - (end - start) - 1;
				char part1[MAX_PATH_SIZE] = { 0 };
				if (strfixsmall(copy, start + 1, end + 1, varVal) == FAILURE) {
					return FAILURE;
				} // one added to account for the % which is being removed
				memset(cpPath, 0, strlen(cpPath));
				MEMCPY_S(cpPath, MAX_PATH_SIZE, copy, MAX_PATH_SIZE);
				*piOffset = offset;
				return SUCCESS;
		}
		if (cpPath[i] == '^' && escaped == FALSE)
		{
			escaped = TRUE;
		/*
			RemoveCharFromString(cpPath, i, MAX_PATH_SIZE - i);
			i--;
		*/
		}
		else
		{
			escaped = FALSE;
		}
	}
	return getFromCMD(cpPath, start, end, piOffset, varVal);
}

/*
* put middle in old in idx start
*/
BOOL strfixsmall(char* old, size_t start, size_t end, char* middle) {
	errno_t e = 0;
	char temp1[MAX_PATH_SIZE] = { 0 };
	MEMCPY_S(temp1, MAX_PATH_SIZE, old, start == 0 ? 0:start - 1 );
	IS_NULL_TERMINATED(temp1, MAX_PATH_SIZE);
	if ((e = strcat_s(temp1, MAX_PATH_SIZE, middle)) != 0) {
		printf_s("strcat_s failed with error code %d\n", e);
		return FAILURE;
	}
	IS_NULL_TERMINATED(temp1, MAX_PATH_SIZE);
	if ((e = strcat_s(temp1, MAX_PATH_SIZE, old + end)) != 0) {
		printf_s("strcat_s failed with error code %d\n", e);
		return FAILURE;
	}
	IS_NULL_TERMINATED(temp1, MAX_PATH_SIZE);
	memset(old, 0, MAX_PATH_SIZE);
	MEMCPY_S(old, MAX_PATH_SIZE, temp1, MAX_PATH_SIZE);

	return SUCCESS;
}



BOOL substr(char* org, char* what, char* with) {
	char* st = NULL;
	char* chOgOrg = org;
	size_t stWithLen = 0;
	SEC_LEN(with, MAX_PATH_SIZE, stWithLen);
	while ((st = strstr(org, what)) != NULL) {
		size_t start = st - chOgOrg;
		if (strfixsmall(chOgOrg, start + 1, start + strlen(what), with) == FAILURE)
			return FAILURE;
		org = (st + stWithLen);
	}
	if (strnlen_s(org, MAX_PATH_SIZE) == MAX_PATH_SIZE) {
		return FAILURE;
	}
	org = chOgOrg;
	return SUCCESS;
}

BOOL substrUpTo(char* org, char* what, char* with) {
	char* st = NULL;
	char* chOgOrg = org;
	size_t stWithLen = 0;
	SEC_LEN(with, MAX_PATH_SIZE, stWithLen);
	if ((st = strstr(org, what)) != NULL) {
		char temp[MAX_PATH_SIZE] = { 0 };
		
		if (strcat_s(temp, MAX_PATH_SIZE, with) != 0) {
			printf_s("cpy failed\n");
			return FAILURE;
		}


		if (strcat_s(temp, MAX_PATH_SIZE, st + strlen(what)) != 0) {
			printf_s("cpy failed\n");
			return FAILURE;
		}
		memset(chOgOrg, 0, MAX_PATH_SIZE);
		MEMCPY_S(chOgOrg, MAX_PATH_SIZE, temp, MAX_PATH_SIZE);
		IS_NULL_TERMINATED(chOgOrg, MAX_PATH_SIZE);
		org = chOgOrg;
		return SUCCESS;

	}
	return SUCCESS;
}



BOOL IsOffsetExpression(char* cpStr, size_t start, size_t end) {
	BOOL bComma = FALSE;
	size_t stCommaIdx = 0;
	BOOL bNumAfterComma = FALSE;
	BOOL bNumBeforeComma = FALSE;
	for (size_t i = start; i < end; i++) {
		if (cpStr[i] == ',' && bComma == FALSE) {
			bComma = TRUE;
			stCommaIdx = i;
			continue;
		}
		else if (isdigit(cpStr[i])) {
			if (bComma == TRUE && bNumAfterComma == FALSE)
				bNumAfterComma = TRUE;
			else if (bComma == FALSE)
				bNumBeforeComma = TRUE;
			continue;
		}
		else if((i == start && cpStr[i] == '-') || ((i == (stCommaIdx + 1)) && (bComma == TRUE) && cpStr[i] == '-'))
		{
			continue;

		}
		else
		{
			return FALSE;
		}
	}
	return (bNumBeforeComma && bComma == FALSE) || (bNumBeforeComma && bComma && bNumAfterComma);
}


BOOL HandleReplaceExpression(char *cpPath, char* copy, char* varVal, size_t start, size_t end, size_t current, size_t ColIdx) {
	MEMCPY_S(copy, MAX_PATH_SIZE, cpPath, MAX_PATH_SIZE);
	char what[MAX_PATH_SIZE] = { 0 };
	char with[MAX_PATH_SIZE] = { 0 };
	BOOL upTo = FALSE;
	if (*(cpPath + ColIdx+1) == '*') {
		upTo = TRUE;
		MEMCPY_S(what, MAX_PATH_SIZE, cpPath + ColIdx + +2, current - ColIdx - 2);
	}
	else {
		MEMCPY_S(what, MAX_PATH_SIZE, cpPath + ColIdx + 1, current - ColIdx - 1);
	}


	MEMCPY_S(with, MAX_PATH_SIZE, cpPath + current + 1, end - current - 1);
	// replace in temp idx ':' to i with i+1 to end-1
	ToUpper(what, MAX_PATH_SIZE);
	ToUpper(with, MAX_PATH_SIZE);
	ToUpper(varVal, MAX_PATH_SIZE);

	if (upTo == FALSE) {
		if (substr(varVal, what, with) == FAILURE) {
			return FAILURE;
		}
	}
	else
	{
		if (substrUpTo(varVal, what, with) == FAILURE) {
			return FAILURE;
		}
	}
	return SUCCESS;

}


BOOL HandleOffsetExpression(char* cpPath, size_t start, size_t end, char* varVal) {
	long long int  varlen = 0;
	long long int offset = 0;
	long long int NumOfChars = 0;
	char temp[MAX_PATH_SIZE] = { 0 };
	BOOL bOneVal = FALSE;
	SEC_LEN(varVal, MAX_PATH_SIZE, varlen);
	char cpOffset[MAX_BASE_10_LEN + 1] = { 0 }; // if number is more than seven digits itsinvalid as max len can be 32000 (plus negative value)
	char cpNumOfChars[MAX_BASE_10_LEN + 1] = { 0 };
	size_t commaIdx = find_char(cpPath + start, ',', end - start);


	if (commaIdx == (end - start)) {
		// only one sided expression
		char* num = (char*)malloc((end - start) * sizeof(char));
		if (num == NULL) {
			printf_s("malloc failure\n");
			return FAILURE;
		}
		memset(num, 0, end - start);
		MEMCPY_S_FREE(num, end - start, cpPath + start, end - start, num);
		offset = atoi(num);
		free(num);
		if (offset >= varlen) {
			memset(varVal, 0, MAX_PATH_SIZE);
			return SUCCESS;
		}
		else if (-offset >= varlen) {
			return SUCCESS;
		}

		if (offset >= 0) {
			MEMCPY_S(temp, MAX_PATH_SIZE, varVal + offset, varlen - offset);
			memset(varVal, 0, MAX_PATH_SIZE);
			MEMCPY_S(varVal, MAX_PATH_SIZE, temp, MAX_PATH_SIZE);
			return SUCCESS;
		}
		else
		{
			MEMCPY_S(temp, MAX_PATH_SIZE, varVal + (varlen+offset), -offset);
			memset(varVal, 0, MAX_PATH_SIZE);
			MEMCPY_S(varVal, MAX_PATH_SIZE, temp, MAX_PATH_SIZE);
			return SUCCESS;
		}
	}
	else
	{
		char* firstnum = (char*)malloc((commaIdx+1) * sizeof(char));
		if (firstnum == NULL) {
			printf_s("malloc failed aborting\n");
			return FAILURE;
		}
		memset(firstnum, 0, commaIdx + 1);
		MEMCPY_S_FREE(firstnum, commaIdx, cpPath + start,commaIdx, firstnum);
		offset = atoi(firstnum);



		char* secondNum = (char*)malloc(((end - start - commaIdx) + 1) * sizeof(char));
		if (secondNum == NULL) {
			printf_s("malloc failed aborting\n");
			return FAILURE;
		}
		memset(secondNum, 0, (end - start - commaIdx) + 1);
		MEMCPY_S_FREE(secondNum, (end - start - commaIdx), cpPath + start + commaIdx + 1, end - start - commaIdx, secondNum);
		NumOfChars = atoi(secondNum);
		free(secondNum);

		if (offset >= 0 && NumOfChars >= 0) {
			if (offset >= varlen) {
				memset(varVal, 0, MAX_PATH_SIZE);
				return SUCCESS;
			}
			MEMCPY_S(temp, MAX_PATH_SIZE, varVal + offset, min(NumOfChars, (varlen - offset)));
			IS_NULL_TERMINATED(temp, MAX_PATH_SIZE);
			memset(varVal, 0, MAX_PATH_SIZE);
			MEMCPY_S(varVal, MAX_PATH_SIZE, temp, MAX_PATH_SIZE);

			return SUCCESS;
		}
		else if (offset < 0 && NumOfChars < 0) {
			if (NumOfChars <= offset || -NumOfChars >= varlen) {
				memset(varVal, 0, MAX_PATH_SIZE);
				return SUCCESS;
			}
			offset = -offset >= varlen ? -varlen: offset;
			MEMCPY_S(temp, MAX_PATH_SIZE, varVal + max(0, (varlen+offset)), -offset+NumOfChars);
			IS_NULL_TERMINATED(temp, MAX_PATH_SIZE);
			memset(varVal, 0, MAX_PATH_SIZE);
			MEMCPY_S(varVal, MAX_PATH_SIZE, temp, MAX_PATH_SIZE);

			return SUCCESS;
		}
		else if (offset >= 0 && NumOfChars < 0) {
			long long int to = varlen + NumOfChars;
			if (to <= offset || offset >= varlen || -NumOfChars >= varlen) {
				memset(varVal, 0, MAX_PATH_SIZE);
				return SUCCESS;

			}
			MEMCPY_S(temp, MAX_PATH_SIZE, varVal + offset, varlen+NumOfChars-offset);
			IS_NULL_TERMINATED(temp, MAX_PATH_SIZE);
			memset(varVal, 0, MAX_PATH_SIZE);
			MEMCPY_S(varVal, MAX_PATH_SIZE, temp, MAX_PATH_SIZE);

			return SUCCESS;
		}
		else if (offset < 0 && NumOfChars >= 0) {
			MEMCPY_S(temp, MAX_PATH_SIZE, varVal + max(0, varlen+offset), min(NumOfChars, min(-offset, varlen)));
			IS_NULL_TERMINATED(temp, MAX_PATH_SIZE);
			memset(varVal, 0, MAX_PATH_SIZE);
			MEMCPY_S(varVal, MAX_PATH_SIZE, temp, MAX_PATH_SIZE);

		}
		else
		{
			printf_s("somthing is very wrong\n");
			return FAILURE;
		}
	}
	return SUCCESS;
}

size_t find_char(char* str, char val, size_t max_size) {
	for (size_t i = 0; i < max_size && str[i] != '\0'; i++) {
		if (str[i] == val)
			return i;
	}
	return max_size;
}


/*
* callers resposibilty to call the function with real parameters undefined when idx > strlen(cpStr)
*/
BOOL RemoveCharFromString(char* cpStr, size_t idx, size_t stMaxLen) {
	char* start = cpStr + idx;
	size_t i = 0;
	while (1) {
		if (i == stMaxLen)
			return FAILURE;
		if (*start == '\0')
			return SUCCESS;
		*start = *(start + 1);
		i++;
		start++;
	}



}
