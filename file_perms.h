// Name: Nikolay Goncharenko
// Email: goncharn@onid.oregonstate.edu
// Class: CS344-400
// Assignment: homework#3 Problem3

// File is copied from the tlpi, Listing 15-3, p.296

 
#ifndef FILE_PERMS_H
#define FILE_PERMS_H
#include <sys/types.h>
#define FP_SPECIAL 0 
/* Include set-user-ID, set-group-ID, and sticky bit information in returned string */

char *filePermStr(mode_t perm, int flags);
#endif
