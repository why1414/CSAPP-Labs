/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i,j;
    int row, col;
    int t0,t1,t2,t3,t4,t5,t6,t7;
    if(M == 32){
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
    }
    else if( M == 64 ){
       for(row = 0; row < N; row+=4){
        for(col = 0; col < M; col+=4){
            for(i = 0;i < 4; i++){
                /* 把 A 的cache block全部读取，减少与B 的cache block 冲突 */
                    t0 = A[row+i][col+0];
                    t1 = A[row+i][col+1];
                    t2 = A[row+i][col+2];
                    t3 = A[row+i][col+3];
                    // t4 = A[row+i][col+4];
                    // t5 = A[row+i][col+5];
                    // t6 = A[row+i][col+6];
                    // t7 = A[row+i][col+7];

                    B[col+0][row+i] = t0;
                    B[col+1][row+i] = t1;
                    B[col+2][row+i] = t2;
                    B[col+3][row+i] = t3;
                    // B[col+4][row+i] = t4;
                    // B[col+5][row+i] = t5; 
                    // B[col+6][row+i] = t6;
                    // B[col+7][row+i] = t7;
                
            }
            }
        } 
    }
    else if( M == 61 ){
        for(row = 0; row < N; row+=16){
            for(col = 0; col < M; col+=16){
                for(i = 0;i < 16 && row+i < N; i++){
                    for( j = 0 ;j < 16 && col+j < M; j++)
                        B[col+j][row+i] = A[row+i][col+j];
                }
            }
        } 
    }
    
}



/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    
    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 


}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

