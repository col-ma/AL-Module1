#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include"main.h"
#include<fileapi.h>
#include<ctype.h>
#include<processenv.h>
#include<string.h>


static const char* EVIL = "C:\\evilevilevil\\";


/*
NEED TO DEAL WITH THE RUNTIME WHEN THE PROGRAM ISNT RUNNING =


*/
int main() {



	while (1)
	{
		printf_s("===============================STARTING PROGRAM===========================\n\n\n");
		RunProgram();
	}
}

DWORD RunProgram() {
	/*
	TODO
	STAGE I
	get filename from user using something
	*
	*/
	char cpPath[MAX_PATH_SIZE] = { 0 };
	char cpNewpath[MAX_PATH_SIZE] = { 0 };



	/* take the inputted path and normalize it to some degree 
	maily standarizes backslashes and forward slashes*/
	if (getFilePath(cpPath, cpNewpath) == FAILURE) {
		printf_s("error while getting path\n");
		return FAILURE;
	}






	/*
	* check if the fle has an executable extension basically useless as there a endless ways around it 
	* but ive implemented it so why not
	*/
	if (ResolveSymbolics(cpNewpath) == FAILURE) {
		printf_s("failed while resolving symbolics\n");
		return FAILURE;
	}
	
	BOOL bIsExecutable = IsExecutableExtension(cpNewpath);
	if (bIsExecutable == TRUE) {
		printf_s("executable extension\n");
		return FAILURE;
	}
	else if (bIsExecutable == FAILURE) {
		printf_s("IsExecutable Failure\n");
		return FAILURE;
	}
	


	//return 0;
	/*
	STAGE II
	in place filter file
	by no access to directory
	(normalize and compare to the non alowed path)
	*/
	char cpTempForFilter[MAX_PATH_SIZE] = { 0 };
	
	errno_t e = 0;
	if ((e = memcpy_s(cpTempForFilter, MAX_PATH_SIZE, cpNewpath, MAX_PATH_SIZE)) != 0) {
		printf_s("copy failed with error code %d\n", e);
		return FAILURE;
	}

	/*
	* check if thefile specified falls under C:\evilevilevil the way it does it is by 
	* comparing the handles of all of the directorys on the way 
	*/
	BOOL bIsEvil = IsEvilRequest(cpTempForFilter);
	if (bIsEvil == TRUE) {
		printf_s("evil reuquest\n");
		return FAILURE;
	}
	else if (bIsEvil == FAILURE) {
		printf_s("an error happend path probably invalid\n");
		return FAILURE;
	}


	/*
	STAGE I
	check if everything on the way exists
	if not create if it already exists ask if it wants to delete it
	*/
	/*
	* creates the actual file deals with all of the cases such as the file exists folders dont
	* might have a few more test cases (a folder with that name exists already?)
	*/
	int error_code = 0;
	HANDLE hFile = CeateFileProcess(cpNewpath, &error_code);
	if (hFile == NULL) {
		// for the case the file exists already
		if (error_code == RECURSE) {
			return SUCCESS;
		}
		// if something failed
		else if (error_code == FAILURE)
		{
			return FAILURE;
		}
		// something else that i cant really predict happend
		else
		{
			return FAILURE;
		}
	}

	/*
	STAGE I
	read a line from the stdin and
	pass it to the file
	*/
	/*
	* write from stdin to the file 
	* also tests that its not an executable file that being written
	*/
	DWORD dwStatusCode = WritetoFile(hFile);
	if (hFile == 0 || CloseHandle(hFile) == 0) {
		printf_s("CloseHandle failed with error code %d\n", GetLastError());
		return FAILURE;
	}
	hFile = NULL;

	if (dwStatusCode == FAILURE) {
		printf_s("an error  occurred while writing to the file aborting");
		return FAILURE;
	}
	else if(dwStatusCode == FALSE)
	{

		printf_s("the file started with executable magic\n");
		return FAILURE;

	}
	/*
	STAGE I
	when done
	close the file reopen it
	and print it to the console
	*/
	// read the content of the file again and print it to the screen
	if (ReadFromFile(cpNewpath) == FAILURE)
		return FAILURE;

	return 0;
}



int getFilePath(char* cpPath, char* cpNewPath) {
	printf_s("enter path: \n");
	fflush(stdin);
	if (fgets(cpPath, MAX_PATH_SIZE, stdin) == NULL) {
		printf_s("nothing has been read EOF encountered\n");
		return FAILURE;
	}
	// when using fgets it reads a \n at the end so replace it with a null terminator
	for (int i = 0; (i < MAX_PATH_SIZE) && (cpPath[i] != 0); i++) {
		if (cpPath[i] == 10) // \n
			cpPath[i] = 0;
	}
	// normalize the path to some extent mainly just changing eveything to the correct slash size and resolving . and ..
	if (ExpandEnvironmentStringsA(cpPath, cpNewPath, MAX_PATH_SIZE) == 0)
		return FAILURE;
	MEMCPY_S(cpPath, MAX_PATH_SIZE, cpNewPath, MAX_PATH_SIZE);
	memset(cpNewPath, 0, MAX_PATH_SIZE);

	if (ParseComplexExpresions(cpPath) == FAILURE)
		return FAILURE;

	getActualPath(cpPath, cpNewPath);

	if (FindAndResolveWildCards(cpNewPath) == FAILURE) {
		printf_s("attempt to reslve wildcards failed\n");
		return FAILURE;
	}

	printf_s("the path was %s\n", cpNewPath);

	return SUCCESS;
}


DWORD WritetoFile(HANDLE hFile) {
	printf_s("please input file content\n");
	DWORD dwBytesWrittenToFile = 0;
	BOOL bFirst = TRUE;
	char content[INITIAL_SIZE] = { 0 };
	/*
	* get the input line from stdin at chunks of 1024 at a time
	*/
	while (1)
	{
		fgets(content, INITIAL_SIZE, stdin);
		if (bFirst == TRUE) {
			bFirst = FALSE;
			// at the first iterration check if the file is an executable by 
			// comparing the start to the windows executable magic
			if (IsExecutableHeader(content))
				return FALSE;
		}
		/*
		STAGE III
		make sure the file written isnt an executable
		(magic and ending)
		*/
		DWORD dwContentBytes = (DWORD) strlen(content);
		if (WriteFile(hFile, content, dwContentBytes, &dwBytesWrittenToFile, NULL) == FALSE) {
			printf_s("Failed to Write string to file error code %d\n", GetLastError());
			return FAILURE;
		}
		if (content[dwContentBytes - 1] == '\n' || dwContentBytes == 1)
			break;
		memset(content, 0, INITIAL_SIZE);
	}
	return SUCCESS;
}


/*
* reads and prints the content of a file to the screen
*/
DWORD ReadFromFile(char* cpPath) {
	printf_s("the inputed file was: \n");
	DWORD dwlastread = 0;
	char buffer[INITIAL_SIZE] = { 0 };

	// at this stage it shouldnt fail as i have created the file above
	HANDLE hFile = CreateFileA(cpPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf_s("Major error aborting\n");
		return FAILURE;
	}
	// each time read a constant buffer amount from the buffer and print it to the screen
	// read file automatically tracks the pointer within the file
	while (1) {
		if (ReadFile(hFile, buffer, INITIAL_SIZE-1, &dwlastread, NULL) == FALSE) {
			printf_s("an error occurred error code %d", GetLastError());
			if (CloseHandle(hFile) == 0)
				printf_s("close handle has failed with error code numerb %d\n", GetLastError());
			return FAILURE;
		};
		printf_s("%s", buffer);
		if (dwlastread != (INITIAL_SIZE - 1))
			break;
		// reset buffer and counter before next iteration
		memset(buffer, 0, INITIAL_SIZE);
		dwlastread = 0;
	}


	if (CloseHandle(hFile) == 0) {
		printf_s("close handle has failed with error code number %d\n", GetLastError());
		return FAILURE;
	};
	return SUCCESS;
}


/*
* create the file for the path given
* if the file exists ask user if to overwrite
* if there are floders in the path that dont exist create them
* create the file
*/
HANDLE CeateFileProcess(char* cpPath, int* error_code) {
	printf_s("creating file %s\n", cpPath);
	// first attempt at creating the file will only succeed if all sub directorys exist and the file doesnt
	HANDLE hFile = CreateFileA(cpPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hFile == INVALID_HANDLE_VALUE) {
		printf_s("an error has  occurred while opening file\n");
		DWORD dwErrorCode = GetLastError();
		// handles the case where the file already exists asks user if to overwrite it 
		if (dwErrorCode == ERROR_FILE_EXISTS) {
			printf_s("file already exists\n");
			char choice = 0;
			char newline = 0;
			while (choice != 'y' && choice != 'n')
			{
				printf_s("do you want to delete the current content?(y/n)");
				choice = getchar();
				newline = getchar(); // clear the newline char as it only passes the input once a newline is entered
			}
			if (choice == 'y') {
				// if an overwrite was requested
				hFile = CreateFileA(cpPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile == INVALID_HANDLE_VALUE) {
					*error_code = FAILURE;
					return NULL;
			}
				return hFile;
			}
			else
			{
				*error_code = RECURSE;
				return NULL;
			}
		}
		// invalid file format just fail
		else if (dwErrorCode == ERROR_INVALID_NAME) {
			printf_s("file format invalid\n");
			*error_code = FAILURE;
			return NULL;
		}
		// the path doesnt exist meaning there are subdirectorys that need to be created
		else if (dwErrorCode == ERROR_PATH_NOT_FOUND) {
			printf_s("path doesnt exist\n");
			char temp[MAX_PATH_SIZE] = { 0 };
			char temp_no_name[MAX_PATH_SIZE] = { 0 };

			if (memcpy_s(temp, MAX_PATH_SIZE, cpPath, MAX_PATH_SIZE) != 0) {
				printf_s("memcpy failed\n");
				*error_code = FAILURE;
				return NULL;
			}
			RemoveLastPart(temp); //remove file name
			if (memcpy_s(temp_no_name, MAX_PATH_SIZE, temp, MAX_PATH_SIZE) != 0) {
				puts("memcpy failed");
				*error_code = FAILURE;
				return NULL;
			}

			if (CreateAllSubDirs(temp) == FAILURE) {
				*error_code = FAILURE;
				return NULL;
			}
			if (BuildDirs(temp, temp_no_name) == FAILURE) {
				*error_code = FAILURE;
				return NULL;
			}
			return CeateFileProcess(cpPath, error_code);
			/*
			Handle path creation create all existing sub directorys
			recall the createFileA call
			*/
		}
		else {
			printf_s("the error code was %u\n", dwErrorCode);
			*error_code = FAILURE;
			return NULL;
		}
		return hFile;
	};
	return hFile;
}



/*
go back on the file given until a known directory is reached 
then start reconstructing from there
*/
DWORD CreateAllSubDirs(char* cpPath) {
	while (strlen(cpPath) > 2)
	{
		DWORD error = CreateDirectoryA(cpPath, NULL);
		if (error == 0) {
			DWORD dwErrorCode = GetLastError();
			if (dwErrorCode == ERROR_PATH_NOT_FOUND) {
				RemoveLastPart(cpPath);
				continue;
			}
			else if (dwErrorCode == ERROR_ALREADY_EXISTS) {
				printf_s("A file with the folder name exists aborting\n");
				return FAILURE;
			}
			else {
				printf_s("Major error aborting error code %u\n", dwErrorCode);
				return FAILURE;
			}

		}
		if (IsEvilRequest(cpPath)) {
			printf_s("evil request\n");
			return FAILURE;
		}
		return SUCCESS;
	}
	printf_s("probably invalid path\n");
	return FAILURE;
}

/*
* given the first exesting directory and the goal tree 
* create all of the directorys on the way 
*/
DWORD BuildDirs(char* cpWorkingPath, char* cpFullPath) {
	cpWorkingPath[strlen(cpWorkingPath)] = '\\';
	while (strcmp(cpWorkingPath, cpFullPath) != 0 && (strlen(cpFullPath) > strlen(cpWorkingPath)))
	{
		AddBlock(cpWorkingPath, cpFullPath);
		if (CreateDirectoryA(cpWorkingPath, NULL) == 0) {
			printf("error encountered while creating dir\n");
			return FAILURE;
		}
		if (IsEvilRequest(cpWorkingPath)) {
			printf_s("blocked attempt o crate evilevilevil dir\n");
			return FAILURE;
		}
	}
	return SUCCESS;
}


// normalize path using the built in function mainly swaps all of the slashes 
// and resolves all of the relative references 
DWORD getActualPath(char* cpPath, char* cpNewPath) {
	GetFullPathNameA(cpPath, MAX_PATH_SIZE, cpNewPath, NULL);
	printf_s("long path name is:\n%s\n", cpNewPath);
	return 0;

}

// given a path remove the last segment
char* RemoveLastPart(char* cpPath) {
	for (int i = (int) strlen(cpPath); i >= 0; i--) {
		if (cpPath[i] != '\\' && cpPath[i] != ':') {
			cpPath[i] = '\0';
		}
		else
		{
			cpPath[i] = '\0';
			break;
		}
	}
	return cpPath;
}

// given a full path and a subpath add a segment
char* AddBlock(char* cpCurrent, char* cpFull) {
	for (size_t i = strlen(cpCurrent); i < strlen(cpFull); i++) {
		if (cpFull[i] == '\\' || cpFull[i] == '\0' || cpFull[i] == EOF) {
			cpCurrent[i] = cpFull[i];
			break;
		}
		cpCurrent[i] = cpFull[i];
	}
	return cpCurrent;
}



// given a path for every segment in the path check if its the evil segment 
// by comparing low level information
BOOL IsEvilRequest(char* cpPath) {
	while (1) {
		if (strlen(cpPath) < 2) {
			return FALSE;
		}

		BOOL bIsSamePath = IsSamePath(cpPath);
		if (bIsSamePath != FALSE) { // True or Failure
			return bIsSamePath;
		}
		RemoveLastPart(cpPath);
	}

}

// given two paths check if they point to the same place by 
// comparing the information about the directories they point to 
// might be able to make it always compare to EVIL withour asking for the inforation each time
// but it would make the code less generic
// i did consider using these functions for another purpose at some stage
BOOL IsSamePath(char* cpPath2) {
	HANDLE hFileEvil = CreateFileA(EVIL, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	errno_t e = GetLastError();
	HANDLE hFile2 = CreateFileA(cpPath2, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS  | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	char a[MAX_PATH_SIZE] = { 0 };

	if (hFile2 == INVALID_HANDLE_VALUE) {
		DWORD dwErrorCode = GetLastError();
		CLOSE_HANDLE(hFileEvil);
		if (dwErrorCode == ERROR_PATH_NOT_FOUND || dwErrorCode == ERROR_FILE_NOT_FOUND || dwErrorCode == ERROR_BAD_PATHNAME || dwErrorCode == ERROR_INVALID_NAME) {
			return FALSE;
		}
		else
		{
			return FAILURE;
		}
	}

	if (hFileEvil == INVALID_HANDLE_VALUE) {
		CLOSE_HANDLE(hFile2);
		if (e == ERROR_PATH_NOT_FOUND || e == ERROR_FILE_NOT_FOUND || e == ERROR_BAD_PATHNAME || e == ERROR_INVALID_NAME) {
			return FALSE;
		}
		else
		{
			return FAILURE;
		}
	}

	return IsSameHandle(hFile2, hFileEvil);


}

// given two handles check if they are referncing the same entity 
// by comparing the disk information about them 
BOOL IsSameHandle(HANDLE hFile2, HANDLE hFileEvil) {
	BY_HANDLE_FILE_INFORMATION info2 = { 0 };
	BY_HANDLE_FILE_INFORMATION infoEvil = { 0 };

	BOOL res2 = GetFileInformationByHandle(hFile2, &info2);
	BOOL resEvil = GetFileInformationByHandle(hFileEvil, &infoEvil);
	
	if (res2 == 0) {
		CLOSE_HANDLE(hFile2);
		CLOSE_HANDLE(hFileEvil);
		DWORD dwErrorCode = GetLastError();
		if (dwErrorCode == ERROR_INVALID_FUNCTION)
			return FALSE;
		printf_s("error code %d\n", GetLastError());
		return FAILURE;
	}

	CLOSE_HANDLE(hFileEvil)
	CLOSE_HANDLE(hFile2);

	return (infoEvil.dwVolumeSerialNumber == info2.dwVolumeSerialNumber) && (infoEvil.nFileIndexHigh == info2.nFileIndexHigh) && (infoEvil.nFileIndexLow == info2.nFileIndexLow);
}



// check if a buffer contains a executable magic
// gets call with INITIAL_SIZE sied buffers
BOOL IsExecutableHeader(char* content) {
	if (memcmp(content, "MZ", 2) == 0) // PE 
		return TRUE;
	if (memcmp(content, "ZM", 2) == 0) // wierd alternative
		return TRUE;
	if (memcmp(content, "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8) == 0) // COM
		return TRUE;
	if (memcmp(content, "\x23\x40\x7E\x5E", 4) == 0) // VBE
		return TRUE;

	return FALSE;
}


/*
* this might have its value something doesnt entirely make sense because looks like the way windows works (while running from the cmd) is:
* I look at the magic if its MZ run it (or at least try)
* II if it isnt MZ look at the extension and decide what to do with it 
* III if there a known extension it is mapped to do that 
* IV if there is nothing known query the user for an app 
* 
* so these functions might help in stage IV because the env path 
* supposidly hold all of the non PEs it knows how to execute 
* 
* 
* useless but understood that one too late as anyfile can be executed if the full path is provided extensions
* just help for shortcuts so either i whitelist extensions which is a impossible and b not what i was required to do or ill focus on the content itself
*/
BOOL IsExecutableExtension(char* cpPath) {
	char* cpExecutableExtensions = (char*)malloc(MAX_PATH_SIZE + 1); // +1 to account for the extra ';' added at the end; 
	if (cpExecutableExtensions == NULL) {
		printf_s("malloc failure\n");
		return FAILURE;
	}


	char* cpExtension = NULL;
	// problem with this is that the EnvironmentVariable can be modyfied but the 
	// alternative is hard coding and im not sure what is less secure maybe worth doing both
	// this is the default value: .COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC
	if (GetEnvironmentVariableA("PATHEXT", cpExecutableExtensions, MAX_PATH_SIZE + 1) == 0) {
		printf_s("failure while trying to access environment variable error code %d\n", GetLastError());
		free(cpExecutableExtensions);
		return FAILURE;
	}


	// according to the windows documentation the max length for this variable is the same as the max string length so the buffer cant be full 
	cpExecutableExtensions[strlen(cpExecutableExtensions)] = ';';
	cpExecutableExtensions[strlen(cpExecutableExtensions) + 1] = '\0';


	cpExtension = GetPointerToExtension(cpPath);
	if (cpExtension == NULL) {
		return FALSE;
	}

	cpExtension = ToSearchableFormat(cpExtension);
	if (cpExtension == NULL) { 
		return FAILURE;
	}

	char *cpContainsExtension = strstr(cpExecutableExtensions, cpExtension);

	free(cpExtension);
	free(cpExecutableExtensions);

	return cpContainsExtension == NULL ? FALSE : TRUE;
}



char* GetPointerToExtension(char* cpPath) {
	for (int i = (int) strlen(cpPath); i >= 0; i--) {
		if (cpPath[i] == '.')
			return &cpPath[i];
		if (cpPath[i] == '\\')
			return NULL;
	}
	return NULL;
}


char* ToSearchableFormat(char* str) {
	size_t stNewLen = strlen(str) + 2; // one for null terminator and another of the ';'
	char* cpExtension = (char*)malloc(stNewLen * sizeof(char));
	if (cpExtension == NULL) {
		printf_s("malloc failure\n");
		return NULL;
	}
	memset(cpExtension, 0, stNewLen);
	if (memcpy_s(cpExtension, stNewLen, str, strlen(str)) != 0) {
		printf_s("error in memcpy\n");
		free(cpExtension);
		return NULL;
	}
	cpExtension[strlen(str)] = ';';
	cpExtension[strlen(str) + 1] = '\0';
	ToUpper(cpExtension, stNewLen);
	return cpExtension;
}


void ToUpper(char* str, size_t stMax) {
	size_t i = 0;
	while (*str && (i < stMax)) {
		*str = toupper((unsigned char)*str);
		str++;
		i++;
	}
}


BOOL ResolveSymbolics(char* cpPath) {
	char local[MAX_PATH_SIZE] = { 0 };
	char local2[MAX_PATH_SIZE] = { 0 };
	char local3[MAX_PATH_SIZE] = { 0 };
	BOOL bRes = FALSE;


	MEMCPY_S(local, MAX_PATH_SIZE, cpPath, MAX_PATH_SIZE);
	MEMCPY_S(local3, MAX_PATH_SIZE, cpPath, MAX_PATH_SIZE);


	while (1) {
		bRes = GetFirstSymbolic(local);
		if (bRes == FALSE) // local is up to first symbolic
			break;
		else if (bRes == FAILURE)
			return FAILURE;

		// HANDLE hFile1 = CreateFileA(local, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
		
		
		//local2 holds the path to the symbolic
		//GetFinalPathNameByHandleA(hFile1, local2, MAX_PATH_SIZE, FILE_NAME_NORMALIZED);
		if (GetCBasedDrive(local, local2) == FAILURE)
			return FAILURE;
		//local3 holds the path that was started with
		if (strcat_s(local2, MAX_PATH_SIZE, local3 + strlen(local)) != 0)
			return FAILURE;

		memset(local, 0, MAX_PATH_SIZE);
		memset(local3, 0, MAX_PATH_SIZE);
		MEMCPY_S(local, MAX_PATH_SIZE, local2, MAX_PATH_SIZE);
		memset(local2, 0, MAX_PATH_SIZE);
		MEMCPY_S(local3, MAX_PATH_SIZE, local, MAX_PATH_SIZE);


	}


	printf_s("absolute path is %s\n", local3);

	memset(cpPath, 0, MAX_PATH_SIZE);
	MEMCPY_S(cpPath, MAX_PATH_SIZE, local3, MAX_PATH_SIZE);

	return SUCCESS;
}



BOOL GetFirstSymbolic(char* cpPath) {
	char cpFirstSymbolic[MAX_PATH_SIZE] = { 0 };
	BOOL bChange = FALSE;
	BOOL exit = FALSE;
	while (1) {
		if (IsSymbolic(cpPath) == TRUE)
		{
			bChange = TRUE;
			memset(cpFirstSymbolic, 0, MAX_PATH_SIZE);
			MEMCPY_S(cpFirstSymbolic, MAX_PATH_SIZE, cpPath, MAX_PATH_SIZE);
		}
		RemoveLastPart(cpPath);
		if (strlen(cpPath) == 2 && toupper(cpPath[0]) == 'C') {
			cpPath[2] = '\\';
			cpPath[3] = '\0';
			exit = TRUE;
		}
		if ((strlen(cpPath)  <= 1 || exit)/* && (!HasValidCharacters(cpPath, MAX_PATH_SIZE))*/)
			break;
	}

	memset(cpPath, 0, MAX_PATH_SIZE);
	MEMCPY_S(cpPath, MAX_PATH_SIZE, cpFirstSymbolic, MAX_PATH_SIZE);
	return bChange;
}


BOOL IsSymbolic(char* cpPath) {
	char cpRealPath[MAX_PATH_SIZE] = { 0 };
	BOOL bIsSymbolic = TRUE;
	HANDLE hFile2 = CreateFileA(cpPath, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile2 == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	//if (GetFinalPathNameByHandleA(hFile2, cpRealPath, MAX_PATH_SIZE, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS) != 0) {
	if (GetCBasedDrive(cpPath, cpRealPath) == SUCCESS) {
		if (strcmp(cpRealPath, cpPath) == 0)
			bIsSymbolic = FALSE;
	};


	DWORD dwAttr = GetFileAttributesA(cpPath);
	if (dwAttr == INVALID_FILE_ATTRIBUTES) {
		if (CloseHandle(hFile2) == 0)
			return FAILURE;
		return FALSE;
	}
	if (CloseHandle(hFile2) == 0) {
		return FAILURE;
	}
	return (bIsSymbolic || (FILE_ATTRIBUTE_REPARSE_POINT & dwAttr));
}



BOOL HasValidCharacters(char* str, size_t stMax) {
	char NonValid[] = "<>:\"/\\|?*";
	BOOL bValid = FALSE;
	size_t i = 0;
	while (*str && (i < stMax)) {
		BOOL current = TRUE;
		for (int j = 0; j < strlen(NonValid); j++) {
			if (NonValid[j] == *str)
				current = FALSE;
		}
		bValid = (bValid || current);
		str++;
		i++;
	}
	return bValid;
}




BOOL GetCBasedDrive(char* cpPathIn, char* cpPathOut) {


	FILE_ID_INFO c = { 0 };
	FILE_ID_DESCRIPTOR d = { 0 };

	HANDLE hFile1 = CreateFileA(cpPathIn, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile1 == INVALID_HANDLE_VALUE) {
		DWORD dwErrorCode = GetLastError();
		if (dwErrorCode == ERROR_PATH_NOT_FOUND || dwErrorCode == ERROR_FILE_NOT_FOUND || dwErrorCode == ERROR_BAD_PATHNAME || dwErrorCode == ERROR_INVALID_NAME) {
			return FAILURE;
		}
		else
		{
			return FAILURE;
		}
	}
	
	
	
	if (GetFileInformationByHandleEx(hFile1, FileIdInfo, &c, sizeof(FILE_ID_INFO)) == 0) {
		DWORD dwLatError = GetLastError();
		CLOSE_HANDLE(hFile1);
		if (dwLatError == ERROR_INVALID_FUNCTION)
			return FALSE;
		printf_s("error %d\n", GetLastError());
		return FALSE;
	}
	HANDLE hBaseHandle = CreateFileA("C:\\", 0, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	

	if (CloseHandle(hFile1) == 0) {
		CLOSE_HANDLE(hBaseHandle);
		return FAILURE;
	}

	if (hBaseHandle == INVALID_HANDLE_VALUE) {
		printf_s("couldnt get base handle\n");
		return FAILURE;
	}

	d.dwSize = sizeof(d);
	d.Type = ExtendedFileIdType;
	d.ExtendedFileId = c.FileId;
	
	HANDLE h = OpenFileById(hBaseHandle, &d, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, FILE_FLAG_BACKUP_SEMANTICS);
	if (h == INVALID_HANDLE_VALUE) {
		DWORD dwLastError = GetLastError();
		if (dwLastError == ERROR_NOT_SUPPORTED)
		{
			CLOSE_HANDLE(hBaseHandle);
			printf_s("file not on C drive\n");
			memset(cpPathOut, 0, MAX_PATH_SIZE);
			MEMCPY_S(cpPathOut, MAX_PATH_SIZE, cpPathIn, MAX_PATH_SIZE);
			return SUCCESS;	
		}
		printf_s("error was %d\n", dwLastError);
		return FAILURE;
	}

	char e[MAX_PATH_SIZE] = { 0 };
	if (GetFinalPathNameByHandleA(h, e, MAX_PATH_SIZE, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS) == 0) {
		printf_s("error while retrieving path\n");
		CLOSE_HANDLE(h);
		CLOSE_HANDLE(hBaseHandle);
		return FAILURE;
	}

	if (CloseHandle(h) == 0) {
		printf_s("failed to close handle\n");
		CLOSE_HANDLE(hBaseHandle);
		return FAILURE;
	}
	CLOSE_HANDLE(hBaseHandle);
	memset(cpPathOut, 0, MAX_PATH_SIZE);
	MEMCPY_S(cpPathOut, MAX_PATH_SIZE, e, MAX_PATH_SIZE);
	
	return SUCCESS;
}



BOOL FindAndResolveWildCards(char* path){
	size_t path_len = 0;
	int offset = 0;
	SEC_LEN(path, MAX_PATH_SIZE, path_len);
	BOOL bHasWindcards = FALSE;
	for (size_t i = 0; i < strlen(path); i++) {
		char h = path[i];
		if (path[i] == '*' || path[i] == '?') {
			if (i == 2 && (strncmp("\\\\?", path, 3)) == 0)
				continue;
			if (ResolveWildCard(path, i, &offset, path_len) == FAILURE)
				return FAILURE;
			SEC_LEN(path, MAX_PATH_SIZE, path_len);
			i += offset;
		}
	}
	return SUCCESS;
}


BOOL ResolveWildCard(char* cpPath, size_t idx, int* offset, size_t path_len) {
	size_t backslashIdx = -1;
	for (size_t i = idx; i < path_len; i++) {
		if (cpPath[i] == '\\') {
			backslashIdx = i;
			break;
		}
	}
	if (backslashIdx == -1) {
		printf_s("name cannot contain wildcard\n");
		return FAILURE;
	}
	cpPath[backslashIdx] = '\0';
	char temp[MAX_PATH_SIZE] = { 0 };



	WIN32_FIND_DATAA fdData = { 0 };
	HANDLE next = FindFirstFileA(cpPath, &fdData);
	if (next == INVALID_HANDLE_VALUE) {
		printf_s("could not resolve wildcard invalid\n");
		return FAILURE;
	}
	MEMCPY_S(temp, MAX_PATH_SIZE, fdData.cFileName, MAX_PATH);
	

	if (FindNextFileA(next, &fdData) != 0) {
		printf_s("ambiguous entry aborting\n");
		return FAILURE;
	}

	if (FindClose(next) == 0) {
		printf_s("failed while resolving wildcards with error code %d\n", GetLastError());
		return FAILURE;
	}

	char temp2[MAX_PATH_SIZE] = { 0 };
	
	MEMCPY_S(temp2, MAX_PATH_SIZE, cpPath, MAX_PATH_SIZE);
	IS_NULL_TERMINATED(temp2, MAX_PATH_SIZE);
	RemoveLastPart(temp2);
	temp2[strlen(temp2)] = '\\';
	if (strcat_s(temp2, MAX_PATH_SIZE, temp) != 0) {
		printf_s("strcat failed\n");
		return FAILURE;
	}

	cpPath[backslashIdx] = '\\';
	if (strcat_s(temp2, MAX_PATH_SIZE, cpPath + backslashIdx) != 0) {
		printf_s("strcat failed\n");
		return FAILURE;
	}
	memset(cpPath, 0, MAX_PATH_SIZE);
	MEMCPY_S(cpPath, MAX_PATH_SIZE, temp2, MAX_PATH_SIZE);
	size_t newlen = 0;
	SEC_LEN(cpPath, MAX_PATH_SIZE, newlen);

	*offset = (int) (newlen - path_len);

	return SUCCESS;
}

