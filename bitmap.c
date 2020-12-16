#include <string.h>
#include "bitmap.h"

//释放数据块
int free_block(int dev, int block){
    struct super_block * sb;
    struct buffer_head* bh;
    if(!(sb = get_super(dev)))
        printf("释放数据块时候超级块没有读入\n");
    if(!(bh = get_hash_table(dev, block)))
        printf("wrong!释放不存在的块\n");
    if(bh->b_count > 1){
        brelse(bh);
        return 0;
    }
    block = block - sb->s_firstdatazone;
    //第0位始终为0
    block++;
    BIT_SET(sb->s_zmap[block/8192]->b_data[(block&8192)/8], 7 - ((block&8192)%8));
    sb->s_zmap[block/8192]->b_dirt = 1;
    return 1;
}
//申请数据块
int new_block(int dev){
    struct buffer_head* bh;
    struct super_block* sb;
    if(!(sb = get_super(dev))){
        printf("释放数据块时候超级块没有读入\n");
    }
    int b = 0;
    for(int i = 0;i<2 ;i++){
        for(int j =0 ;j<1024;j++){
            if( (b = find_first_zero_in_char(sb->s_zmap[i]->b_data[j])) > -1){
                BIT_CLEAR(sb->s_zmap[i]->b_data[j], 7 - b);
                b = b + i*8192 + j*8 + sb->s_firstdatazone - 1;
                sb->s_zmap[i]->b_dirt = 1;
                break;
            }
        }
        if(b)
            break;
    }
    if(!(bh = getblk(dev, b))){
        printf("申请空闲块buffer失败\n");
    }
    if(bh->b_count != 1){
        printf("b_count不为1\n");
    }
    memset(bh->b_data, 0, 1024);
    return b;
}
//释放i节点
void free_inode(struct m_inode* inode){
    struct super_block* sb;
    struct buffer_head* bh;
    if(!inode)
        return;
    if(!inode->i_dev){
        memset(inode, 0, sizeof(struct m_inode));
        return;
    }
    if(inode->i_count > 1){
        printf("wrong!释放还在使用的i节点,i_count=%d\n",inode->i_count);
        return;
    }
    if(inode->i_nlinks)
        printf("wrong！还有链接\n");
    if(!(sb = get_super(inode->i_dev))){
        printf("读取超级块失败\n");
    }
    if(!(bh = sb->s_imap[inode->i_num/8192]))
        printf("buffer没有i节点位图\n");
    BIT_SET(bh->b_data[(inode->i_num & 8192)/8], 7 - ((inode->i_num & 8192)%8));
    bh->b_dirt = 1;
    memset(inode, 0, sizeof(*inode));
}

//申请内存i节点
struct m_inode* new_inode(int dev){
    struct m_inode* inode;
    struct super_block* sb;
    struct buffer_head* bh;
    if(!(inode = get_empty_inode())){
       printf("从i节点表申请失败！\n");
       return NULL;
    }
    if(!(sb = get_super(dev)))
        printf("获取超级块失败\n");
    int b = 0;
    for(int i = 0;i<2 ;i++){
        for(int j =0 ;j<1024;j++){
            if( (b = find_first_zero_in_char(sb->s_imap[i]->b_data[j])) > -1){
                BIT_CLEAR(sb->s_imap[i]->b_data[j], 7 - b);
                b = b + i*8192 + j*8;
                sb->s_imap[i]->b_dirt = 1;
                break;
            }
        }
        if(b)
            break;
    }
    inode->i_count = 1;
    inode->i_dev = dev;
    inode->i_nlinks = 1;
    inode->i_num = b;
    inode->i_dirt = 1;
    inode->i_mtime = inode->i_atime = inode->i_ctime = 201975;
    return inode;
}

int find_first_zero_in_char(char c){
    int i;
    for(i = 0; i < 8; i++){
        if(BIT_CHECK(c, 7-i)){
            return i;
        }
    }
    return -1;
}