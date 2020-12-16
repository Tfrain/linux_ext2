#include "namei.h"


struct m_inode* _namei (char *filename, struct m_inode* base, int follow_links);
//检测文件的访问权限
int permission(struct m_inode* inode, int mask){
    int mode = inode->i_mode;
    if(inode->i_dev)
        return 0;
    if((mode & mask & 0007) == mask)
        return 1;
    return 0;
}
//名字匹配
int match (int len, const char* name, struct dir_entry* de){
    if(!de || !de->inode || len > NAME_LEN)
        return 0;
     //如 ‘/a//b’可以匹配  
    if(!len && (de->name[0] == '.') && (de->name[1] == '\0'))
        return 1;
    if(len > NAME_LEN && de->name[len])
        return 0;
    int j =0;
    for(int i = 0; i < len ; i ++){
        if(name[i] != de->name[i]){
            return 0;
        }
        j = i;
    }
    if(de->name[j + 1]){
        return 0;
    }
    return 1;
}

//找文件名称对应的I节点
struct buffer_head* find_entry(struct m_inode ** dir, char* name, int namelen, struct dir_entry ** res_dir){
    int entries;
    int block, i;
    struct buffer_head* bh;
    struct dir_entry* de;
    struct super_block* sb;
    if(!(*dir)->i_size){
        printf("名字长度为0\n");
    }
    entries = (*dir)->i_size / (sizeof(struct dir_entry));
    // printf("此时长度%d", entries);
    *res_dir = NULL;
    if(namelen == 2 && name[0] == '.' && name[1] == '.' ){
        if((*dir) == current.root)
            namelen = 1;
        else if((*dir)->i_num == ROOT_INO){
            sb = get_super((*dir)->i_dev);
            if(sb->s_imount){
                iput(*dir);
                (*dir) = sb->s_imount;
                (*dir)->i_count++;
            }
        }
    }
    if(!(block = (*dir)->i_zone[0]))
        return NULL;
    if(!(bh = bread((*dir)->i_dev, block)))
        return NULL;
    i =0 ;
    de = (struct dir_entry *) bh->b_data;
    while(i < entries){
        if((char* )de >= BLOCK_SIZE + bh->b_data){
            brelse(bh);
            bh = NULL;
            if(!(block = bmap(*dir, i/DIR_ENTRIES_PER_BLOCK)) || !(bh = bread((*dir)->i_dev, block))){
                i += DIR_ENTRIES_PER_BLOCK;
                continue;
            }
            de = (struct dir_entry*) bh->b_data;
        }
        if(match(namelen, name, de)){
            *res_dir = de;
            // printf("匹配到name:%s,i节点号:%d\n", de->name, de->inode);
            return bh;
        }
        de++;
        i++;
    }
    brelse(bh);
    return NULL;
}
//添加目录项，1kb内创建数据块
struct buffer_head* add_entry(struct m_inode* dir, char * name, int namelen, struct dir_entry** res_dir){
    int block, i;
    struct buffer_head* bh;
    struct dir_entry* de;
    *res_dir = NULL;
    if(!(block = dir->i_zone[0])){
        return NULL;
    }
    if(!(bh = bread(dir->i_dev, block))){
        return NULL;
    }
    i = 0;
    de =(struct dir_entry*) bh->b_data;
    while(1){
        if((char* )de >= BLOCK_SIZE + bh->b_data){
            brelse(bh);
            bh = NULL;
            block = create_block(dir, i/DIR_ENTRIES_PER_BLOCK);
            if(!block)
                return NULL;
            if(!(bh = bread(dir->i_dev, block))){
                i += DIR_ENTRIES_PER_BLOCK;
                continue;
            }
            de = (struct dir_entry*) bh->b_data;
        }
        if(i* sizeof(struct dir_entry) >= dir->i_size){
            de->inode = 0;
            dir->i_size = (i + 1)*sizeof(struct dir_entry);
            dir->i_dirt = 1;
            dir->i_ctime = 202076;
        }
        if(!de->inode){
            dir->i_mtime = 202076;
            for(i=0; i<NAME_LEN; i++){
                de->name[i] = (i<NAME_LEN)? name[i] : 0;
            }
            bh->b_dirt = 1;
            *res_dir = de;
            return bh;
        }
        de++;
        i++;
    }
}

//找链接文件
struct m_inode* follow_link(struct m_inode* dir, struct m_inode* inode){
    struct buffer_head* bh;
    if(!dir){
        dir = current.root;
        dir->i_count++;
    }
    if(!inode){
        iput(inode);
        return NULL;
    }
    if(!S_ISLNK(inode->i_mode)){
        iput(dir);
        return inode;
    }
    if(!inode->i_zone[0] || !(bh = bread(inode->i_dev, inode->i_zone[0]))){
        iput(inode);
        return NULL;
    }
    inode = _namei(bh->b_data, dir, 0);
}

//找到顶层目录项
struct m_inode* get_dir (char * pathname, struct m_inode* inode){
    char c;
    char * thisname;
    struct buffer_head* bh;
    int namelen, inr;
    struct dir_entry* de;
    struct m_inode* dir;

    if(!inode){
        inode = current.pwd;
        inode->i_count ++;
    }
    if((c = pathname[0]) == '/'){
        iput(inode);
        inode = current.root;
        pathname++;
        inode->i_count ++;
    }
    while(1){
        thisname = pathname;
        if(!S_ISDIR(inode->i_mode)){
            iput(inode);
            return NULL;
        }
        for(namelen = 0;(c = pathname[0]) && (c != '/');namelen++, pathname++){
            // printf("当前字符%c\n",c);
        }
        if(!c){
            if(inode->i_num == 1){
                inode = current.root;
            }
            return inode;
        }
        pathname ++;
        if(!(bh = find_entry(&inode, thisname, namelen, &de))){
            iput(inode);
            return NULL;
        }
        inr = de->inode;
        brelse(bh);
        dir = inode;
        if(!(inode = iget(dir->i_dev, inr))){
            iput(inode);
            return NULL;
        }
        if(!(inode = follow_link(dir, inode)))
            return NULL;
    }
}

//返回顶层目录名称和i节点指针，
struct m_inode* dir_namei(char *pathname, int* namelen, char** name, struct m_inode* base ){
    char c;
    char* basename;
    struct m_inode* dir;
    if(!(dir  = get_dir(pathname, base)))
        return NULL;
    basename = pathname;
    while(c = pathname[0]){
        pathname++;
        if(c == '/')
            basename = pathname;
    }
    *namelen = pathname - basename;
    *name = basename;
    return dir;
}

//找文件对应的i节点，最后一个参数，是否跟随符号链接
struct m_inode * _namei(char* pathname, struct m_inode* base, int follow_links){
    char* basename;
    int inr,namelen;
    struct m_inode* dir;
    struct buffer_head* bh;
    struct dir_entry* de;
    if(!(dir = dir_namei(pathname, &namelen, &basename, NULL))){
        return NULL;
    }
    if(!namelen)
        return dir;
    if(!(base = dir_namei(pathname, &namelen, &basename, base))){
        return NULL;
    }
    if(!namelen)
        return base;
    if(!(bh = find_entry(&base, basename, namelen, &de))){
        iput(base);
        return NULL;
    }
    inr = de->inode;
    brelse(bh);
    if(!(dir = iget(base->i_dev, inr))){
        iput(base);
        return NULL;
    }
    if(follow_links){
        dir = follow_link(base, dir);
    }else
    {
        iput(dir);
    }
    dir->i_atime = 202076;
    dir->i_dirt = 1;
    return dir;
}
//无链接
struct m_inode* lnamei(char* pathname){
    return _namei(pathname, NULL, 0);
}
//有链接
struct m_inode* namei(char * pathname){
    return _namei(pathname, NULL, 1);
}

//下面开始写open, mkdir ,rmdir内部实现....boom，标志位复杂
//这个函数返回0，*res_inode为所需文件i节点
int open_namei(char* pathname, int flag, int mode, struct m_inode ** res_inode){
    char* basename;
    int namelen, inr, dev;
    struct m_inode* dir, * inode;
    struct buffer_head* bh;
    struct dir_entry* de;
    //文件截0是只读
    if((flag & O_TRUNC) && !(flag && O_ACCMODE)){
        flag |= O_WRONLY;
    }
    if(!(dir = dir_namei(pathname, &namelen, &basename, NULL))){
        printf("出错，找不到顶层目录\n");
        return 1;
    }
    if (!namelen)
    {
        //是目录且不读写，不创建，不截0
        if(!(flag & (O_ACCMODE | O_CREAT | O_TRUNC))){
            *res_inode = dir;
            return 0;
        }
        iput(dir);
        return 1;
    }
    // printf("顶层目录文件长度%lu\n", dir->i_size);
    bh = find_entry(&dir, basename, namelen, &de);
    //找不到对应的目录目录项表明创建文件
    if(!bh){
        if(!(flag & O_CREAT)){
            iput(dir);
            return 1;
        }
        // if(!permission(dir, MAY_WRITE)){

        // }
        inode = new_inode(dir->i_dev);
        if(!inode){
            printf("申请inode失败\n");
            return 1;
        }
        inode->i_mode = mode;
        inode->i_dirt = 1;
        bh = add_entry(dir, basename, namelen, &de);
        if(!bh){
            printf("添加目录项失败\n");
            return 1;
        } 
        printf("申请的i节点号%d\n", inode->i_num);
        de->inode = inode->i_num;
        bh->b_dirt = 1;
        brelse(bh);
        iput(dir);
        *res_inode = inode;
        return 0;
    }
    inr = de->inode;
    dev = dir->i_dev;
    brelse(bh);
    if(!(inode = follow_link(dir, iget(dev, inr)))){
        printf("不可能的错误！\n");
        return 1;
    }
    inode->i_atime = 201976;
    if(flag & O_TRUNC){
        truncate(inode);
    }
    *res_inode = inode;
    return 0;
}


//创建一个文件
int sys_mknod(char* filename, int mode, int dev){
    char* basename;
    int namelen;
    struct m_inode* dir, *inode;
    struct buffer_head * bh;
    struct dir_entry* de;
    printf("开始创建文件\n");
    if(!(dir = dir_namei(filename, &namelen, &basename, NULL)))
        return 1;
    if(!namelen){
        iput(inode);
        return 1;
    }
    bh = find_entry(&dir, basename, namelen, &de);
    inode = new_inode(dir->i_dev);
    inode->i_mode = mode;
    inode->i_mtime = inode->i_atime = 201976;
    inode->i_dirt = 1;
    bh = add_entry(dir, basename, namelen, &de);
    de->inode = inode->i_num;
    bh->b_dirt = 1;
    iput(dir);
    iput(inode);
    brelse(bh);
    return 0;
}

//创建目录，获取顶层目录，/则找文件名称对应I节点，申请inode（目录也用内存inode），申请数据块block，读取数据区
// 目录项的记录inode等于inode属性节点号，初始化.和..目录，如果不是根，则挂载（添加目录项）
int sys_mkdir(char* pathname, int mode){
    char* basename;
    int namelen;
    struct m_inode *dir, *inode;
    struct buffer_head * bh, *dir_block;
    struct dir_entry* de;
    if(!(dir = dir_namei(pathname, &namelen, &basename, NULL))){
        printf("获取顶层目录失败\n");
        return 1;
    }
    if(strcmp(pathname, "/")) {
        bh = find_entry(&dir, basename, namelen, &de);
    }
    inode = new_inode(dir->i_dev);
    if(!inode){
        printf("申请不到i节点\n");
        return 1;
    }
    inode->i_size = 2*sizeof(struct dir_entry);
    inode->i_dirt = 1;
    inode->i_atime = inode->i_mtime = 201976;
    if(!(inode->i_zone[0] = new_block(inode->i_dev))){
        printf("申请数据块失败\n");
        iput(dir);
        iput(inode);
        return 1;
    }
    inode->i_dirt = 1;
    if(!(dir_block = bread(inode->i_dev, inode->i_zone[0]))){
        printf("读取第一块数据区失败\n");
        iput(dir);
        iput(inode);
        return 1;
    }
    de = (struct dir_entry*) dir_block->b_data;
    de->inode = inode->i_num;
    strcpy(de->name, ".");
    // .. 目录inode号等于上一层node号
    de++;
    de->inode = dir->i_num;
    strcpy(de->name, "..");

    inode->i_nlinks = 2;
    dir_block->b_dirt = 1;
    brelse(dir_block);
    inode->i_mode = S_IFDIR | (mode & 0777);
    inode->i_dirt = 1;
    if(!strcmp(pathname, "/")){
        current.root = inode;
    }
    //如果是根不执行挂载
    if(strcmp(pathname, "/")) {
        bh = add_entry(dir, basename, namelen, &de);
        if(!bh){
            printf("出错！\n");
            iput(dir);
            iput(inode);
            return 1;
        }
        de->inode = inode->i_num;
        bh->b_dirt = 1;
        dir->i_nlinks ++;
        dir->i_dirt = 1;
        iput(dir);
        iput(inode);
        brelse(bh);
    }
    return 0;
}

//删除文件，找顶层目录，找目录项，释放目录，
int unlink(char* name){
    char* basename;
    int namelen;
    struct  m_inode* dir, * inode;
    struct buffer_head* bh;
    struct dir_entry* de;
    if(!(dir = dir_namei(name, &namelen, &basename, NULL))){
        printf("找不到顶层目录\n");
        return 0;
    }
    bh = find_entry(&dir, basename, namelen, &de);
    if(!bh){
        iput(dir);
        printf("找不到目录项!\n");
        return 0;
    }
    if(!(inode = iget(dir->i_dev, de->inode))){
        iput(dir);
        brelse(bh);
        return 0;
    }
    if(S_ISDIR(inode->i_mode)){
        printf("释放目录文件\n");
        return 0;
    }
    if(!inode->i_nlinks){
        printf("i节点链接数为0\n");
        return 0;
    }
    de->inode = 0;
    bh->b_dirt = 1;
    brelse(bh);
    inode->i_nlinks--;
    inode->i_dirt = 1;
    inode->i_ctime = 201976;
    iput(inode);
    iput(dir);
    return 1;
}
//判断目录空
int empty_dir(struct m_inode* inode){
    int nr, block;
    int len;
    struct buffer_head* bh;
    struct dir_entry* de;
    len = inode->i_size / sizeof(struct dir_entry);
    if(len == 2){
        return 1;
    }else
    {
        return 0;
    }
}
//删除目录
int d_rmdir(char* name){
    char* basename;
    int namelen;
    struct m_inode* dir, *inode;
    struct buffer_head* bh;
    struct dir_entry* de;
    if(!(dir = dir_namei(name, &namelen, &basename, NULL))){
        printf("找不到顶层目录\n");
        return 0;
    }
    bh = find_entry(&dir, basename, namelen, &de);
    if(!bh){
        iput(dir);
        printf("找不到目录项!\n");
        return 0;
    }
    if(!(inode = iget(dir->i_dev, de->inode))){
        iput(dir);
        brelse(bh);
        return 0;
    }
    if(!S_ISDIR(inode->i_mode)){
        iput(dir);
        iput(inode);
        printf("不是目录文件,失败\n");
        brelse(bh);
        return 0;
    }
    if(!empty_dir(inode)){
        iput(dir);
        iput(inode);
        printf("目录不为空，删除失败\n");
        brelse(bh);
        return 0;
    }
    de->inode = 0;
    // printf("de的name:%s", de->name);
    bh->b_dirt = 1;
    brelse(bh);
    inode->i_nlinks = 0;
    inode->i_dirt = 1;
    dir->i_nlinks --;
    dir->i_ctime = dir->i_mtime = 201976;
    dir->i_dirt = 1;
    iput(dir);
    iput(inode);
    return 1;
}


