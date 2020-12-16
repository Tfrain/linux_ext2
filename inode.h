#ifndef _INODE_H
#define _INODE_H
    #include "fs.h"
    #include "bitmap.h"
    #include "buffer.h"
    #include "super.h"
    #include "truncate.h"

    #include <string.h>
    #include <stdio.h>

    extern struct super_block super_block[NR_SUPER];

    void sync_inodes(void );

    int _bmap(struct m_inode* inode, int block, int create);

    int bmap(struct m_inode* inode, int block);

    int create_block(struct m_inode* inode, int block);

    void iput(struct m_inode* inode);

    struct m_inode* get_empty_inode(void);

    struct m_inode* iget(int dev, int nr);

    void read_inode(struct m_inode* inode);

    void write_inode(struct m_inode* inode);

#endif 