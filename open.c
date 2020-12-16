#include "open.h"

//last -----open-------用户最多打开50，系统最多打开80
int hsc_open(char* filename, int flag, int mode){
    struct m_inode* inode;
    struct file* f;
    int i, fd;
    for(fd = 0; fd < NR_OPEN; fd++){
        if(!current.filp[fd])
            break;
    }
    f =  0 + file_table;
    for(i = 0; i < NR_FILE; i++, f++){
        if(!f->f_count){
            break;
        }
    }
    (current.filp[fd] = f)->f_count ++;
    if((i = open_namei(filename, flag, mode, &inode)) < 0){
        current.filp[fd] = NULL;
        f->f_count = 0;
        return i;
    }
    f->f_mode = mode;
    f->f_flags = flag;
    f->f_count = 1;
    f->f_inode = inode;
    f->f_pos = 0;
    return fd;
}
//创建文件，调用open方法，找到用户打开文件表空项和系统打开文件表空项
int hsc_creat (char* filename, int mode){
    return hsc_open(filename, O_CREAT | O_TRUNC, mode);
}
//关闭文件
int hsc_close(unsigned int fd){
    struct file* filp;
    if(!(filp = current.filp[fd])){
        printf("无对应文件描述符\n");
        return 0;
    }
    current.filp[fd] = NULL;
    if(!filp->f_count){
        printf("文件引用数为0\n");
        return 0;
    }
    if(--filp->f_count == 0){
        return 1;
    }
    iput(filp->f_inode);
    return 1;
}