#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//#define N 10
//#define M 15
//#define STEPS 100


/*The Game of Life, also known simply as Life, is a cellular automaton devised by the British mathematician John Horton Conway in 1970.
The "game" is a zero-player game, meaning that its evolution is determined by its initial state, requiring no further input.
One interacts with the Game of Life by creating an initial configuration and observing how it evolves, or, for advanced "players," by creating patterns with particular properties.
The universe of the Game of Life is an infinite two-dimensional orthogonal grid of square cells, each of which is in one of two possible states, alive or dead, or "populated" or "unpopulated".
We can suppose to use a matrix of char, which marks if the cell is alive or dead.
Every cell interacts with its eight neighbors, which are the cells that are horizontally, vertically, or diagonally adjacent. At each step in time, the following transitions occur:
- Any live cell with fewer than 2 live neighbors dies as if caused by underpopulation.
- Any live cell with 2 or 3 live neighbors lives on to the next generation.
- Any live cell with more than 3 live neighbors dies, as if by overpopulation.
- Any dead cell with exactly 3 live neighbors becomes a live cell, as if by reproduction.
The initial pattern constitutes the seed of the system. The first generation is created by applying the above rules simultaneously to every cell in the seed—births and deaths occur simultaneously,
 and the discrete moment at which this happens is sometimes called a tick (in other words, each generation is a pure function of the preceding one).
The rules continue to be applied repeatedly to create further generations.

m4.large
*/

int N, M, STEPS;

void matrixInit(char ** matrix_input){

    //srand(time(NULL));
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
}

char **allocMatrix(int rows, int cols) {
    char *data = (char *)malloc(rows*cols*sizeof(char));
    char **matrix= (char **)malloc(rows*sizeof(char*));
    for (int i=0; i<rows; i++)
        matrix[i] = &(data[cols*i]);

    return matrix;
}

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

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    N = atoi(argv[1]);
    M = atoi(argv[2]);
    STEPS = atoi(argv[3]);


    struct {
        int dx;
        int dy;
    } directions[] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

    int dest, source, rc, count, tag=1;
    int world_size, world_rank, cells_alive = 0, cells_dead = 0;;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Request request_send = MPI_REQUEST_NULL;
    MPI_Request request_receive = MPI_REQUEST_NULL;
    MPI_Request request_receive1 = MPI_REQUEST_NULL;

    if (world_size > N && world_rank == 0) {
        printf("Impossible to execute with a number of processor greater than the number of rows\n");
        MPI_Finalize();
        return 0;
    } else if (world_size > N && world_rank != 0) {
        MPI_Finalize();
        return 0;
    }

    char buffer[N][M];
    int size = N / (world_size);
    int mod = N % (world_size);

    int time = MPI_Wtime();
 
    if (world_rank == 0) {

        char **matrix = allocMatrix(N, M);
        //printf("\nMatrice in input\n");
        matrixInit(matrix);

        int step = 0, i, j, k;
        int index[world_size-1];


        for (i = 0; i < world_size; i++) {
            index[i] = size;
            if (i < mod)
                index[i]++;
        }

        tag = 1;

        for (i = 1; i < world_size; i++) {

            j = (i) * index[i-1];

            int end = j + index[i];
            if (i > mod) {
                j += mod;
                end += mod;
            }
            
            int send = end - j;
            char **out = allocMatrix(send, M);

            int l, m = 0;

            
            for (k = j; k < end; k++) {
                for (l = 0; l < M; l++) {
                    out[m][l] = matrix[k][l];
                }
                m++;
            }
            
            dest = i;
                        
            MPI_Send(&(out[0][0]), send*M, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
            free(out);
            
        }

        char **borders = allocMatrix(2, M);
        char **borders_received = allocMatrix(2, M);

        for (i = 0; i < M; i++) {
            borders[0][i] = matrix[0][i];
        }
        
        if (index[0] > 1) {
            for (i = 0; i < M; i++)
                borders[1][i] = matrix[index[0]-1][i];
        } else {
            for (i = 0; i < M; i++)
                borders[1][i] = matrix[0][i];
        }

        step = 0;
        char final[N][M];
        char temp[index[0]+2][M];
        int interior = 0;
        k = 0;

        for (i = 1; i < index[0]+1; i++) {
            for (j = 0; j < M; j++) {
                temp[i][j] = matrix[k][j];
            }
            k++;
        }


        while(1) {

            //Sending and then receiving border info to neighbours
            MPI_Isend(&borders[0][0], M, MPI_CHAR, world_size-1, tag, MPI_COMM_WORLD, &request_send);
            MPI_Isend(&borders[1][0], M, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &request_send);
            MPI_Irecv(&borders_received[0][0], M, MPI_CHAR, world_size-1, tag, MPI_COMM_WORLD, &request_receive);
            MPI_Irecv(&borders_received[1][0], M, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &request_receive1);

            k = 0;

            if (step != 0) {
                for (i = 1; i < index[0]+1; i++) {
                    for (j = 0; j < M; j++) {
                        temp[i][j] = final[k][j];
                    }
                    k++;
                }
            }
 

            int d, x, y;
            k = 1;

            //update interior portion
            if (index[0] > 2) {
                interior = 1;
                for (i = 2; i < index[0]; i++) {
                    for (j = 0; j < M; j++) {
                        for(d = 0; d < 8; d++) {
                            x = i + directions[d].dx;
                            y = j + directions[d].dy;
                            if (isInMatrix(x,y)) {
                                if (temp[x][y] == 'd') {
                                    cells_dead++;
                                } else if(temp[x][y] == 'a') {
                                    cells_alive++;
                                }
                            }
                        }
                        final[k][j] = cellChecker(temp[i][j], cells_alive, cells_dead);
                        cells_dead = 0;
                        cells_alive = 0;
                    }
                    k++;
                }
            }

            //update  borders

            MPI_Wait(&request_receive, &status);
            MPI_Wait(&request_receive1, &status);

            x = 0;
            y = index[0]+1;

            if (world_size == 2){
                x = index[0]+1;
                y = 0;
            }
            for (i = 0; i < M; i++) {
                temp[x][i] = borders_received[0][i];
            }

            for (i = 0; i < M; i++) {
                temp[y][i] = borders_received[1][i];
            }

            if (interior == 1) {

                k = 0;
                for (j = 0; j < M; j++) {
                        for(d = 0; d < 8; d++) {
                            x = 1 + directions[d].dx;
                            y = j + directions[d].dy;
                            if (isInMatrix(x,y)) {
                                if (temp[x][y] == 'd') {
                                    cells_dead++;
                                } else if(temp[x][y] == 'a') {
                                    cells_alive++;
                                }
                            }
                        }
                        final[k][j] = cellChecker(temp[1][j], cells_alive, cells_dead);
                        cells_dead = 0;
                        cells_alive = 0;
                }

                k = index[0]-1;

                for (j = 0; j < M; j++) {
                        for(d = 0; d < 8; d++) {
                            x = index[0] + directions[d].dx;
                            y = j + directions[d].dy;
                            if (isInMatrix(x,y)) {
                                if (temp[x][y] == 'd') {
                                    cells_dead++;
                                } else if(temp[x][y] == 'a') {
                                    cells_alive++;
                                }
                            }
                        }
                        final[k][j] = cellChecker(temp[index[0]][j], cells_alive, cells_dead);
                        cells_dead = 0;
                        cells_alive = 0;
                }
            } else {
                k = 0;

                for (i = 1; i < index[0]+1; i++) {
                    for (j = 0; j < M; j++) {
                        for(d = 0; d < 8; d++) {
                            x = i + directions[d].dx;
                            y = j + directions[d].dy;
                            if (isInMatrix(x,y)) {
                                if (temp[x][y] == 'd') {
                                    cells_dead++;
                                } else if(temp[x][y] == 'a') {
                                    cells_alive++;
                                }
                            }
                        }
                        final[k][j] = cellChecker(temp[i][j], cells_alive, cells_dead);
                        cells_dead = 0;
                        cells_alive = 0;
                    }
                    k++;
                }
            }

            for (i = 0; i < M; i++) {
                borders[0][i] = final[0][i];
            }
        
            if (index[0] > 1) {                
                for (i = 0; i < M; i++)
                    borders[1][i] = final[index[0]-1][i];
            } else {
                for (i = 0; i < M; i++)
                    borders[1][i] = final[0][i];
            }

            step++;

            if (step >= STEPS) {
                free(borders);
                free(borders_received);
                break;
            }
        }

        int sum = index[0];
        for (i = 1; i < world_size; i++) {
            MPI_Recv(&final[sum][0], N*M, MPI_CHAR, i, tag, MPI_COMM_WORLD, &status);
            rc = MPI_Get_count(&status, MPI_CHAR, &count);
            sum += count/M;
        }

        /*printf("\nFinal Matrix\n");
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                printf("%c", final[i][j]);
            }
            printf("\n");
        }*/

        free(matrix[0]);
        free(matrix);

    } else {

        rc = MPI_Recv(&buffer[0][0], N*M, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
        rc = MPI_Get_count(&status, MPI_CHAR, &count);
        int n = count/M, i, j;

        char **borders = allocMatrix(2, M);
        char **borders_received = allocMatrix(2, M);

        for (i = 0; i < M; i++) {
            borders[0][i] = buffer[0][i];
        }
        
        if (n > 1) {
            for (i = 0; i < M; i++)
                borders[1][i] = buffer[n-1][i];
        } else {
            for (i = 0; i < M; i++)
                borders[1][i] = buffer[0][i];
        }

        int step = 0, k, interior = 0;
        source = world_rank-1;
        dest = world_rank+1;
        if (world_rank == world_size-1)
            dest = 0;

        char temp[n+2][M];

        while(1) {

            //Sending and then receiving border info to neighbours
            MPI_Isend(&borders[0][0], M, MPI_CHAR, source, tag, MPI_COMM_WORLD, &request_send);
            MPI_Isend(&borders[1][0], M, MPI_CHAR, dest, tag, MPI_COMM_WORLD, &request_send);
            MPI_Irecv(&borders_received[0][0], M, MPI_CHAR, source, tag, MPI_COMM_WORLD, &request_receive);
            MPI_Irecv(&borders_received[1][0], M, MPI_CHAR, dest, tag, MPI_COMM_WORLD, &request_receive1);

            k = 0;

            for (i = 1; i < n+1; i++) {
                for (j = 0; j < M; j++) {
                    temp[i][j] = buffer[k][j];
                }
                k++;
            }

            int d, x, y;
            k = 1;

            if(n > 2) {
                interior = 1;
                for (i = 2; i < n; i++) {
                    for (j = 0; j < M; j++) {
                        for(d = 0; d < 8; d++) {
                            x = i + directions[d].dx;
                            y = j + directions[d].dy;
                            if (isInMatrix(x,y)) {
                                if (temp[x][y] == 'd') {
                                    cells_dead++;
                                } else if(temp[x][y] == 'a') {
                                    cells_alive++;
                                }
                            }
                        }
                        buffer[k][j] = cellChecker(temp[i][j], cells_alive, cells_dead);
                        cells_dead = 0;
                        cells_alive = 0;
                    }
                    k++;
                }
            }

            //update  borders

            MPI_Wait(&request_receive, &status);
            MPI_Wait(&request_receive1, &status);


            x = 0;
            y = n+1;
            if (world_size == 2){
                x = n+1;
                y = 0;
            }
            
            for (i = 0; i < M; i++) {
                temp[x][i] = borders_received[0][i];
            }

            for (i = 0; i < M; i++) {
                temp[y][i] = borders_received[1][i];
            }

            if (interior == 1) {
                k = 0;
                for (j = 0; j < M; j++) {
                        for(d = 0; d < 8; d++) {
                            x = 1 + directions[d].dx;
                            y = j + directions[d].dy;
                            if (isInMatrix(x,y)) {
                                if (temp[x][y] == 'd') {
                                    cells_dead++;
                                } else if(temp[x][y] == 'a') {
                                    cells_alive++;
                                }
                            }
                        }
                        buffer[k][j] = cellChecker(temp[1][j], cells_alive, cells_dead);
                        cells_dead = 0;
                        cells_alive = 0;
                }

                k = n-1;


                for (j = 0; j < M; j++) {
                        for(d = 0; d < 8; d++) {
                            x = n + directions[d].dx;
                            y = j + directions[d].dy;
                            if (isInMatrix(x,y)) {
                                if (temp[x][y] == 'd') {
                                    cells_dead++;
                                } else if(temp[x][y] == 'a') {
                                    cells_alive++;
                                }
                            }
                        }
                        buffer[k][j] = cellChecker(temp[n][j], cells_alive, cells_dead);
                        cells_dead = 0;
                        cells_alive = 0;
                }
            } else {

                k=0;
                for (i = 1; i < n+1; i++) {
                    for (j = 0; j < M; j++) {
                        for(d = 0; d < 8; d++) {
                            x = i + directions[d].dx;
                            y = j + directions[d].dy;
                            if (isInMatrix(x,y)) {
                                if (temp[x][y] == 'd') {
                                    cells_dead++;
                                } else if(temp[x][y] == 'a') {
                                    cells_alive++;
                                }
                            }
                        }
                        buffer[k][j] = cellChecker(temp[i][j], cells_alive, cells_dead);
                        cells_dead = 0;
                        cells_alive = 0;
                    }
                    k++;
                }

            }

            for (i = 0; i < M; i++) {
                borders[0][i] = buffer[0][i];
            }
        
            if (n > 1) {                
                for (i = 0; i < M; i++)
                    borders[1][i] = buffer[n-1][i];
            } else {
                for (i = 0; i < M; i++)
                    borders[1][i] = buffer[0][i];
            }


            step++;

            if (step >= STEPS) {
                tag = 1;
                MPI_Send(&(buffer[0][0]), n*M, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
                free(borders);
                free(borders_received);
                break;
            }
        }
 
    }

    MPI_Barrier(MPI_COMM_WORLD);
    float etime = MPI_Wtime() - time;
    MPI_Finalize();

    if (world_rank == 0) {
        printf("\nEnded\n");
        printf("Time: %1.2f\n", etime);
    }
}