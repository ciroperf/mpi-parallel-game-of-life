#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define N 200
#define M 300
#define STEPS 200
/*
This version of the program runs game of life, locally with a single thread

- Any live cell with fewer than **2** live neighbors dies as if caused by underpopulation.
- Any live cell with **2** or **3** live neighbors lives on to the next generation.
- Any live cell with more than **3** live neighbors dies, as if by overpopulation.
- Any dead cell with exactly **3** live neighbors becomes a live cell, as if by reproduction.
*/

char cellChecker(char current_cell, int alive, int dead) {
    char out;

    if (current_cell == 'a') {
        if (alive < 2)
            out = 'd';
        else if (alive == 2 || alive == 3)
            out = 'a';
        else if (alive > 3)
            out = 'd';
    } else {
        if (alive == 3)
            out = 'a';
        else
            out = 'd';
    }

    return out;
}

int isInMatrix(int x, int y) {

    if ((x >= 0 && x < N) && (y >= 0 && y < M))
        return 1;
    else
        return 0;
}


int main(int argc, char *argv[]) {

    char matrix_input[N][M];
    char matrix_output[N][M];

    srand((unsigned)0);
    int i,j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if ((rand() % 2) == 0) 
                matrix_input[i][j] = 'd';
            else
                matrix_input[i][j] = 'a';
            
            //printf("%c", matrix_input[i][j]);
        }
        
        //printf("\n");
    }

    int exit = 1;
    int cells_alive = 0;
    int cells_dead = 0;

    struct {
        int dx;
        int dy;
    } directions[] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

    int x, y, k;
    clock_t begin = clock();
    for(exit = 0; exit < STEPS; exit++) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < 8; k++) {
                    x = i + directions[k].dx;
                    y = j + directions[k].dy;

                    if (x < 0) {         
                        x = N-1;
                    } else if (x >= N)
                        x = 0;
                    if (isInMatrix(x,y)) {
                        if (matrix_input[x][y] == 'd') {
                            cells_dead++;
                        } else if(matrix_input[x][y] == 'a') {
                            cells_alive++;
                        }
                    }                   
                }
                matrix_output[i][j] = cellChecker(matrix_input[i][j], cells_alive, cells_dead);
                cells_dead = 0;
                cells_alive = 0;       
            }  
        }

        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                matrix_input[i][j] = matrix_output[i][j];
            }
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("\nMatrice risultante\n");
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            printf("%c", matrix_output[i][j]);
        }
        printf("\n");
    }
    printf("Time elapsed: %fs\n", time_spent);

    return 0;

}
