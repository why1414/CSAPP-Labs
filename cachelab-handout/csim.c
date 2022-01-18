#include "cachelab.h"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>





typedef struct{
    int valid;
    int tag;
    int LRUstamp;
} cache_line, *cache_set, **cache;

cache _cache = NULL;
int S, E, B, s, b, v;
char filename[1024];
int hit, miss, evict;

void printUsage();                    /* print help messages */
void initCache();                     /* 为cache 分配内存并初始化 */
void parseTrace();                    /* 解析trace file, 判断操作类型 L or S or M */
void updateBlock(unsigned int addr);  /* 检查cache 是否命中，若未中则加载入cache */
void updateStamp();                   /* 每轮指令后，所有cache line 的时间戳加一*/

void printUsage(){
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
            "Options:\n"
            "  -h         Print this help message.\n"
            "  -v         Optional verbose flag.\n"
            "  -s <num>   Number of set index bits.\n"
            "  -E <num>   Number of lines per set.\n"
            "  -b <num>   Number of block offset bits.\n"
            "  -t <file>  Trace file.\n\n"
            "Examples:\n"
            "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
            "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void initCache(){
    _cache = (cache) malloc(S*sizeof(cache_set));
    for(int i=0; i < S; i++){
        _cache[i] = (cache_set)malloc(E*sizeof(cache_line));
        for(int j =0; j < E; j++){
            _cache[i][j].valid = 0;
            _cache[i][j].tag = -1;
            _cache[i][j].LRUstamp = -1;
        }
    }
}

void parseTrace(){
    FILE* fp = fopen(filename, "r");
    if(fp == NULL){
        printf("open fail\n");
        exit(-1);
    }

    char operation;
    unsigned int addr;
    int size;
    while(fscanf(fp," %c %x,%d\n", &operation, &addr, &size) > 0){

        if(v) printf("%c %x,%d ", operation, addr, size);

        switch(operation){
            case 'L':
                updateBlock(addr);
                break;
            case 'M':
                updateBlock(addr);
            case 'S':
                updateBlock(addr);
                break;
        }
        if(v) printf("\n");
        updateStamp();

    }

    fclose(fp);
    for(int i = 0; i < S; i++){
        free(_cache[i]);
    }
    free(_cache);

}

void updateBlock(unsigned int addr){
    // 全1 -1U  
    int setIdx = (addr>>b) & ((-1U) >> (64-s));
    int tagAddr = addr >> (b + s);

    int max_stamp = INT_MIN;
    int max_stamp_index = -1;

    for(int j = 0; j < E; j++){
        if(_cache[setIdx][j].tag == tagAddr){ /* if hit */
            _cache[setIdx][j].LRUstamp = 0; 
            hit++;
            if(v) printf("hit ");
            return ;
        }
    }

    for(int j = 0; j < E; j++){
        if(_cache[setIdx][j].valid == 0){ /* if find a free block */
            _cache[setIdx][j].valid = 1;
            _cache[setIdx][j].tag = tagAddr;
            _cache[setIdx][j].LRUstamp = 0;
            miss++;
            if(v) printf("miss ");
            return ;
        }
    }

    miss++;
    evict++;
    if(v) printf("miss eviction ");

    for(int j = 0; j < E; j++){ /* find the rarest used block */
        if(_cache[setIdx][j].LRUstamp > max_stamp){
            max_stamp = _cache[setIdx][j].LRUstamp;
            max_stamp_index = j;
        }
    }
    _cache[setIdx][max_stamp_index].tag = tagAddr;
    _cache[setIdx][max_stamp_index].LRUstamp = 0;
    _cache[setIdx][max_stamp_index].valid = 1;
    return ;

}

void updateStamp(){
    for(int i = 0; i < S; i++){
        for(int j = 0; j < E; j++){
            if(_cache[i][j].valid)
                _cache[i][j].LRUstamp++;
        }
    }
}


int main(int argc, char **argv)
{
    int opt;
    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch (opt)
        {
        case 'h':
            printUsage();
            break;
        case 'v':
            v = 1;
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            strcpy(filename, optarg);
            break;
        default:
            printUsage();
        }
    }

    if(s <= 0 || E <= 0 || b <=0 || filename == NULL){
        printf("Wrong args\n");
        exit(1);
    }

    S = 1<<s;
    B = 1<<b;

    initCache();
    parseTrace();
    

    printSummary(hit, miss, evict);
    return 0;
}
