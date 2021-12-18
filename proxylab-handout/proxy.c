#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_CACHE_NUM MAX_CACHE_SIZE/MAX_OBJECT_SIZE

typedef struct{
    char* name;
    char* body;
    int LRU;
    int isEmpty;
    sem_t *LRU_mutex; // mutex for modify LRU for each object
}object;

typedef struct{
    object* objectList[MAX_CACHE_NUM];
    int objectNum;
    sem_t *RW_mutex; // mutex for read and write on the whole cachePool
    int readcnt; // record number of readers for cache
    sem_t *RC_mutex; // readcnt mutex
    int index;  // index for LRU

}Cache;

Cache cachePool;



/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int connfd);
void parse_url(char*url, char* hostname,char* port, char* uri);
void *thread(void *vargp);
int parse_request(rio_t* rp, char* method, char* host, char* port, char* uri);
void parse_Header(rio_t* rp, char* clientbuf, char* host);
int readCache(int fd, char* url);
void writeCache(char* url, char* object);
void initCache(Cache* cachePool);


int main(int argc, char** argv)
{
    int listenfd, *connfdp;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    /*check command line args*/
    if (argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    printf("%s", user_agent_hdr);
    listenfd = Open_listenfd(argv[1]);
    initCache(&cachePool);
    while(1){
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA*) &clientaddr, clientlen, hostname, 
                    MAXLINE, port, MAXLINE, 0);
        // doit(connfd);
        Pthread_create(&tid, NULL, thread, (void*)connfdp);
        // Close(connfd);
    }
    
    return 0;
}

void* thread(void * vargp){
    int connfd = *((int*)vargp);
    Pthread_detach(Pthread_self());
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}

void doit(int connfd){
    #define PORT_LEN 20
    char clientbuf[MAXLINE], method[MAXLINE],
        host[MAXLINE], port[PORT_LEN], uri[MAXLINE];
    char url[MAXLINE], objectbuf[MAX_OBJECT_SIZE];
    int clientfd;
    rio_t rio, client_rio;
    
    Rio_readinitb(&rio, connfd);
    /*parse fisrst line of request, and close invalid request*/
    if(!parse_request(&rio, method, host, port, uri)){
        return ;
    }
    /*check cache*/
    sprintf(url,"%s%s%s", host, port, uri);
    if(readCache(connfd, url)){
        printf("find object in cache\n");
        return ;
    }
    /*build a proxy web client*/
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&client_rio, clientfd);
    /*proxy request line*/
    strcpy(clientbuf, "");
    sprintf(clientbuf, "%s %s %s\r\n", method, uri, "HTTP/1.0");
    parse_Header(&rio, clientbuf, host); // parse the rest header
    /*send proxy request to web server*/
    Rio_writen(clientfd, clientbuf, strlen(clientbuf));
    int n;
    int objectSize = 0;
    /*forward the webserver's replies to the host*/
    while((n = Rio_readlineb(&client_rio, clientbuf, MAXLINE))){
        printf("proxy server received %d bytes\n", (int)n);
        Rio_writen(connfd, clientbuf, n);
        strcpy(objectbuf+objectSize,clientbuf);
        objectSize += n;
    }
    if(objectSize < MAX_OBJECT_SIZE){
        writeCache(url, objectbuf);
    }
    Close(clientfd);
    

}

int parse_request(rio_t* rp, char* method, char* host, char* port, char* uri){
    char buf[MAXLINE], url[MAXLINE], version[MAXLINE];
    if(!Rio_readlineb(rp, buf, MAXLINE))
        return 0;
    printf("%s", buf);
    sscanf(buf, "%s %s %s",method, url, version);
    if(strcasecmp(method, "GET")){
        fprintf(stderr,"invalid method\n");
        return 0;
    }
    if(!strstr(url, "http://")){
        fprintf(stderr,"invalid URL\n");
        return 0;
    }
    parse_url(url, host, port, uri);
    return 1;
}

void parse_url(char* url, char* hostname, char* port, char* uri){
    // char *ptr;
    char* p_hostname = url+sizeof("http://")-1;
    char* p_port = index(p_hostname, ':');
    char* p_uri = index(p_hostname, '/');
    if(p_uri){
        strcpy(uri, p_uri);
        *p_uri = '\0';
    }else{
        printf("invalid uri\n");
    }
    if(p_port){
        strcpy(port, p_port+1);
        *p_port = '\0';
    }
    else{
        strcpy(port, "80");
    }
    strcpy(hostname, p_hostname);
    printf("hostname: %s\nport: %s\nuri: %s\n", 
            hostname, port, uri);
    
}

void parse_Header(rio_t* rp, char* clientbuf, char* host){
    char buf[MAXLINE], header[MAXLINE];
    /*read headr from client*/
    do{
        Rio_readlineb(rp, buf, MAXLINE);
	    printf("%s", buf);
        if(strstr(buf,"Connection:") || strstr(buf, "User-Agent:")
            || strstr(buf, "Proxy-Connection:"))
            continue;
        sprintf(header, "%s%s", header, buf);
    }while(strcmp(buf, "\r\n"));
    /*rewrite the header for proxy http request*/
    if(!strstr(header, "Host:"))
        sprintf(clientbuf,"%sHost: %s\r\n", clientbuf, host);
    sprintf(clientbuf,"%s%s\r\n", clientbuf, user_agent_hdr);
    sprintf(clientbuf,"%sConnection: close\r\n", clientbuf);
    sprintf(clientbuf,"%sProxy-Connection: close\r\n", clientbuf);
    sprintf(clientbuf, "%s%s", clientbuf, header);
    sprintf(clientbuf,"%s\r\n", clientbuf);
    return ;
    
}

void initCache(Cache* cachePool){
    cachePool->RC_mutex = (sem_t*) Malloc(sizeof(sem_t));
    cachePool->RW_mutex = (sem_t*) Malloc(sizeof(sem_t));
    Sem_init(cachePool->RW_mutex, 0, 1);
    Sem_init(cachePool->RC_mutex, 0, 1);
    cachePool->readcnt = 0;
    cachePool->objectNum = 0;
    cachePool->index = 0;
    for(int i=0;i<MAX_CACHE_NUM;i++){
        cachePool->objectList[i] =(object*) Malloc(sizeof(object));
        cachePool->objectList[i]->name = (char*) Malloc(MAXLINE);
        cachePool->objectList[i]->body = (char*) Malloc(MAX_OBJECT_SIZE);
        cachePool->objectList[i]->isEmpty = 1;
        cachePool->objectList[i]->LRU = 0;
        cachePool->objectList[i]->LRU_mutex = (sem_t*) Malloc(sizeof(sem_t));
        Sem_init(cachePool->objectList[i]->LRU_mutex, 0, 1);
    }
    printf("initalize cachePool successfully\n");
}

int readCache(int fd, char* url){
    int ret=0;
    P(cachePool.RC_mutex);
    cachePool.readcnt++;
    if(cachePool.readcnt == 1){
        P(cachePool.RW_mutex);
    }   
    V(cachePool.RC_mutex);
    int i=0;
    for(i=0;i<MAX_CACHE_NUM;i++){
        if(cachePool.objectList[i]->isEmpty)
            continue;
        if(!strcmp(url, cachePool.objectList[i]->name)){
            Rio_writen(fd, cachePool.objectList[i]->body, MAX_OBJECT_SIZE );
            P(cachePool.objectList[i]->LRU_mutex);
            cachePool.objectList[i]->LRU = 1;
            V(cachePool.objectList[i]->LRU_mutex);
            ret = 1;
            break;
        }
        
    }
    P(cachePool.RC_mutex);
    cachePool.readcnt--;
    if(cachePool.readcnt == 0){
        V(cachePool.RW_mutex);
    }
    V(cachePool.RC_mutex);
    return ret;
    // printf("search in cache finished \n");
}
void writeCache(char* url, char* object){
    P(cachePool.RW_mutex);
    int tmpidx = cachePool.index;
    /*when the cachePool is full, delete one object according to LRU*/
    if(cachePool.objectNum == MAX_CACHE_NUM){
        printf("cachePool is full, LRUing...\n");
        while(cachePool.objectList[tmpidx]->LRU){
            cachePool.objectList[tmpidx]->LRU = 0;
            tmpidx = (tmpidx + 1)%MAX_CACHE_NUM;
        }
        cachePool.objectList[tmpidx]->isEmpty = 1; //delete object when LRU equals 0
        cachePool.objectNum--;
    }
    /*search empty position tmpidx*/
    while(!cachePool.objectList[tmpidx]->isEmpty){
        tmpidx = (tmpidx + 1)%MAX_CACHE_NUM;
    }
    strcpy(cachePool.objectList[tmpidx]->name,url);
    strcpy(cachePool.objectList[tmpidx]->body, object);
    cachePool.objectList[tmpidx]->LRU = 1;
    cachePool.objectList[tmpidx]->isEmpty = 0;
    cachePool.objectNum++;
    cachePool.index = (tmpidx + 1)%MAX_CACHE_NUM;
    V(cachePool.RW_mutex);
    // printf("write cache successful, url: %s\n", url);
}


