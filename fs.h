#ifndef _FS_H
#define _FS_H
#include "dataType.h" 
//buffer_head用到task_struct
#include  <linux/sched.h>

#define BLOCK_SIZE 1024
//引导块
#define INTRO_START 0
//超级块 
#define SUPER_START 1024
//i节点位图
#define INODE_MAP 2048
//数据块位图
#define BLOCK_MAP 4*1024
//i节点
#define INODE_AREA 6*1024
//数据块
#define BLOCK_AREA 6*1024 + 1024*500
//hash数组
#define NR_HASH 80
//超级块数组个数
#define NR_SUPER 8
//定义魔数
#define SUPER_MAGIC 0x137f
//i节点位图个数
#define I_MAP_NR 2
//数据块位图个数
#define Z_MAP_NR 2
//系统最多打开文件数
#define NR_FILE 80
//用户最多打开文件数
#define  NR_OPEN 50 
//定义根文件系统
#define ROOT_DEV 1
//定义根文件系统根i节点号
#define ROOT_INO 1
//定义inode表项数
#define NR_INODE 50
//每块i节点数
#define INODE_PER_BLOCK ((BLOCK_SIZE)/sizeof(struct d_inode))
//每块目录项数
#define DIR_ENTRIES_PER_BLOCK ((BLOCK_SIZE)/sizeof(struct dir_entry))
//文件名长度
#define NAME_LEN 15
//文件类型屏蔽码高四位
#define S_IFMT 00170000
//符号链接文件
#define S_IFLNK 0120000
//常规文件
#define S_IFREG 0100000
//目录文件
#define S_IFDIR 0040000

//判断目录文件
#define S_ISDIR(m)  ( ((m) & S_IFMT) == S_IFDIR)
//判断常规文件
#define S_ISREG(m)  ( ((m) & S_IFMT) == S_IFREG)
//判断链接文件
#define S_ISLNK(m)  ( ((m) & S_IFMT) == S_IFLNK)


//缓冲头
struct  buffer_head
{
    char* b_data; 
    _u64 b_blocknr; //设备中块号
    _u16 b_dev; //设备号
    _u8 b_uptodate;//同步完毕
    _u8 b_dirt;//脏位
    _u8 b_count;
    _u8 b_lock; 
    struct task_struct* b_wait;
    struct buffer_head* b_prev;//hash指针
    struct buffer_head* b_next;
    struct buffer_head* b_prev_free; //空闲节点指针
    struct buffer_head* b_next_free;
};
//磁盘i节点，主要是装饰
struct d_inode {
    _u16 i_mode;
    _u16 i_uid;
    _u64 i_size;
    _u8 i_gid;
    _u64 i_mtime;
    _u8 i_nlinks;
    _u16 i_zone[9];
};

//内存i节点
struct m_inode {
    _u16 i_mode;//目录和文件
    _u16 i_uid;
    _u64 i_size;
    _u8 i_gid;
    _u64 i_mtime;
    _u8 i_nlinks;
    _u16 i_zone[9];
    //内存存在,有等待i节点的进程链表未写出
    _u64 i_atime; //访问时间
    _u64 i_ctime; //修改时间
    _u16 i_dev; //设备号，和block配合
    _u16 i_num; //i节点号
    _u16 i_count; //引用次数
    _u8 i_dirt;// 脏位
    _u8 i_update;
    _u8 i_mount;
    _u8 i_lock;
    _u8 i_pipe;// 未实现
};

//磁盘超级块
struct d_super_block
{
    _u16 s_ninodes; //节点数
    _u16 s_nzones;//数据块数
    _u16 s_imap_blocks;//i节点数据位图块数
    _u16 s_zmap_blocks;//数据块位图数
    _u16 s_firstdatazone;//第一个数据逻辑块
    _u16 s_log_zone_size;
    _u64 s_max_size;
    _u16 s_magic;
};

//内存超级块
struct super_block{
    _u16 s_ninodes; //节点数
    _u16 s_nzones;//数据块数
    _u16 s_imap_blocks;//i节点数据位图块数
    _u16 s_zmap_blocks;//数据块位图数
    _u16 s_firstdatazone;//第一个数据逻辑块
    _u16 s_log_zone_size;
    _u64 s_max_size;
    _u16 s_magic;
    //内存存在
    struct buffer_head* s_imap[2];
    struct buffer_head* s_zmap[2];//和缓冲联系
    _u16 s_dev;
    struct m_inode* s_isup;//被安装的根i节点
    struct m_inode* s_imount;//安装到的i节点
    _u64 s_time;
    _u8 s_dirt;
    _u8 s_lock;
};

//系统打开文件
struct file{
    _u16 f_mode; //文件操作模式
    _u16 f_flags; //打开控制模式
    _u16 f_count;
    struct m_inode* f_inode;
    long f_pos;
};

//定义task，简单的pcd
struct task {
    struct m_inode* pwd;
    struct m_inode* root;
    struct file* filp[NR_OPEN];
};

//目录项结构
struct dir_entry{
    unsigned short inode;
    char name[NAME_LEN];
};
#endif 