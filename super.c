#include "super.h"

struct super_block super_block[NR_SUPER];

struct file file_table[NR_FILE];


struct task current;
//从超级块数组中取得超级块
struct super_block* get_super(int dev){
    struct super_block* s;
    s = 0 + super_block;
    while(s < super_block + NR_SUPER){
        if(dev == s->s_dev)
            return s;
        else
            s++;
    }
    return NULL;
}
//从磁盘读超级块
struct super_block* read_super(int dev){
    struct super_block* s;
    struct buffer_head* bh;
    if(s = get_super(dev)){
        return  s;
    }
    for(s = 0 + super_block; ;s++){
        if(s > super_block + NR_SUPER)
            return NULL;
        if(!s->s_dev)
            break;
    }
    s->s_dev = dev;
    s->s_isup = NULL;
    s->s_imount = NULL;
    s->s_time = 0;
    s->s_dirt = 0;
    if(!(bh = bread(dev, 1))){
        printf("读超级块出错\n");
        return NULL;
    }
    //强转，bh->b_data无格式
    *((struct d_super_block*) s) = *((struct d_super_block *) bh->b_data);
    brelse(bh);
    printf("super: %d\n", s->s_magic);
    if(s->s_magic != SUPER_MAGIC){
        printf("不支持此类文件系统\n");
        return NULL;
    }
    for(int i = 0 ; i<I_MAP_NR; i++){
        s->s_imap[i] = NULL;
    }
    for(int i = 0; i<Z_MAP_NR; i++){
        s->s_zmap[i] = NULL;
    }
    int block = 2;
    for(int i = 0; i < s->s_imap_blocks ; i++){
        if(s->s_imap[i] = bread(dev, block)){
            block++;
        }else{
            printf("读i节点位图出错\n");
        }
    }
    for(int i = 0 ; i < s->s_zmap_blocks ; i++){
        if(s->s_zmap[i] = bread(dev, block)){
            block++;
        }else{
            printf("读数据块位图出错\n");
        }
    }
    BIT_CLEAR(s->s_imap[0]->b_data[0], 7);
    BIT_CLEAR(s->s_zmap[0]->b_data[0], 7);
    // find_first_zero_in_char(s->s_imap[0]->b_data[0]);
    return s;
}

//安装根文件系统，80个文件打开表文件数量为0，8个内存超级块dev和lock置0
//从磁盘读超级块，根文件系统为1，先去磁盘超级块中找超级块，没有则初始化，根初始化为1，先使用内存buffer结合block号操作，然后将buffer转换为磁盘超级块
// 将此内存超级块中inode节点位图和数据块位图置空再分配buffer 进行初始化，最后建立位图和缓存data之间的关系，这里的dev 为1，会将这些dev为1的bufffer通过超级块联系在一起
//iget 从设备i节点中取i节点到内存中,第二个参数inode节点号，返回内存inode（根1,1，其中先从内存i节点找空的inode，然后超级块通过判断inode 的设备号读超级块，然后分配一个buffer来读出数据到inode）
// 需要超级块是为了计算应该分配的block号去分配buffer，最后将内存inode引用加3，将超级块的根节点和挂载节点初始化为此内存inode，将进程控制块的root的pwd和root指定为根/
void mount_root(void){
    struct super_block* p;
    struct m_inode* mi;
    int i;
    for(i = 0; i < NR_FILE ;i++){
        file_table[i].f_count = 0;
    }
    for(p = 0 + super_block; p < super_block + NR_SUPER; p++){
        p->s_dev = 0;
        p->s_lock = 0;
    }
    if(!(p = read_super(ROOT_DEV)))
        printf("读根文件系统超级块失败\n");
    if(!(mi = iget(ROOT_DEV, ROOT_INO)))
        printf("读根i节点失败\n");
    mi->i_count += 3;
    p->s_isup = p->s_imount = mi;
    mi->i_mode |= S_IFDIR;
    mi->i_mode |= MAY_EXEC;
    current.pwd = mi;
    current.root = mi;
    mi = current.root;
}