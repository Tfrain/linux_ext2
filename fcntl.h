#ifndef _FCNTL_H
#define _FCNTL_H
    
    //文件访问模式屏蔽码
    #define O_ACCMODE 00003
    //只读
    #define O_RDONLY 00
    //只写
    #define O_WRONLY 01
    //读写
    #define O_RDWR 02
    //创建 
    #define O_CREAT 00100
    //截0
    #define O_TRUNC 01000 
    //添加方式
    #define O_APPEND 02000
    //文件读写指针偏移值至最开始
    #define  SEEK_SET 0
    // 当前读写位置
    #define  SEEK_CUR 1
    //文件末尾处
    #define SEEK_END 2 
#endif 