#ifndef _FILE_DEV_H
#define _FILE_DEV_H
    #include "fs.h"
    #include "inode.h"
    #include "buffer.h"
    #include "fcntl.h"

    #include <stdio.h>

    int file_read(struct m_inode* inode, struct file* filp, char* buf, int count);

    int file_write(struct m_inode* inode, struct file* filp, char* buf, int count);

#endif 