// Sudoku puzzle verifier and solver

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct{
  int **grid;
  int psize;
  int val; // Can represent either row, col, or box number
  bool *result;
} ThreadData;

bool missingInRow(int **grid, int psize, int row, int val){
  for (int col = 1; col <= psize; col++){
    if (grid[row][col] == val){
      return false;
    }
  }
  return true;
}

bool missingInColumn(int **grid, int psize, int col, int val){
  for (int row = 1; row <= psize; row++){
    if (grid[row][col] == val){
      return false;
    }
  }
  return true;
}

bool missingInBox(int **grid, int psize, int startRow, int startCol, int val){
  int boxSize = sqrt(psize);
  for (int i = 0; i < boxSize; i++){
    for (int j = 0; j < boxSize; j++){
      if (grid[startRow + i][startCol + j] == val){
        return false;
      }
    }
  }
  return true;
}

void *checkRow(void *arg){
  ThreadData *data = (ThreadData *)arg;
  int **grid = data->grid;
  int psize = data->psize;
  int row = data->val;
  for(int i = 1; i <= psize; i++){
    if(missingInRow(grid, psize, row, i)){
      *(data->result) = false;
      pthread_exit(NULL);
    }
  }
  *(data->result) = true;
  pthread_exit(NULL);
}

void *checkCol(void *arg){
  ThreadData *data = (ThreadData *)arg;
  int **grid = data->grid;
  int psize = data->psize;
  int col = data->val;
  for(int i = 1; i <= psize; i++){
    if(missingInCol(grid, psize, col, i)){
      *(data->result) = false;
      pthread_exit(NULL);
    }
  }
  *(data->result) = true;
  pthread_exit(NULL);
}

void *checkBox(void *arg){
  ThreadData *data = (ThreadData *)arg;
  int **grid = data->grid;
  int psize = data->psize;
  int index = data->val;

  int boxSize = sqrt(psize);
  int startRow = (index - 1) / boxSize * boxSize + 1;
  int startCol = (index - 1) % boxSize * boxSize + 1;

  for (int i = 1; i <= psize; i++){
    if (missingInBox(grid, psize, startRow, startCol, i)){
      *(data->result) = false;
      pthread_exit(NULL);
    }
  }
  *(data->result) = true;
  pthread_exit(NULL);
}

bool checkValid(int **grid, int psize){
  pthread_t threads[3 * psize];
  bool results[3 * psize];
  ThreadData data[3 * psize];

  // for each in psize launch a thread for col, row and box
  for (int i = 1; i <= psize; i++){
    data[i - 1].grid = grid;
    data[i - 1].psize = psize;
    data[i - 1].val = i;
    data[i - 1].result = &results[i - 1];
    pthread_create(&threads[i - 1], NULL, checkRow, &data[i - 1]);

    data[i + psize - 1].grid = grid;
    data[i + psize - 1].psize = psize;
    data[i + psize - 1].val = i;
    data[i + psize - 1].result = &results[i + psize - 1];
    pthread_create(&threads[i + psize - 1], NULL, checkCol, &data[i + psize - 1]);

    data[i + 2 * psize - 1].grid = grid;
    data[i + 2 * psize - 1].psize = psize;
    data[i + 2 * psize - 1].val = i;
    data[i + 2 * psize - 1].result = &results[i + 2 * psize - 1];
    pthread_create(&threads[i + 2 * psize - 1], NULL, checkBox, &data[i + 2 * psize - 1]);

  }

  // Join the threads
  for(int i = 0; i < 3 * psize; i++){
    pthread_join(threads[i], NULL);
    if(!results[i]){
      return false;
    }
  }
  return true;
}


// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  bool edited;
  *complete = true; //assuming puzzle is complete by default
  edited = true;
  // first we check if puzzle is complete then we check if it is valid
  while(edited){ // using edited as a flag to see if we have to check through again

    edited = false;
    for (int row = 1; row <= psize; row++) {
      for (int col = 1; col <= psize; col++) {

        if (grid[row][col] == 0) {
          *complete = false;
          fillCell(psize, grid, row, col);

          if(grid[row][col] != 0){
            edited = true;
          }
        }
      }
    }
  }
  // puzzle can only be valid if complete
  if(*complete){
    *valid = checkValid(grid, psize);
  }
  
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
