#ifndef _BITMAP_H 
#define _BITMAP_H
#include "fs.h"
#include "super.h"
#include "buffer.h"
#include "inode.h"

#include <string.h>

#define BIT_SET(a, b) ((a) |= ((1ULL)<<(b)))

#define BIT_CLEAR(a, b) ((a) &= ~((1ULL)<<(b)))

#define BIT_CHECK(a, b) (!!((a) & ((1ULL)<<(b))))

int free_block(int dev, int block);

int new_block(int dev);

void free_inode(struct m_inode* inode);

struct m_inode* new_inode(int dev);

int find_first_zero_in_char(char c);

//释放数据块
#endif // 