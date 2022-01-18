# Cache Lab

Cache lab 包含两部分，part A: 实现一个 Cache simulator，part B: 优化一个小矩阵转置函数，使其cache miss 数量最小。

## PART A

实现一个 cache 模拟器，接受valgrind memory trace 作为输入，模拟cache 上的 hit/miss 行为。当cache满了的时候，实行LRU替换策略。（该模拟器只需要模拟hit/miss/eviction的行为，并不需要实际去缓存数据。）

## 要点：cache 结构

1. cache 结构： 一个cache由 S = 2^s 个cache set组成，每个cache set又包含E个cache line。每隔cache line包含：一个valid bit 指明该cache line中是否包含有效数据，t = m-(b+s)个 tag bit可以唯一的标识存储在该cache line 中的block，还有一个block存储数据(大小为：B = 2^b)。如下所示：


    ```c
    typedef struct{
        int valid;    /* valid bit*/
        int tag;      /* tag bits ， 其实就是该blcok地址的前t位*/
        int LRUstamp; /* 用于实现LRU的时间戳 */
        /* char blcok[B] 本实验不需要实现 */
    } cache_line,
    ```

2. cache simulator实现解析命令行参数，实验文档中推荐 用getopt来解析参数，具体可以查看 man 手册 "man 3 getopt"。

3. LRU替换策略：当cache满了后，要添加新的block，就需要替换已经存在的blcok。LRU策略( Least Recently Used) 选取cache中上次访问时间最久远的block，将他替换掉。本实验PART A部分参考 [LeyN](https://zhuanlan.zhihu.com/p/79058089), 采用是: 每个cache line 拥有一个LRUstamp时间戳，以 LRUstamp=0 表示当前时间，也就是最新时间。每访问一次 cache 后 (L/S/M)，将所有cache line的LRUstamp 统一增。LRUstamp 越大，说明上一次访问该cache line的时间越久远，替换时选择LRUstamp最大的cache line即可。

## PART B

优化一个矩阵转置函数，也是本实验最终的部分，可以帮助更好的理解cache的工作方式。

* 实验中给定的cache 参数为：(s = 5, E = 1, b = 5)

>s = 5: 表明该cache一共有 2^5=32 个cache set，每个地址的中间s位地址作为这32个cache set的索引。

>E = 1: 表明每个set中只有一个block/cache line

>b = 5: 表明每个block的大小为 2^5=32 bytes.


## 要点：分块
1. 32 × 32 (M = 32, N = 32)
   
    数组一行有32个int(4 bytes)，因此一行需要4个block才装得下。整个cache只能装下8行该数组。因为数组地址是连续的，且该数组的长宽是block的整数倍。因此数组中每隔8行，在相同列的元素就会被映射到同一个cache set/block中, i.e., A[0][0] 与 A[8][0] 会映射到相同block中。当先读A[0][0], A[0][0]-A[0][7]会被整体读入cache中，再读A[8][0]就会导致cache中的A[0][0]被evict。

    为了避免每8行发生evict, 我们将分块的大小设置为8 * 8，32 * 32矩阵被分为了16个子矩阵。理论 miss数：16 * 8 * 2 =256

    然而，除了数组A可能与自己发生evict以外，因为数组B也与A共用同一cache，也可能发生数组A缓存的block被数组B中的block替换。实验中分析，显示A B相同下标的元素会被映射到同一个block中。在转置函数的内循环中，先访问 A[i][j] 再访问 B[j][i]，当 i = j 时，就会导致多余的evict。(实际上只有对线上的元素才会造成转置过程中 A B 映射冲突)

    解决方法：在函数中定义 8 个局部变量，每次读入一个A block时，将该block中所有值存入8个局部变量中。然后，一次性将该8个值付给数组B的相应元素。这样可以有效的避免数组A的某个block，被数组B evict掉导致的miss.

    ```c
    for(row = 0; row < N; row+=8){
        for(col = 0; col < M; col+=8){
            for(i = 0;i < 8; i++){
                if(row == col){
                /* 把 A 的cache block全部读取，减少与B 的cache block 冲突 */
                    t0 = A[row+i][col+0];
                    t1 = A[row+i][col+1];
                    t2 = A[row+i][col+2];
                    t3 = A[row+i][col+3];
                    t4 = A[row+i][col+4];
                    t5 = A[row+i][col+5];
                    t6 = A[row+i][col+6];
                    t7 = A[row+i][col+7];

                    B[col+0][row+i] = t0;
                    B[col+1][row+i] = t1;
                    B[col+2][row+i] = t2;
                    B[col+3][row+i] = t3;
                    B[col+4][row+i] = t4;
                    B[col+5][row+i] = t5; 
                    B[col+6][row+i] = t6;
                    B[col+7][row+i] = t7;
                }
                else {
                    for( j = 0 ; j < 8; j++ )
                        B[col+j][row+i] = A[row+i][col+j];
                }
                
            }
        }
    }
    ```

2. 64 × 64 (M = 64, N = 64)
   
   与32 * 32数组的分析类似，64 * 64数组的不同点在于全部的cache只能装下该数组的4行。这就导致数组中同一列的元素，每隔4行就会被映射到同一个cache line / block中，也就是冲突。因此，这里采用的是 4 * 4 的子矩阵分块，一共分为 16*16= 256个子矩阵。按此分块法：因为A是按行读，每个block仍然是存储8个int，因此A的理论miss率为 1/8，但B是按列读，因此其理论miss率下降为4.
   该方法的理论最优miss数为：(64*64)/8 + (64 * 64)/4 = 1536. 然后再利用局部变量优化A B映射冲突问题。

   注：此方法实现不了实验的满分要求 1300，但比较简单易懂，优化后的 miss数为1699.

3. 61 × 67 (M = 61, N = 67)
   
   该矩阵由于不对称，且行/列不是block的整数倍，因此相同数组中的同一列的元素不存在相差4行就必定冲突的情况(具体冲突情况有待分析)。直接分块为8的倍数，参考网上资料选择了16*16.

    ```c
    for(row = 0; row < N; row+=16){
                for(col = 0; col < M; col+=16){
                    for(i = 0;i < 16 && row+i < N; i++){
                        for( j = 0 ;j < 16 && col+j < M; j++)
                            B[col+j][row+i] = A[row+i][col+j];
                    }
                }
            }
    ```

## SCORE

    Part A: Testing cache simulator
    Running ./test-csim
                            Your simulator     Reference simulator
    Points (s,E,b)    Hits  Misses  Evicts    Hits  Misses  Evicts
        3 (1,1,1)       9       8       6       9       8       6  traces/yi2.trace
        3 (4,2,4)       4       5       2       4       5       2  traces/yi.trace
        3 (2,1,4)       2       3       1       2       3       1  traces/dave.trace
        3 (2,1,3)     167      71      67     167      71      67  traces/trans.trace
        3 (2,2,3)     201      37      29     201      37      29  traces/trans.trace
        3 (2,4,3)     212      26      10     212      26      10  traces/trans.trace
        3 (5,1,5)     231       7       0     231       7       0  traces/trans.trace
        6 (5,1,5)  265189   21775   21743  265189   21775   21743  traces/long.trace
        27


    Part B: Testing transpose function
    Running ./test-trans -M 32 -N 32
    Running ./test-trans -M 64 -N 64
    Running ./test-trans -M 61 -N 67

    Cache Lab summary:
                            Points   Max pts      Misses
    Csim correctness          27.0        27
    Trans perf 32x32           8.0         8         287
    Trans perf 64x64           3.4         8        1699
    Trans perf 61x67          10.0        10        1992
            Total points    48.4        53

## References

https://zhuanlan.zhihu.com/p/79058089

https://www.cnblogs.com/liqiuhao/p/8026100.html?utm_source=debugrun&utm_medium=referral


