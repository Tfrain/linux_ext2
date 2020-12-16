#ifndef _NAMEI_H
#define _NMAEI_H
    #include "fs.h"
    #include "super.h"
    #include "fcntl.h"
    #include "truncate.h"
    #include "bitmap.h"
    #include "inode.h"


    #include <string.h>
    #include <stdio.h>

    #define MAY_EXEC 1
    #define MAY_WRITE 2
    #define MAY_READ 4


    int permission(struct m_inode* inode, int mask);

    int match (int len, const char* name, struct dir_entry* de);

    struct buffer_head* find_entry(struct m_inode ** dir, char* name, int namelen, struct dir_entry ** res_dir);

    struct buffer_head* add_entry(struct m_inode* dir, char * name, int namelen, struct dir_entry** res_dir);

    struct m_inode* follow_link(struct m_inode* dir, struct m_inode* inode);

    struct m_inode* get_dir (char * pathname, struct m_inode* inode);

    struct m_inode* dir_namei(char *pathname, int* namelen, char** name, struct m_inode* base );

    struct m_inode * _namei(char* pathname, struct m_inode* base, int follow_links);

    struct m_inode* lnamei(char* pathname);

    struct m_inode* namei(char * pathname);

    int open_namei(char* pathname, int flag, int mode, struct m_inode ** res_inode);

    int sys_mknod(char* filename, int mode, int dev);

    int sys_mkdir(char* pathname, int mode);
    
    int unlink(char * name);

    int d_rmdir(char * name);
#endif 