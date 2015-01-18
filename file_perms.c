// Name: Nikolay Goncharenko
// Email: goncharn@onid.oregonstate.edu
// Class: CS344-400
// Assignment: homework#3 Problem3

//!!! File is copied from the tlpi, Listing 15-4, p.296

#define _XOPEN_SOURCE 500
#include <sys/stat.h>
#include <stdio.h>
/* Interface for this implementation */
#include "file_perms.h"
#define STR_SIZE sizeof("rwxrwxrwx")
#define FP_SPECIAL 0
#ifndef FILE_PERMS_H
#define FILE_PERMS_H 
#endif

/* Return ls(1)-style string for file permissions mask */
char * filePermStr(mode_t perm, int flags)
{
    static char str[STR_SIZE];
    snprintf(str, STR_SIZE, "%c%c%c%c%c%c%c%c%c",
    (perm & S_IRUSR) ? 'r' : '-', (perm & S_IWUSR) ? 'w' : '-',
    (perm & S_IXUSR) ?
    (((perm & S_ISUID) && (flags & FP_SPECIAL)) ? 's' : 'x') :
    (((perm & S_ISUID) && (flags & FP_SPECIAL)) ? 'S' : '-'),
    (perm & S_IRGRP) ? 'r' : '-', (perm & S_IWGRP) ? 'w' : '-',
    (perm & S_IXGRP) ?
    (((perm & S_ISGID) && (flags & FP_SPECIAL)) ? 's' : 'x') :
    (((perm & S_ISGID) && (flags & FP_SPECIAL)) ? 'S' : '-'),
    (perm & S_IROTH) ? 'r' : '-', (perm & S_IWOTH) ? 'w' : '-',
    (perm & S_IXOTH) ?
    (((perm & S_ISVTX) && (flags & FP_SPECIAL)) ? 't' : 'x') :
    (((perm & S_ISVTX) && (flags & FP_SPECIAL)) ? 'T' : '-'));
    return str;
}
