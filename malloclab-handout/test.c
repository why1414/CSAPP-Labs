#include <stdio.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"
int main(){
    mem_init();
    char *p = (char*)mm_malloc(20);
    strcpy(p,"hello world\n");
    printf("%s",p);
    mm_free(p);
    return 0;
}