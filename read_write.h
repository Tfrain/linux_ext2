#ifndef READ_WRITE_H
#define READ_WRITE_H
    #include "file_dev.h"
    #include "fs.h"
    #include "super.h"
    #include "fcntl.h"

    int read(unsigned int fd, char* buf, int count);

    int write(unsigned int fd, char* buf, int count);

    int lseek(unsigned int fd, long offset, int origin);
#endif 