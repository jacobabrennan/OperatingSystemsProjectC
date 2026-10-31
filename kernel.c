

/*== Operating Systems Project C =============================================*/

/*-- Attribution -------------------------------*/
/*
    All work by Jacob A Brennan
    Based on assignments by Dr. Margaret Black
*/

/*-- Dependencies ------------------------------*/
#include "./kernel.h"
#include "./terminal.h"

/*-- Constants ---------------------------------*/
/* Messages */
#define MESSAGE_ERROR "Error: Malformed Interrupt\r\n"
/* Hard Drive magic numbers */
#define HD_DEVICE_NUMBER 0x80
#define HD_MODE_READ 2
#define HD_SECTOR_SIZE 512
/* Interrupts */
#define INTERRUPT_BIOS_IO 0x10
#define INTERRUPT_BIOS_READ_SECTOR 0x13
#define INTERRUPT_BIOS_READ_KEY 0x16
#define INTERRUPT_BIOS_PRINT_CHARACTER 0xE
#define INTERRUPT_SYSCALL 0x21
/* File System */
#define FILE_SYSTEM_SECTOR_BOOT 0
#define FILE_SYSTEM_SECTOR_MAP 1
#define FILE_SYSTEM_SECTOR_DIRECTORY 2
#define FILE_SYSTEM_SECTOR_KERNEL 3
#define FILE_SYSTEM_SECTOR_TEST 30
#define FILE_SYSTEM_DIRECTORY_ENTRY_SIZE 32
#define FILE_SYSTEM_FILE_NAME_SIZE 6
/*#define FILE_SYSTEM_FILE_SIZE_MAX (26*(HD_SECTOR_SIZE))*/
#define FILE_SYSTEM_DIRECTORY_MAX_ENTRIES ((HD_SECTOR_SIZE)/(FILE_SYSTEM_DIRECTORY_ENTRY_SIZE))
/* Note integer devision above. */
/* OS Misc. Magic Numbers and strings */
#define SEGMENT_PROGRAM_DESTINATION 0x2000
#define PROGRAM_NAME_SHELL "shell"

/* "Inline Functions" */
#define PACK_BYTES(H, L) (((H)<<8)+(L))
#define PRINT_CHAR(character) interrupt(INTERRUPT_BIOS_IO, PACK_BYTES(INTERRUPT_BIOS_PRINT_CHARACTER,(character)), 0, 0, 0)

/*-- Forward Declarations ----------------------*/
/* int interrupt (int number, int AX, int BX, int CX, int DX); */
/* Strings */
void printString(char *);
void readString(char *);
/* Hard Drive */
void readSector(char *, int);
/* File System */
int fileSystemNameCompare(char *, char*);
void readFile(char *, char *, int *);
/* Program Execution */
void executeProgram(char *);
void terminate();
/* Interrupts */
void handleInterrupt21(int, int, int, int);

/*-- Program Entry Point -----------------------*/
int main(void) {
	makeInterrupt21();
    interrupt(INTERRUPT_SYSCALL, SYSCALL_EXECUTE_PROGRAM, PROGRAM_NAME_SHELL, 0, 0);
    PRINT_CHAR('!');
    return 0;
}

/*-- Strings -----------------------------------*/
void printString(char *characters) {
    char letterIndex = 0;
    char letterValue = characters[letterIndex];
    while(ASCII_END_OF_STRING != letterValue) {
        /* Draw character at index */
        PRINT_CHAR(letterValue);
        /* Maintain Loop */
        letterIndex++;
        letterValue = characters[letterIndex];
    }
}
void readString(char *characters) {
    /*
        Characters is an array of at least 80 elements. How many? I don't know.
        The directions say to add line feed and null terminator as the "last
        two characters characters in the array". Without knowing the length of
        the array, this isn't possible. Instead, I'm interpreting this to mean
        "the last two characters that we add to the array at the current
        position".
        Note that this function will result in a buffer overflow if the user
        enters more than 77 characters.
    */
    int index = 0;
    int characterInput;
    while(1) {
        characterInput = interrupt(INTERRUPT_BIOS_READ_KEY, 0, 0, 0, 0);
        switch(characterInput) {
            case ASCII_BACKSPACE:
                if(0 >= index) { continue;}
                index--;
                PRINT_CHAR(ASCII_BACKSPACE);
                PRINT_CHAR(ASCII_SPACE);
                PRINT_CHAR(ASCII_BACKSPACE);
                break;
            case ASCII_ENTER:
                PRINT_CHAR(ASCII_ENTER);
                PRINT_CHAR(ASCII_LINE_FEED);
                characters[index] = ASCII_END_OF_STRING;
                /* The above line isn't what was specified in the assignment,
                    but the control characters make dealing with user input much
                    more complicated in the later assignments.
                characters[index+0] = ASCII_ENTER;
                characters[index+1] = ASCII_LINE_FEED;
                characters[index+2] = ASCII_END_OF_STRING;
                */
                return;
            default:
                characters[index] = characterInput;
                PRINT_CHAR(characterInput);
                index++;
                break;
        }
    }
}

/*-- Hard Drive --------------------------------*/
void readSector(char *buffer, int sector) {
    /* As per assignment:
        - use 0 for the track,
        - 0 for the head,
        - and sector+1 for the relative sector number.
        - for the hard disk, use 0x80
    */
    int ax = PACK_BYTES(HD_MODE_READ, 1);
    int bx = buffer;
    int cx = PACK_BYTES(0, sector+1);
    int dx = PACK_BYTES(0, HD_DEVICE_NUMBER);
    interrupt(INTERRUPT_BIOS_READ_SECTOR, ax, bx, cx, dx);
}

/*-- File System -------------------------------*/
int fileSystemNameCompare(char *fileNameA, char *fileNameB) {
    /* Handles max-length / null termination differently than stringCompare. */
    char letterA;
    char letterB;
    int index;
    for(index = 0; index < FILE_SYSTEM_FILE_NAME_SIZE; index++) {
        letterA = fileNameA[index];
        letterB = fileNameB[index];
        /* Return false if strings don't match at any point. */
        if(letterA != letterB) {
            return 0;
        }
        /* Return false if both strings contain null character at index 0. */
        if(ASCII_END_OF_STRING == letterA && ASCII_END_OF_STRING == letterB) {
            if(0 == index) { return 0;}
        /* Return true if both strings terminate early. */
            return 1;
        }
    }
    return 1;
}
void readFile(char *fileName, char *fileBuffer, int *sectorsRead) {
    char fileSystemDirectory[HD_SECTOR_SIZE];
    char *DirectoryEntry = 0;
    int index = 0;
    if(0 != sectorsRead) {
        *sectorsRead = 0;
    }
    /* 1. Load the directory sector into a 512 byte character array using readSector */
    readSector(&fileSystemDirectory, FILE_SYSTEM_SECTOR_DIRECTORY);
    /* 2. Go through the directory trying to match the file name. */
    for(index = 0; index < FILE_SYSTEM_DIRECTORY_MAX_ENTRIES; index++) {
        DirectoryEntry = fileSystemDirectory + index*FILE_SYSTEM_DIRECTORY_ENTRY_SIZE;
        if(!fileSystemNameCompare(fileName, DirectoryEntry)) {
            DirectoryEntry = 0;
            continue;
        }
        break;
    }
    /* 2+. If you do not find it, set the number of sectors read to 0 and return. */
    if(0 == DirectoryEntry) {
        return;
    }
    /* 3. Using the sector numbers in the directory, load the file, sector by sector, into the buffer array.
        You should add 512 to the buffer address every time you call readSector.
        Make sure to increment the number of sectors read as you go.
    */
    for(index = FILE_SYSTEM_FILE_NAME_SIZE; index < FILE_SYSTEM_DIRECTORY_ENTRY_SIZE; index++) {
        if(0 == DirectoryEntry[index]) {
            break;
        }
        readSector(fileBuffer+HD_SECTOR_SIZE*(index-FILE_SYSTEM_FILE_NAME_SIZE), DirectoryEntry[index]);
        if(0 != sectorsRead) {
            *sectorsRead = *sectorsRead+1;
        }
    }
}

/*-- Program Execution -------------------------*/
void executeProgram(char *name) {
    char program[FILE_SYSTEM_FILE_SIZE_MAX];
    int sectorsRead;
    int index;
    readFile(name, &program, &sectorsRead);
    if(0 == sectorsRead) { return;}
    for(index = 0; index < sectorsRead*HD_SECTOR_SIZE; index++) {
        putInMemory(SEGMENT_PROGRAM_DESTINATION, index, program[index]);
    }
    launchProgram(SEGMENT_PROGRAM_DESTINATION);
    /* Program will never return from above line. */
}
void terminate() {
    /* Not sure how to not hard code these values in the function itself. */
    char shellname[FILE_SYSTEM_FILE_NAME_SIZE];
    shellname[0] = 's';
    shellname[1] = 'h';
    shellname[2] = 'e';
    shellname[3] = 'l';
    shellname[4] = 'l';
    shellname[5] = ASCII_END_OF_STRING;
    executeProgram(shellname);
}

/*-- Interrupts --------------------------------*/
void handleInterrupt21(int ax, int bx, int cx, int dx) {
    switch(ax) {
        case SYSCALL_PRINT_STRING:
            printString((char *)bx);
            break;
        case SYSCALL_READ_STRING:
            readString((char *)bx);
            break;
        case SYSCALL_READ_SECTOR:
            readSector((char *)bx, cx);
            break;
        case SYSCALL_READ_FILE:
            readFile((char *)bx, (char *)cx, (int *)dx);
            break;
        case SYSCALL_EXECUTE_PROGRAM:
            executeProgram((char *)bx);
            break;
        case SYSCALL_TERMINATE:
            terminate();
            break;
        default:
            /* Print an error message */
            PRINT_CHAR(ASCII_ENTER);
            PRINT_CHAR(ASCII_LINE_FEED);
            printString(MESSAGE_ERROR);
            break;
    }
}
