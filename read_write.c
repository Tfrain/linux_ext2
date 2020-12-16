#include "read_write.h"

//底层读文件，
int read(unsigned int fd, char* buf, int count){
    struct file* file;
    struct m_inode* inode;
    if(fd >= NR_OPEN || count < 0 || !(file = current.filp[fd])){
        return 0;
    }
    if(!count)
        return 0;
    inode = file->f_inode;
    //此处可以判断是那种类型的文件，然后调用相应的读写函数，这里只写了普通
    //文件的读写，后期完善吧(觉得管道会有意思些)
    return file_read(inode, file, buf, count);
}

//底层写文件
int write(unsigned int fd, char* buf, int count){
    struct file* file;
    struct m_inode* inode;
    if(fd >= NR_OPEN || count < 0 || !(file = current.filp[fd])){
        return 0;
    }
    if(!count)
        return 0;
    inode = file->f_inode;
    return file_write(inode, file, buf, count);
}
// 从文件的offset开始
int lseek(unsigned int fd, long offset, int origin){
    struct file* file;
    int tmp;
    if(fd >= NR_OPEN || !(file = current.filp[fd])){
        return 0;
    }
    switch(origin){
        case 0: 
                file->f_pos = offset;
                break;
        case 1: 
                file->f_pos += offset;
                break;
        case 2:
                tmp = file->f_inode->i_size + offset;
                file->f_pos = offset;
                break;
        default:
            return 0;
    }
    return file->f_pos;
}