#include "myMemory.h"
#include "init.h"
#include "super.h"
#include "fs.h"
#include "namei.h"
#include "open.h"
#include "inode.h"
#include "namei.h"
#include "read_write.h"
#include "cmd.h"

int main(int argc , char** argv) {
    //初始化文件为ext2文件系统
    initFileSys();
    //初始化buffer
    getBuffer();
    initBuffer();
    //挂载根文件系统
    mount_root();
    //手动创建根目录
    sys_mkdir("/",0777);
    //创建文件first
    printf("_________________________________________\n");
    int fd;
    fd = hsc_creat("/first", 0777 | S_IFREG);
    printf("文件描述符:%d\n", fd);
    char* buf = "hello world";
    write(fd, buf, 13);

    char res[13];
    lseek(fd, 0, SEEK_SET);
    read(fd, res, 13);
    printf("读的结果:%s\n", res);
    hsc_close(fd);  
    
    fd = hsc_creat("/test", 0777 | S_IFREG);
    printf("文件描述符:%d\n", fd);
    char* buf1 = "test ext2";
    write(fd, buf1, 13);
     hsc_close(fd);


    fd = hsc_creat("/link", 0777 | S_IFLNK);
    char * buf2 = "/first";
    write(fd, buf2, 9);

    fd = hsc_creat("/testlink", 0777 | S_IFLNK);
    char * buf3 = "/test";
    write(fd, buf3, 9);

    sys_mkdir("/dire", 0777 | S_IFDIR);
    sys_mkdir("/dire/wei", 0777 | S_IFDIR);
    sys_mkdir("/dir", 0777 | S_IFDIR);
    hsc_close(fd);


    //ls原型
    printf("_________________________________________\n");
    struct buffer_head* bh;
    int size =  current.root->i_size / (sizeof(struct dir_entry));
    bh = bread(current.root->i_dev, current.root->i_zone[0]);
    struct dir_entry * de = (struct dir_entry*) bh->b_data;
    printf("当前文件目录\n");
    for(int i=0; i < size;i++, de++){
        printf("%s\n", de->name);
    } 
    printf("_________________________________________\n");


    //符号链接
    fd = hsc_open("/link", O_RDWR, 0777);
    char res1[14];
    printf("打开符号链接文件\n");
    read(fd, res1, 14);
    printf("符号链接：%s\n", res1);
    printf("_________________________________________\n");

    fd = hsc_creat("/dire/haha", 0777 | S_IFREG);
    write(fd, buf1, 14);
    lseek(fd, 0, SEEK_SET);
    char res2[14];
    read(fd, res2, 14);
    printf("读出的数据%s\n", res2);
    printf("_________________________________________\n");

    //进入shell
    current.pwd = current.root;
    while(1){
        char cmd[50];
        printf("$ ");
        scanf("%s", cmd);
        if(!strcmp(cmd, "ls")){
            list();
            continue;
        }
        if(!strcmp(cmd, "mkdir")){
            mkdir();
            continue;
        }
        if(!strcmp(cmd, "touch")){
            touch();
            continue;
        }
        if(!strcmp(cmd, "exit")){
            return 0;
            continue;
        }
        if(!strcmp(cmd, "write")){
            shell_write();
            continue;
        }
        if(!strcmp(cmd, "read")){
            shell_read();
            continue;
        }
        if(!strcmp(cmd, "ln")){
            ln();
            continue;
        }
        if(!strcmp(cmd, "cd")){
            cd();
            continue;
        }
        if(!strcmp(cmd, "rm")){
            rm();
            continue;
        }
        if(!strcmp(cmd, "rmdir")){
            rmdir();
            continue;
        }
        int i = 1;
        if(!strcmp(cmd, "find")){
            lookup("-", &i);
            continue;
        }
        if(!strcmp(cmd, "pwd")){
            pwd(i);
            continue;
        }
        if(!strcmp(cmd, "help")){
            help();
            continue;
        }
        else{
            printf("\ncommond not found : %s\n", cmd);
            printf("you can try help for help\n");
        }
    }
    return 0;
}