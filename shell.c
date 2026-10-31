

/*== Shell ===================================================================*/

/*-- Attribution -------------------------------*/
/*
    All work by Jacob A Brennan
    Based on assignments by Dr. Margaret Black
*/

/*-- Dependencies ------------------------------*/
#include "./terminal.h"
#include "./kernel.h"

/*-- Constants ---------------------------------*/
/* Shell commands (and misc) */
#define SHELL_COMMAND_TYPE "type"
#define SHELL_COMMAND_EXEC "exec"
#define SHELL_PROMPT "AHOY>"
/* Errors */
#define ERROR_MESSAGE_COMMAND_NOT_FOUND "Error. Unrecognized command: "
#define ERROR_MESSAGE_FILE_NAME_MISSING "Error. Missing File Name."
#define ERROR_MESSAGE_FILE_NOT_FOUND "Error. File not found: "
#define ERROR_MESSAGE_MALFORMED_PROGRAM "Error. Executed malformed program."

/*-- Forward Declarations ----------------------*/
int stringCompare(char *, char *);
void commandType(char *);
void commandExec(char *);

/*-- -------------------------------------------*/
int main(void) {
    char commandBuffer[TERMINAL_LINE_LENGTH];
    char commandParam1[TERMINAL_LINE_LENGTH];
    int index;
    /* Note: There is no guarantee the user's input will fit in an 80 character
        buffer, especially due to the addition of control characters at the end.
        As security and robustness are not the focus of this project, I will not
        be addressing these edge cases.
    */
    while(1) {
        for(index=0; index < TERMINAL_LINE_LENGTH; index++) {
            commandBuffer[index] = 0;
        }
        syscall(SYSCALL_PRINT_STRING, SHELL_PROMPT, 0, 0);
        syscall(SYSCALL_READ_STRING, commandBuffer, 0, 0);
        for(index=0; index < TERMINAL_LINE_LENGTH; index++) {
            if(ASCII_END_OF_STRING == commandBuffer[index] || ASCII_SPACE == commandBuffer[index]) {
                commandParam1[index] = ASCII_END_OF_STRING;
                index++; /* There's a space after param1, advance index to param2 */
                /* (If it isn't a space, then it's an end of string, and all the
                    rest are end of string, also.)*/
                break;
            }
            commandParam1[index] = commandBuffer[index];
        }
        if(stringCompare(SHELL_COMMAND_TYPE, commandParam1)) {
            commandType(commandBuffer+index);
        } else if(stringCompare(SHELL_COMMAND_EXEC, commandParam1)) {
            commandExec(commandBuffer+index);
        } else {
            syscall(SYSCALL_PRINT_STRING, ERROR_MESSAGE_COMMAND_NOT_FOUND, 0, 0);
            syscall(SYSCALL_PRINT_STRING, commandParam1, 0, 0);
            syscall(SYSCALL_PRINT_STRING, "\r\n", 0, 0);
        }
    }
}
int stringCompare(char *stringA, char *stringB) {
    /*
    Let's just ignore all the red flags that come with rolling your own
        string compare function...
    */
    char letterA;
    char letterB;
    int index = 0;
    while(1) {
        letterA = stringA[index];
        letterB = stringB[index];
        if(letterA != letterB) {
            return 0;
        }
        if(ASCII_END_OF_STRING == letterA || ASCII_END_OF_STRING == letterB) {
            return 1;
        }
        index++;
    }
}

/*-- Commands ----------------------------------*/
/*
    These would normally be implemented as separate programs, correct? I guess
    it makes more sense to do them as functions in this assignment, as there is
    a limited number of file slots available, and compiling multiple files would
    more complicated than necessary.
*/
void commandType(char *params) {
    char fileData[FILE_SYSTEM_FILE_SIZE_MAX];
    int sectorsRead = 0;
    if(ASCII_END_OF_STRING == params[0]) {
        syscall(SYSCALL_PRINT_STRING, ERROR_MESSAGE_FILE_NAME_MISSING, 0, 0);
        syscall(SYSCALL_PRINT_STRING, "\r\n");
        return;
    }
    syscall(SYSCALL_READ_FILE, params, fileData, &sectorsRead);
    if(0 == sectorsRead) {
        syscall(SYSCALL_PRINT_STRING, ERROR_MESSAGE_FILE_NOT_FOUND, 0, 0);
        syscall(SYSCALL_PRINT_STRING, params, 0, 0);
        syscall(SYSCALL_PRINT_STRING, "\r\n");
        return;
    }
    syscall(SYSCALL_PRINT_STRING, fileData, 0, 0);
    syscall(SYSCALL_PRINT_STRING, "\r\n");
}
void commandExec(char *params) {
    if(ASCII_END_OF_STRING == params[0]) {
        syscall(SYSCALL_PRINT_STRING, ERROR_MESSAGE_FILE_NAME_MISSING, 0, 0);
        syscall(SYSCALL_PRINT_STRING, "\r\n");
        return;
    }
    syscall(SYSCALL_EXECUTE_PROGRAM, params, 0, 0);
    /* If the program executes, the rest of the function will not be run. */
    syscall(SYSCALL_PRINT_STRING, ERROR_MESSAGE_FILE_NOT_FOUND, 0, 0);
    syscall(SYSCALL_PRINT_STRING, params, 0, 0);
    syscall(SYSCALL_PRINT_STRING, "\r\n");
}

/* I ran into a very hard to track down bug while writing this, and spent
several hours bug hunting. In the process this code became very bad. Bad.
With apologies for how disorganized this is.*/
