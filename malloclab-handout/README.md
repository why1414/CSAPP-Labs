# Malloc Lab
此实验要求实现一个C语言动态内存分配器(a dynamic storage allocator for C programs)，即实现自己版本的：malloc，free，和realloc.

具体实现要求包含以下四个函数：
```c
int   mm_init(void);
void *mm_malloc(size_t size);
void *mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
```

## 实现
首先，根据《CSAPP》中9.9.12的样例实现分配器的mm_init()与mm_malloc().