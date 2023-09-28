#include <stdio.h>
#include <stdlib.h>
#include<Windows.h>
#include "main.h"


BOOL RunCmd(char* exp, char* output)
{
    FILE* fp;
    char response[MAX_PATH_SIZE] = { 0 };
    char templt[] = "echo:%s";
    char* cmd = (char*)malloc(strlen(templt) + strlen(exp) * 2);
    snprintf(cmd, strlen(templt) + strlen(exp) * 2, templt, exp);

    if (escapeEscapables(cmd, strlen(templt) + strlen(exp) * 2) == FAILURE) {
        free(cmd);
        return FAILURE;
    }


    memset(output, 0, MAX_PATH_SIZE);



    /* Open the command for reading. */
    fp = _popen(cmd, "r");
    if (fp == NULL) {
        free(cmd);
        return FAILURE;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(response, sizeof(response), fp) != NULL) {
        for (int i = 0; i < strlen(response); i++) {
            if (response[i] == '\n' && response[i + 1] == '\0')
                response[i] = '\0';
        }
        strcat_s(output, MAX_PATH_SIZE, response);
    }

    /* close */
    _pclose(fp);
    free(cmd);
    return SUCCESS;
}


BOOL IsEscapable(char a) {
    char escapabales[] = "&\\<>^|()@!;'`!\"";
    for (int i = 0; i < strlen(escapabales); i++) {
        if (a == escapabales[i])
            return TRUE;
    }
    return FALSE;
}


BOOL escapeEscapables(char* cmd, size_t sz) {
    for (int i = 0; i < strlen(cmd); i++) {
        if (IsEscapable(cmd[i])) {
            if (addEscape(cmd, i, sz) == FAILURE)
                return FAILURE;
            i++;
        }
    }
    return TRUE;
}


BOOL addEscape(char* cmd, int i, size_t sz) {
    char* temp = (char*)malloc(sz);
    if (temp == NULL)
        return FAILURE;
    char bad = cmd[i];
    cmd[i] = '\0';
    memset(temp, 0, sz);
    if (strcat_s(temp, sz, cmd) != 0) {
        free(temp);
        return FAILURE;
    }

    temp[i] = '^';
    temp[i + 1] = '\0';
    cmd[i] = bad;
    if (strcat_s(temp, sz, cmd + i) != 0) {
        free(temp);
        return FAILURE;
    }
    memset(cmd, 0, sz);
    if (memcpy_s(cmd, sz, temp, sz) != 0) {
        free(temp);
        return FAILURE;
    }
    free(temp);
    return SUCCESS;
}
