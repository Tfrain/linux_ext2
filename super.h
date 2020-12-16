#ifndef _SUPER_H
#define _SUPER_H
    #include "fs.h"
    #include "buffer.h"
    #include "inode.h"
    #include "namei.h"
    #include "bitmap.h"
    

    #include <stdio.h>
    extern struct file file_table[NR_FILE];

    extern struct super_block super_block[NR_SUPER];

    extern void mount_root(void);

    extern struct super_block* get_super(int dev);

    extern struct super_block* read_super(int dev);

    extern struct task current;


#endif