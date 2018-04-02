/*
 * CS351: System Programming
 * Machine Problem #4 :  Cache Simulation and Optimization
 * Jay Patel | A20328510
 * */


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

void not_so_simple_blocking(int M, int N, int A[N][M], int B[M][N]);
void simple_blocking(int M, int N, int A[N][M], int B[N][M], int blocksize);
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
    switch(N) {
        case 32:
            // simple_blocking(M, N, A, B, 8); gives valid result i.e. 287 misses
            not_so_simple_blocking(M, N, A, B); // originally implemented for 64x64 but gives improved performance on 32x32 too.
            break;
        case 64:
            // simple_blocking(M, N, A, B, 4);
            not_so_simple_blocking(M, N, A, B);
            break;
        default:    
            simple_blocking(M, N, A, B, 16);
            break;
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

char not_so_simple_blocking_desc[] = "Transpose with not so simple blocking";
void not_so_simple_blocking(int M, int N, int A[N][M], int B[M][N])
{
    // temp array holds values temporarily for transposition
    int i, j, col, row, temp[8];
    for (col = 0; col < N; col += 8) {
        for (row = 0; row < N; row += 8) {
            // take care of upper half (first four rows) of the 8 x 8 block first
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 8; j++) { // store a row in temp
                    temp[j] = A[col + i][row + j];      
                }
                // place the values in B, in the first four rows with 4 out of 8 values in temp in their correct place while the rest will be dealt with later
                for (j = 0; j < 4; j++) {
                    B[row + j][col + i] = temp[j];      
                }
                for (j = 0; j < 3; j++) {
                    B[row + j][col + i + 4] = temp[j + 5];      
                }   
                B[row + 3][col + i + 4] = temp[4];
            }       
            
            // Deal with the remaining values that were not placed in their final position earlier.  
            for (j = 0; j < 4; j++) {
                temp[j] = A[col + j + 4][row + 4];      
            }   
            for (j = 0; j < 4; j++) {
                temp[j + 4] = A[col + j + 4][row + 3];      
            }
            
            for (j = 0; j < 4; j++) {
                B[row + 4][col + j] = B[row + 3][col + j + 4];      
            }   
            for (j = 0; j < 4; j++) {
                B[row + 4][col + j + 4] = temp[j];      
            }
            for (j = 0; j < 4; j++) {
                B[row + 3][col + j + 4] = temp[j + 4];      
            }

            // Deal with the lower halve (last four rows) of the 8 x 8 block      
            for (i = 0; i < 3; i++) {   
                for (j = 0; j < 4; j++) {
                    temp[j] = A[col + j + 4][row + i + 5];      
                }   
                for (j = 0; j < 4; j++) {
                    temp[j + 4] = A[col + j + 4][row + i];      
                }
            
                for (j = 0; j < 4; j++) {
                    B[row + i + 5][col + j] = B[row + i][col + j + 4];      
                }   
                for (j = 0; j < 4; j++) {
                    B[row + i + 5][col + j + 4] = temp[j];      
                }
                for (j = 0; j < 4; j++) {
                    B[row + i][col + j + 4] = temp[j + 4];      
                }   
            }


        }   
    }
} 


char simple_blocking_desc[] = "Transpose with simple blocking";
void simple_blocking(int M, int N, int A[N][M], int B[M][N], int blocksize)
{
    int col, row, blk_r, blk_c;
    int d_idx, d_val;

    for (row = 0; row < N; row += blocksize) {
        for (col = 0; col < M; col += blocksize) {
            for (blk_r = row; (blk_r < N) && (blk_r < row + blocksize); blk_r++) {
                for (blk_c = col; (blk_c < M) && (blk_c < col + blocksize); blk_c++) {
                    if (blk_r != blk_c)
                        B[blk_c][blk_r] = A[blk_r][blk_c]; 
                    else { // record diagonals
                        d_val = A[blk_r][blk_c]; 
                        d_idx = blk_r;
                    }
                }
                // place diagonals appropriately
                if(row == col)
                    B[d_idx][d_idx] = d_val;
            }
        }       
    }
}



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
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

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
