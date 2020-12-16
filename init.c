#include "init.h"

FILE* disk;
// 引导块 1kb，打开文件，创建流式传输
extern void initIntroduce(char* DISK){
    if((disk = fopen(DISK, "r+"))== NULL){
        fclose(disk);
        printf("引导块初始化失败\n");
    }else{
        char buffer[1024] = {'0'};
        fseek(disk, INTRO_START, SEEK_SET);
        fwrite(buffer, strlen(buffer), 1, disk);
        fclose(disk);
        printf("引导块初始化\n");
    }
}
// 超级块1kb，这写东西只是简单的条件限制，并不需要很大空间存储，为了方便buffer概念，弄成这么大
extern void initSuper(char* DISK){
    if((disk = fopen(DISK, "r+")) == NULL){
        //fclose(disk);
        printf("超级块初始化失败\n");
    }else{
        struct d_super_block super;
        super.s_ninodes = 16*1024;
        super.s_nzones = 16*1024;
        super.s_imap_blocks = 2;
        super.s_zmap_blocks = 2;
        super.s_firstdatazone = 505;
        super.s_log_zone_size = 50;
        super.s_max_size = 1024*7 + 1024*1024 + 1024*1024*1024;
        super.s_magic = 0x137f;
        fseek(disk, SUPER_START, SEEK_SET);
        fwrite(&super, sizeof(struct d_super_block), 1, disk);
        fclose(disk);
        printf("超级块初始化\n");
    }
}
// inode 节点位图 2kb，2kb应该是根据inode的数量限制来初始化的
void initimap(char* DISK){
    if((disk = fopen(DISK, "r+")) == NULL){
        fclose(disk);
        printf("初始化i节点位图失败\n");
    }else{
        char buffer[1024];
        memset(buffer, 255, sizeof(buffer));
        fseek(disk, INODE_MAP, SEEK_SET);
        fwrite(buffer, sizeof(buffer), 1, disk);
        fwrite(buffer, sizeof(buffer), 1, disk);
        fclose(disk);
        printf("i节点位图初始化\n");
    }
}
// 数据块位图2kb
void initzmap(char* DISK){
    if((disk = fopen(DISK, "r+")) == NULL){
        fclose(disk);
        printf("初始化数据块位图失败\n");
    }else{
        char buffer[1024];
        memset(buffer, 255, sizeof(buffer));
        fseek(disk, BLOCK_MAP, SEEK_SET);
        fwrite(buffer, sizeof(buffer), 1, disk);
        fwrite(buffer, sizeof(buffer), 1, disk);
        fclose(disk);
        printf("数据块位图初始完成\n");
    }
}
// 磁盘i节点500kb，之后全是数据块的部分
void inititable(char* DISK){
    if((disk = fopen(DISK, "r+")) == NULL){
        fclose(disk);
        printf("磁盘i节点表初始失败\n");
    }else{
        struct d_inode d_inode;
        d_inode.i_mode = 0;
        d_inode.i_uid = 0;
        d_inode.i_size =0;
        d_inode.i_gid = 0;
        d_inode.i_mtime = 0;
        d_inode.i_nlinks = 0;
        d_inode.i_zone[9] = 0;
        // 将 6kb之后的内容的每 1 kb 直到 500kb，每1kb当做一个 block，里面有多少个i节点也整好初始化完毕
        for(int j = 0; j < 500 ; j++){
            fseek(disk, INODE_AREA + (j * BLOCK_SIZE), SEEK_SET);
            for(int i =0 ;i < INODE_PER_BLOCK; i++)
                fwrite(&d_inode, sizeof(struct d_inode), 1, disk);
        }
        fclose(disk);
        printf("磁盘i节点初始化成功\n");
    }
}

//磁盘初始化
void initFileSys(void ){
    initIntroduce(DISK1);
    initSuper(DISK1);
    initimap(DISK1);
    initzmap(DISK1);
    inititable(DISK1);
}