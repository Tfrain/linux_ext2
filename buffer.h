#ifndef _BUFFER_H
#define _BUFFER_H
    #include "myMemory.h"
    #include "fs.h"
    #include "rwblock.h"

    #include <stdio.h>

    extern void initBuffer();

    extern struct buffer_head* find_buffer(int dev, int block);

    extern int check_buffer_lock(struct buffer_head* h);
    
    extern struct buffer_head* bread(int dev, int block);

    extern struct buffer_head* getblk(int dev, int block);

    extern void insert_into_queues(struct buffer_head* bh);

    extern void remove_from_queues(struct buffer_head* bh);

    extern void brelse(struct buffer_head* bh);
    
    extern void sync_dev(int dev);

    struct buffer_head* get_hash_table(int dev, int block);
#endif 