#include <stdio.h>
#include "csapp.h"
#include <assert.h>
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

typedef struct{
    char method[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];
    char host[MAXLINE];
    char port[MAXLINE];
    char path[MAXLINE];
}reqLine;

typedef struct{
    char* rqst_line;
    char* hdr_lines;
}reqHeader;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";


void doit(int client_fd);
void *thread(void *vargp);
void parse_request(char* buf, reqLine* request_line);
void parse_uri(char* uri,char* host,char* port, char* path);
void parse_Header(rio_t* rp, char* proxybuf, reqLine* request_line);
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

void doit(int client_fd){
    #define PORT_LEN 20
    char clientbuf[MAXLINE], objectbuf[MAX_OBJECT_SIZE], proxybuf[MAXLINE];
    reqHeader new_hdr;
    reqLine request_line;
    int proxy_fd;
    rio_t proxy_rio, client_rio;
    
    /*parse request line and header*/
    Rio_readinitb(&client_rio, client_fd);
    if (!Rio_readlineb(&client_rio, clientbuf, MAXLINE))
        return;
    printf("%s", clientbuf);
    parse_request(clientbuf, &request_line);
    printf("host: %s\nport: %s\npath: %s\n", request_line.host, 
    request_line.port, request_line.path);
    /*check cache*/
    if(readCache(client_fd, request_line.uri)){
        printf("find object in cache\n");
        return ;
    }
    printf("Not in cache, build a connection\n");
    /*build a proxy web client*/
    proxy_fd = Open_clientfd(request_line.host, request_line.port);
    Rio_readinitb(&proxy_rio, proxy_fd);
    parse_Header(&client_rio, proxybuf, &request_line); // parse the rest header
    /*send proxy request to web server*/
    printf("%s",proxybuf);
    Rio_writen(proxy_fd, proxybuf, strlen(proxybuf));
    int n;
    int objectSize = 0;
    /*forward the webserver's replies to the host*/
    while((n = Rio_readlineb(&proxy_rio, proxybuf, MAXLINE))){
        printf("proxy server received %d bytes\n", (int)n);
        Rio_writen(client_fd, proxybuf, n);
        strcpy(objectbuf+objectSize,clientbuf);
        objectSize += n;
    }
    if(objectSize < MAX_OBJECT_SIZE){
        writeCache(request_line.uri, objectbuf);
    }
    Close(proxy_fd);
    

}

void parse_request(char* buf, reqLine* request_line){
    char* method = request_line->method;
    char* uri =   request_line->uri;
    char* version = request_line->version;
    char* host = request_line->host;
    char* port = request_line->port;
    char* path = request_line->path;
    sscanf(buf, "%s %s %s", method, uri, version);
    assert(!strcasecmp(method, "GET"));
    parse_uri(uri, host, port, path);

}


void parse_uri(char* uri, char* host, char* port, char* path){
    assert(strstr(uri, "http://"));
    char buf[MAXLINE];
    strcpy(buf, uri);
    char* ptr_host = buf+strlen("http://");
    char* ptr_port = strstr(ptr_host, ":");
    char* ptr_path = strstr(ptr_host, "/");
    assert(ptr_path);
    *ptr_path = '\0';
    if(ptr_port){
        *ptr_port = '\0';
        strcpy(port, ptr_port+1);    
    }
    else
        strcpy(port, "80");
    strcpy(host, ptr_host);
    *ptr_path = '/';
    strcpy(path, ptr_path);
}

void parse_Header(rio_t* rp, char* proxybuf, reqLine* request_line){
    char buf[MAXLINE];
    /*build proxy request line*/
    strcpy(proxybuf, "");
    sprintf(proxybuf, "%s %s %s\r\n", request_line->method, 
    request_line->path, "HTTP/1.0");
    sprintf(proxybuf,"%sHost: %s\r\n", proxybuf, request_line->host);
    sprintf(proxybuf,"%s%s", proxybuf, user_agent_hdr);
    sprintf(proxybuf,"%s%s", proxybuf, conn_hdr);
    sprintf(proxybuf,"%s%s", proxybuf, proxy_conn_hdr);
    /*read headr from client*/
    while(Rio_readlineb(rp, buf, MAXLINE) > 0){
        if(!strcmp(buf, "\r\n")) break;
        printf("%s", buf);
        if(strstr(buf,"Connection:") || strstr(buf, "User-Agent:")
            || strstr(buf, "Proxy-Connection:") || strstr(buf,"Host:"))
            continue;
        sprintf(proxybuf, "%s%s", proxybuf, buf);
    }
    sprintf(proxybuf,"%s\r\n", proxybuf);
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
    printf("search in cache finished \n");
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


