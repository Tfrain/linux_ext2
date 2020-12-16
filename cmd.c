#include "cmd.h"

char Pwd[50];
char Tmp[20];

void list(){
    struct buffer_head* bh;
    struct m_inode* inode;
    int size =  current.pwd->i_size / (sizeof(struct dir_entry));
    bh = bread(current.pwd->i_dev, current.pwd->i_zone[0]);
    struct dir_entry * de = (struct dir_entry*) bh->b_data;
    printf("___________________________当前文件目录\n");
    for(int i=0; i < size;i++, de++){
        if(de->inode){
            inode = iget(1,  de->inode);
            if(S_ISDIR(inode->i_mode)){
                printf("drwxrwxrwx %s\n", de->name);
            }else if(S_ISLNK(inode->i_mode)){
                bh = bread(inode->i_dev, inode->i_zone[0]);
                printf("lrwxrwxrwx %s -> %s\n", de->name, bh->b_data);
            }else if(S_ISREG(inode->i_mode)){
                printf("-rwxrwxrwx %s\n", de->name);
            }
        }
    } 
}

void mkdir(){
    char name[50];
    memset(name, 0, sizeof(50));
    // printf("输入目录名称:\n");
    scanf("%s", name);
    sys_mkdir(name, 0777 | S_IFDIR);
}

void touch(){
    char name[50];
    memset(name, 0, sizeof(50));
    // printf("请输入文件名称:\n");
    scanf("%s", name);
    hsc_creat(name, 0777 | S_IFREG);
}

void shell_write(){
    char name[50];
    memset(name, 0, sizeof(50));
    int fd, off;
    printf("请输入文件名称:\n");
    scanf("%s", name);
    int res = 1;
    lookup(name, &res);
    if (!res) {
        printf("%s 是一个目录，不能 write\n", name);
        setbuf(stdin, NULL);//使stdin输入流由默认缓冲区转为无缓冲区
        return;
    }
    fd = hsc_open(name, O_RDWR, 0777);
    printf("输入写的内容:\n");
    scanf("%s", name);
    scanf("%d", &off);
    lseek(fd, off, SEEK_SET);
    write(fd, name, 50);
    hsc_close(fd);
}

void shell_read(){
    char name[50];
    memset(name, 0, sizeof(50));
    int fd, off;
    printf("请输入文件名称:\n");
    scanf("%s", name);
    int res = 1;
    lookup(name, &res);
    if (!res) {
        printf("%s 是一个目录，不能read\n", name);
        setbuf(stdin, NULL);//使stdin输入流由默认缓冲区转为无缓冲区
        return;
    }
    fd = hsc_open(name, O_RDWR, 0777);
    printf("请输入读取文件位置:\n");
    scanf("%d", &off);
    lseek(fd, off, SEEK_SET);
    read(fd, name, 50);
    printf("_____________文件内容\n");
    printf("%s\n", name); 
}

void ln(){
    char name[50];
    memset(name, 0, sizeof(50));
    int fd;
    // printf("请输入文件名称:\n");
    scanf("%s", name);
    fd = hsc_creat(name, 0777 | S_IFLNK);
    // printf("请输入链接文件名:\n");
    scanf("%s", name);
    write(fd, name, 50);
}

void pwd(int i) {
    Pwd[0]='/';
    if (i == 0) {
    if (!strcmp(Tmp, "//.")) {
        strcpy(Pwd,"/");
    } else if (!strcmp(Tmp, "../.")) {
        if(strlen(Pwd) == 1) {
        
        } else {
             char *p;    
            p = strrchr(Pwd, '/');
            int size = strlen(p); 
            Pwd[strlen(Pwd)-size]='\0'; 
            if (strlen(Pwd) == 0) {
                strcpy(Pwd, "/");
            }
        }
    } else {
       if (strlen(Pwd) != 1) {
            strcat(Pwd, "/");
        }
        strcat(Pwd, Tmp);
        Pwd[strlen(Pwd)-2] = '\0';
    }
    } else {
        printf("%s\n", Pwd);
    }
}

void cd() {
    char name[50];
    // cd 无参数到根目录
    if (getchar() == '\n') {
        name[0]='/';
        name[1]='\0';
    } else {
           scanf("%s", name);
           int res=1;
            lookup(name, &res);
            if (res) {
                printf("%s 是一个文件或者不存在，不能cd\n", name);
                return;
            }
    }
    for(int i = 0; i < sizeof(name); i++){
        if(!name[i]){
            name[i] = '/';
            name[i+1] = '.';
            name[i+2]='\0';
            break;
        }
    }
    strcpy(Tmp,name);
    pwd(0);
    current.pwd =  get_dir(name, NULL);
}

void lookup(char* dir, int* res) {

    char search[50];
    memset(search, 0, 50);
    struct buffer_head* bh;
    struct m_inode* inode;
    int size =  current.pwd->i_size / (sizeof(struct dir_entry));
    bh = bread(current.pwd->i_dev, current.pwd->i_zone[0]);
    struct dir_entry * de = (struct dir_entry*) bh->b_data;

    if (strcmp("-", dir)) {
        for(int i=0; i < size;i++, de++){
        if(de->inode){
            inode = iget(1,  de->inode);
            if(S_ISDIR(inode->i_mode) && !strcmp(de->name, dir)){
                *res = 0;
            }
        }
    } 
    } else {
        scanf("%s",  search);
        for(int i=0; i < size;i++, de++){
        if(de->inode){
            inode = iget(1,  de->inode);
            if(S_ISDIR(inode->i_mode) && !strcmp(de->name, search)){
                printf("drwxrwxrwx %s\n", de->name);
                return;
            }else if(S_ISLNK(inode->i_mode) && !strcmp(de->name, search)){
                bh = bread(inode->i_dev, inode->i_zone[0]);
                printf("lrwxrwxrwx %s -> %s\n", de->name, bh->b_data);
                return;
            }else if(S_ISREG(inode->i_mode) && !strcmp(de->name, search)){
                printf("-rwxrwxrwx %s\n", de->name);
                return ;
            } else if  (i == size -1) {
                printf("没有该文件或目录\n");
                return;
            }
        }
    }    
    }
}

void rm(){
    char name[50];
    memset(name, 0, 50);
    scanf("%s", name);
    if(!unlink(name)){
        printf("删除失败!\n");
    }
}

void rmdir(){
    char name[50];
    memset(name, 0, 50);
    scanf("%s", name);
    if(!d_rmdir(name)){
        printf("删除失败\n");
    }
}

void help(){
    printf("you fool, there are some suggestions\n");
    printf("mkdir [目录文件]...创建目录\n\n");
    printf("ls ...列出当前目录所有文件\n\n");
    printf("cd [路径]...切换目录\n\n");
    printf("touch [文件]...创建普通文件\n\n");
    printf("ln [链接文件]  [源文件]...创建符号连接文件\n\n");
    printf("rm [文件]...删除文件\n\n");
    printf("rmdir [文件]...删除目录文件\n\n");
    printf("read [文件] [偏移值]...读文件\n\n");
    printf("write [文件] [内容] [偏移值]\n\n");
    printf("find [文件]\n\n");
    printf("pwd ...输出当前路径\n\n");
}