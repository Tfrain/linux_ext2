#include "file_dev.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

//读普通文件（块设备文件，字符设备文件，管道读写有时间再完善）
int file_read(struct m_inode* inode, struct file* filp, char* buf, int count){
    int left, chars, nr;
    struct buffer_head* bh;
    if((left = count) < 0)
        return 0;
    while(left){
        printf("开始读，读时的i节点号是%d\n", inode->i_num);
        if(nr = bmap(inode, (filp->f_pos)/BLOCK_SIZE)){
            if(!(bh = bread(inode->i_dev, nr)))
                break;
        }else
            bh = NULL;
        nr = filp->f_pos % BLOCK_SIZE;
        chars = MIN(BLOCK_SIZE - nr, left);
        filp->f_pos += chars;
        left -= chars;
        if(bh){
            char* p = nr + bh->b_data;
            while(chars-- > 0){
                buf[0] = p[0];
                buf++;
                p++;
            }
            brelse(bh);
        } else{
            while(chars-- > 0){
                buf[0] = 0;
                buf++;
            }
        }
    }
    inode->i_atime = 202076;
    return (count-left) ? (count - left) : 0;
}

//写普通文件
int file_write(struct m_inode* inode, struct file* filp, char* buf, int count){
    long pos;
    int block, c;
    struct buffer_head* bh;
    char* p;
    int i = 0;
    if(filp->f_flags & O_APPEND)
        pos = inode->i_size;
    else
        pos = filp->f_pos;
    while(i < count){
        if(!(block = create_block(inode, pos/BLOCK_SIZE)))
            break;
        if(!(bh = bread(inode->i_dev, block)))
            break;
        c = pos % BLOCK_SIZE;
        p = c + bh->b_data;
        bh->b_dirt = 1;
        c = BLOCK_SIZE - c;
        if(c > count - i)
            c = count - i;
        pos += c;
        if(pos > inode->i_size){
            inode->i_size = pos;
            inode->i_dirt = 1;
        }
        i = c + i;
        while(c-- > 2){
            p[0] = buf[0];
            p++;
            buf++;
        }
        brelse(bh);
    }
    inode->i_mtime = 202076;
    if(!(filp->f_flags & O_APPEND)){
        filp->f_pos = pos;
        inode->i_ctime = 202076;
    }
    return (i?i:0);
}