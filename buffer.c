#include "buffer.h"

extern char* b_start;
extern char* b_end;
struct buffer_head* start_buffer;
static struct buffer_head* free_list; 
int buffer_nr = 0;
struct buffer_head* hash_table[NR_HASH];

//初始化内存buffer作为内核缓冲区，3M文件申请大概 2891 个buffer，建立 buffer_head 和数据块关联
void initBuffer(){
    start_buffer = (struct buffer_head* ) b_start;
    struct  buffer_head* h = start_buffer;
    void* b = (void* )b_end;
    while((b -= BLOCK_SIZE) >= ((void*) (h+1))){
        h->b_dev = 0;
        h->b_dirt = 0;
        h->b_blocknr = 0;
        h->b_count = 0;
        h->b_lock = 0;
        h->b_uptodate = 0;
        h->b_wait = NULL;
        h->b_next = NULL;
        h->b_prev = NULL;
        h->b_data = (char* )b;
        h->b_prev_free = h-1;
        h->b_next_free = h+1;
        h++;
        buffer_nr++;
    }
    h--;
    printf("buffer个数:%d\n", buffer_nr);
    // 凑成一个循环？
    free_list = start_buffer;
    free_list->b_prev_free = h;
    h->b_next_free = free_list;
    // hash 初始化为空
    for(int i = 0;i<NR_HASH;i++){
        hash_table[i] = NULL;
    }
}

#define _hashfn(dev, block) (((unsigned) (dev^block)) % NR_HASH)

#define hash(dev, block) hash_table[_hashfn(dev, block)]
//通过HASH找buffer
struct buffer_head * find_buffer(int dev, int block){
    struct buffer_head* tmp;
    for(tmp = hash(dev, block); tmp != NULL ;tmp = tmp->b_next){
        if(tmp->b_dev == dev && tmp->b_blocknr == block)
            return tmp;
    }
    return NULL;
}
//新版本sleep_on函数有变化
// static void wait_on_buffer(struct buffer_head* bh){
//     while(bh->b_lock){ 
//         sleep_on(&bh->b_wait);
//     }
// }

//检查是否上锁
int check_buffer_lock(struct buffer_head* h){
    if(h->b_lock)
        return 1;
    return 0;
}

//写回同步
void sync_dev(int dev){
    int i;
    struct buffer_head* bh;
    bh = start_buffer;
    for(int i = 0; i < buffer_nr; i++ ,bh++){
        if(bh->b_dev == dev && bh->b_dirt == 1)
            ll_write_block(bh);
    }
}
//释放缓冲块
void brelse(struct buffer_head* bh){
    if(!bh)
        return;
    if(!(bh->b_count))
        return;
    if(!(bh->b_count--))
        printf("该块已经被释放");
    //是否要在释放的时候同步，这样做效率会低
    if(bh->b_dirt){
        sync_dev(bh->b_dev);
    }
    bh->b_lock = 0;
}
//从hash和free_list删除
void remove_from_queues(struct buffer_head* bh){
    if(bh->b_next)
        bh->b_next->b_prev = bh->b_prev;
    if(bh->b_prev)
        bh->b_prev->b_next = bh->b_next;
    if(hash(bh->b_dev, bh->b_blocknr) == bh)
        hash(bh->b_dev, bh->b_blocknr) = bh->b_next;
    if(!(bh->b_prev_free) || !(bh->b_next_free))
        printf("buffer空闲链表错误\n");
    bh->b_prev_free->b_next_free = bh->b_next_free;
    bh->b_next_free->b_prev_free = bh->b_prev_free;
    if(free_list == bh)
        free_list = bh->b_next_free;
}

//加入到hash和free_list
void insert_into_queues(struct buffer_head* bh){
    bh->b_next_free = free_list;
    bh->b_prev_free = free_list->b_prev_free;
    free_list->b_prev_free->b_next_free = bh;
    free_list->b_prev_free = bh;
    bh->b_next = NULL;
    bh->b_prev = NULL;
    bh->b_next = hash(bh->b_dev, bh->b_blocknr);
    hash(bh->b_dev, bh->b_blocknr) = bh;
    if(bh->b_next)
        bh->b_next->b_prev = bh;
}
//取buffer，并同步文件，通过hash dev 和 block 号找buffer，并调整hash 和freelist
struct buffer_head* getblk(int dev, int block){
    sync_dev(dev);
    struct buffer_head* tmp, * bh;
    if(bh = find_buffer(dev, block)){
        return bh;
    }
    for(tmp = start_buffer; ; tmp = tmp->b_next_free){
        // printf("buffer块号：%lu,引用数：%d,脏位dirt:%d,设备号：%d\n",tmp->b_blocknr,tmp->b_count,tmp->b_dirt,tmp->b_dev);
        if(tmp->b_count){
            continue;
        }
        if(tmp->b_dirt)
            sync_dev(tmp->b_dev);
        tmp->b_count = 1;
        tmp->b_dirt = 0;
        tmp->b_uptodate = 0;
        remove_from_queues(tmp);
        tmp->b_dev = dev;
        tmp->b_blocknr = block;
        insert_into_queues(tmp);
        return tmp;
    }
}

//申请一块buffer放数据
struct buffer_head* bread(int dev, int block){
    struct buffer_head* bh;
    if(!(bh = getblk(dev, block)))
        printf("无法获取缓冲块\n");
    if(bh->b_uptodate)
        return bh;
    ll_read_block(bh);
    if(bh->b_uptodate)
        return bh;
    brelse(bh);
    return NULL;
}

//从hash表中找数据，引用计数加1
 struct buffer_head* get_hash_table(int dev, int block){
    struct buffer_head* bh;
    if(!(bh = find_buffer(dev, block))){
        return NULL;
    }
    bh->b_count++;
    return bh;    
}