

/*== Kernel ==================================================================*/

/*-- Attribution -------------------------------*/
/*
    All work by Jacob A Brennan
    Based on assignments by Dr. Margaret Black
*/

/*-- Include Guard (open) ----------------------*/
#ifndef OS_KERNEL
#define OS_KERNEL

/*-- Constants ---------------------------------*/
/* System Calls */
#define SYSCALL_PRINT_STRING 0
#define SYSCALL_READ_STRING 1
#define SYSCALL_READ_SECTOR 2
#define SYSCALL_READ_FILE 3
#define SYSCALL_EXECUTE_PROGRAM 4
#define SYSCALL_TERMINATE 5
/* Quirks of the kernel that have to be known to the rest of the system. */
#define FILE_SYSTEM_FILE_SIZE_MAX (26*(512))
/* The above is the max sectors per file times bytes per sector. */

/*-- Include Guard (close) ---------------------*/
#endif
