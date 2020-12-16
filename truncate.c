#include "truncate.h"

//释放一次间接块
int free_ind (int dev, int block){
    struct buffer_head* bh;
    unsigned short* p;
    int i;
    if(bh = bread(dev, block)){
        p = (unsigned short*) bh->b_data;
        for(i = 0;i < 512; i++, p++){
            if(*p){
                if(free_block(dev, *p)){
                    *p = 0;
                    bh->b_dirt = 1;
                }else{
                    return 0;
                }
            }
        }
    }
    brelse(bh);
    return free_block(dev, block);
}

//释放二次间接块
int free_dind(int dev, int block){
    struct buffer_head* bh;
    unsigned short* p;
    int i;
    if(bh = bread(dev, block)){
        p = (unsigned short*) bh->b_data;
        for(i = 0;i < 512; i++){
            if(*p){
                if(free_ind(dev, *p)){
                    *p = 0;
                    bh->b_dirt = 1;
                }else{
                    return 0;
                }
            }
        }
    }
    brelse(bh);
    return free_block(dev, block);
}

void truncate(struct m_inode* inode){
    int i;
    int block_busy;
    for(i =0; i<7;i++){
        if(inode->i_zone[i]){
            if(free_block(inode->i_dev, inode->i_zone[i]))
                inode->i_zone[i] = 0;
        }
    }
    if(inode->i_zone[7]){
        if(free_ind(inode->i_dev, inode->i_zone[7]))
            inode->i_zone[7] = 0;
    }
    if(inode->i_zone[8]){
        if(free_dind(inode->i_dev, inode->i_zone[8]))
            inode->i_zone[8] = 0;
    }
    inode->i_dirt = 1;
    inode->i_size = 0;
    inode->i_mtime = inode->i_ctime = 201976;
}