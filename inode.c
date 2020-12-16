#include <stdio.h>
#include "inode.h"

struct m_inode inode_table[NR_INODE];

void read_inode(struct m_inode* inode);

void write_inode(struct m_inode* inode);

//内存i节点写盘(同步)
void sync_inodes(void){
    int i;
    struct m_inode* inode;
    for(i = 0; i < NR_INODE; i++){
        inode = 0 + inode_table;
        if(inode->i_dirt && !inode->i_pipe){
            write_inode(inode);
        }
    }
}

//第二个参数是文件中的数据块号
int _bmap(struct m_inode* inode, int block, int create){
    struct buffer_head* bh;
    int i;
    if(block >= 7+ 512 +512*512)
        printf("block过大\n");
    if(block < 7){
        if(create && !inode->i_zone[block]){
            if(inode->i_zone[block] = new_block(inode->i_dev)){
                inode->i_ctime = 202076;
                inode->i_dirt = 1;
                sync_inodes();
            }
        }
        printf("inode的数据块：inode->i_zone: %d\n", inode->i_zone[block]);
        return inode->i_zone[block];
    }
    block -= 7;
    if(block < 512){
        if(create && !inode->i_zone[7]){
            if(inode->i_zone[7] = new_block(inode->i_dev)){
                inode->i_ctime = 202076;
                inode->i_dirt = 1;
            }
        }
        if(!inode->i_zone[7])
            return 0;
        if(!(bh = bread(inode->i_dev, inode->i_zone[7]))){
            return 0;
        }
        i = ((unsigned short * ) (bh->b_data))[block];
        if(create && !i){
            if(i = new_block(inode->i_dev)){
                ((unsigned short *)(bh->b_data))[block] = i;
                bh->b_dirt = 1;
            }
        }  
        brelse(bh);
        return i;
    }
    block -= 512;
    if(create && !inode->i_zone[8]){
        if(inode->i_zone[8] = new_block(inode->i_dev)){
            inode->i_dirt = 1;
            inode->i_ctime = 201976;
        }
    }
    if(!inode->i_zone[8]){
        return 0;
    }
    i = ((unsigned short*)(bh->b_data)) [block/512];
    if(create && !i){
        if(i = new_block(inode->i_dev)){
            ((unsigned short*) (bh->b_data))[block/512] = i;
            bh->b_dirt = 1;
        }
    }
    brelse(bh);
    if(!i)
        return 0;
    if(!(bh = bread(inode->i_dev, i))){
        return 0;
    }
    i = ((unsigned short*) (bh->b_data)) [block&512];
    if(create && !i){
        if(i = new_block(inode->i_dev)){
            ((unsigned short*) (bh->b_data)) [block&512] = i;
            bh->b_dirt = 1;
        }
    }
    brelse(bh);
    return i;
}

//查找inode->i_zone对应的数据块号
int bmap(struct m_inode* inode, int block){
    return _bmap(inode, block, 0);
}

//创建一个数据块
int create_block(struct m_inode* inode, int block){
    return _bmap(inode, block, 1);
}

//放回i节点(包括写回,turncate)
void iput(struct m_inode* inode){
    if(!inode->i_count){
        printf("释放c_count为0的块\n");
        return;
    }
    //其实这些地方需要判断文件类型(在这个地方我们默认是普通文件)，先把框架给搭起来
    if(inode->i_count>1){
        inode->i_count --;
        return;
    }
    if(!inode->i_nlinks){
        truncate(inode);
        free_inode(inode);
        return;
    }
    if(inode->i_dirt){
        write_inode(inode);
    }
    //这个时候为0
    inode->i_count --;
}

//内存i节点表获取一个空项
struct m_inode* get_empty_inode(void){
    struct m_inode* inode;
    static struct m_inode* last_inode = inode_table;
    int i;
    inode = NULL;
    for(i = NR_INODE ; i ; i--){
        if(++last_inode >= inode_table + NR_INODE)
            last_inode = inode_table;
        if(!last_inode->i_count){
            inode = last_inode;
            if(inode->i_dirt)
                write_inode(inode);
        }
    }
    memset(inode, 0, sizeof(*inode));
    inode->i_count = 1;
    return inode;
}

//从设备i节点中取i节点到内存中,第二个参数i节点号（1,1）
struct m_inode* iget(int dev, int nr){
    struct m_inode* inode, *empty;
    empty = get_empty_inode();
    inode = inode_table;
    while(inode < NR_INODE + inode_table){
        if(inode->i_dev != dev || inode->i_num != nr){
            inode++;
            continue;
        }
        inode->i_count++;
        //找挂载点
        if(inode->i_mount) {
            int i;
            for(i = 0;i < NR_INODE; i++){
                if(super_block[i].s_imount == inode)
                    break;
            }
            iput(inode);
            dev = super_block[i].s_dev;
            nr = ROOT_INO;
            inode = inode_table;
            continue;
        }
        if(empty)
            iput(empty);
        return inode;
    }
    //从磁盘读
    if(!empty)
        return NULL;
    inode = empty;
    inode->i_dev = dev;
    inode->i_num = nr;
    read_inode(inode);
    return inode;
}

void read_inode(struct m_inode* inode){
    struct super_block* sb;
    struct buffer_head* bh;
    int block;
    if(!(sb = get_super(inode->i_dev))){
        printf("读超级块失败\n");
    }
    block = 2 + sb->s_imap_blocks +sb->s_zmap_blocks + (inode->i_num - 1)/INODE_PER_BLOCK;
    if(!(bh = bread(inode->i_dev, block))){
        printf("申请Buffer失败\n");
    }
    *(struct d_inode*) inode = ((struct d_inode*) bh->b_data) [(inode->i_num%INODE_PER_BLOCK)];
    brelse(bh);
}

void write_inode(struct m_inode* inode){
    struct super_block* sb;
    struct buffer_head* bh;
    int block;
    if(!(sb = get_super(inode->i_dev))){
        printf("读超级块失败\n");
    }
    block = 2 + sb->s_imap_blocks +sb->s_zmap_blocks + (inode->i_num - 1)/INODE_PER_BLOCK;
    if(!(bh = bread(inode->i_dev, block))){
        printf("读Buffer失败\n");
    }
    ((struct d_inode*) bh->b_data) [(inode->i_num%INODE_PER_BLOCK)] = *(struct d_inode*) inode;
    bh->b_dirt = 1;
    inode->i_dirt = 0;
    brelse(bh);
}

