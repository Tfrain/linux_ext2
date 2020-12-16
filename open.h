#ifndef _OPEN_H
#define _OPEN_H
    #include "fs.h"
    #include "namei.h"
    #include "super.h"
    
    #include <stdio.h>

    int hsc_open(char* filename, int flag, int mode);

    int hsc_creat (char* filename, int mode);

    int hsc_close(unsigned int fd);
#endif