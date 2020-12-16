#include "rwblock.h"

FILE* rw;
//读磁盘, 打开磁盘file有效，从磁盘file中读block到缓冲头链表中，不论多少，链表数据设置为1024
void ll_read_block(struct buffer_head* bh){
    if(bh->b_dev == 1){
        if((rw = fopen(DISK1, "r+")) == NULL){
            printf("读设备号为1的磁盘失败");
        }  
        // printf("读磁盘dev:%d, block:%lu\n", bh->b_dev, bh->b_blocknr);
        int block = 1024 * bh->b_blocknr;
        fseek(rw, block, SEEK_SET);
        fread(bh->b_data, 1024, 1, rw);
        fclose(rw);
    }
    bh->b_uptodate = 1;
}
//写磁盘， 打开磁盘file有效，从缓冲头链表中block写到磁盘file中，不论多少，链表数据设置为1024
void ll_write_block(struct buffer_head* bh){
    if(bh->b_dev == 1){
        // printf("写磁盘dev:%d,block:%lu\n", bh->b_dev, bh->b_blocknr);
        if((rw = fopen(DISK1, "r+")) == NULL){
            printf("写设备号为1的磁盘失败");
        }
        fseek(rw, 1024*bh->b_blocknr, SEEK_SET);
        fwrite(bh->b_data, 1024, 1, rw);
        fclose(rw);
    }
    bh->b_dirt = 0;
}