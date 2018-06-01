#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "LinkedList.h"
#include "webutils.h"

/**
 * @Author: CJ Maxwell - NJIT
 * @Last Updated: 4/25/17
 * @Title: C-based Ubiquitously Recursive Script for Evaluating Sudoku (CURSES)
 *
 * *** Dependent on included LinkedList.h header file & Data_Structures.so shared library ***
 *
 * NOTE: All mistakes made while finding solution will be logged to STDERR.
 *          It is wise to run program suppressing / redirecting errors to file.
 * */

typedef struct Box{LinkedList my_row, my_column, my_sector; int rownum, columnnum, sectornum;}Box;

void print(char *board[9][9]){
    int i,j;
    for (i=0; i < 9; i++){
        for(j=0; j< 9; j++)
            printf("%s ", board[i][j]);
        printf("\n");
    }
    printf("\n");
}

void pprintf(FILE* fd, char *board[9][9]){
    fprintf(fd,"+-+-+-+-+-+-+-+-+-+\n");
    int i,j;
    for (i=0;i<9;i++){
        for (j=0;j<9;j++)
            if (board[i][j] == "0")
                fprintf(fd,"| ");
            else
                fprintf(fd,"|%s",board[i][j]);
        fprintf(fd,"|\n+-+-+-+-+-+-+-+-+-+\n");
    }
    fprintf(fd,"\n");
}

void make9x9(FILE * fl, char * board[9][9]){
    int col=0, row = -1;
    size_t  size = 21;
    char *line=(char*)malloc(21);
    while (~getline(&line, &size, fl))
        if (line[0]=='+')
            continue;
        else
            for (col=1, row++; col < size; col+=2)
                if (line[col] == ' '){
                    board[row][col/2]="0";
                }
                else if (line[col] == '\n')
                    break;
                else {
                    board[row][col/2]=malloc(2);
                    board[row][col/2][0]=line[col];
                    board[row][col/2][1]='\0';
                }
}

void transform(char *orig[9][9], char *transform[9][9]) {
    int i,j;
    for (i=0; i<9;i++)
        for (j=0; j< 9; j++)
            transform[j][i]=orig[i][j];
}

int findSector(int i, int j){
    switch (i) {
        case 0: case 1: case 2:
            switch (j) {
                case 0: case 1: case 2:
                    return 0;
                case 3: case 4: case 5:
                    return 1;
                default:
                    return 2;
            }
        case 3: case 4: case 5:
            switch (j) {
                case 0: case 1: case 2:
                    return 3;
                case 3: case 4: case 5:
                    return 4;
                default:
                    return 5;
            }
        default:
            switch (j) {
                case 0: case 1: case 2:
                    return 6;
                case 3: case 4: case 5:
                    return 7;
                default:
                    return 8;
            }
    }
} //(open me to laugh [programmer humor is an aquire taste])

void subdivide(char *board[9][9], char *boxes[9][9]) {
    int i,j,section, sizes[9]={0,0,0,0,0,0,0,0,0};
    for (i=0; i<9;i++)
        for (j=0; j< 9; j++) {
            section = findSector(i, j);
            boxes[section][sizes[section]++]=board[i][j];
        }
}

int findIntInList(int num, LinkedList list){
    int count = 0;
    Link * tmp = list.head;
    while (tmp){
        if (tmp->type==INT && tmp->payload.i==num)
            count++;
        tmp=tmp->next;
    }
    return count;
}

int getWeight(LinkedList list){
    int count = 9;
    Link * tmp = list.head;
    while(tmp){
        if (tmp->type==INT && tmp->payload.i==0)
            count--;
        tmp=tmp->next;
    }
    return count;
}

int updatePointers(Box** boi, int guess){
    Box * boa = *boi;
    Link * curr;
    int j, row, col, offset;

    row=boa->rownum; col=boa->columnnum;
    offset=col%3 + 3 * (row%3); //num n items away from top left box of a sector

    for (j=0, curr=boa->my_row.head; j < col; j++, curr=curr->next); //note the semicolon, This syntax advances curr n items from head

    if (!curr->payload.i || guess == 0) //disallow overwriting of filled boxes. Zero is treated as an exception and is considered like an erase mark
        curr->payload.i=guess;
    else
        return 0;

    for (j=0, curr=boa->my_column.head; j < row; j++, curr=curr->next); // same here

    if (!curr->payload.i || guess == 0)
        curr->payload.i=guess;
    else
        return 0;

    for (j=0, curr=boa->my_sector.head; j < offset; j++, curr=curr->next); //and here

    if (!curr->payload.i || guess == 0)
        curr->payload.i=guess;
    else
        return 0;

    return 1; //pointers updated successfully
}

int checkBoard(Box grid[9][9]) {
    int i, rt, ct, st, numoccurances=0; Link * curr;
    Box diagnal;
    for (i=0; i < 9; i++) {
        diagnal=grid[i][i];
        numoccurances=0;
        for (rt = 0, curr = diagnal.my_row.head; curr; rt += curr->payload.i, curr = curr->next){
            if (curr->type!=INT)
                return 0;
            if (curr->payload.i==i+1)
                numoccurances++;
            if (numoccurances>1)
                return 0;
        };
        numoccurances=0;
        for (ct = 0, curr = diagnal.my_column.head; curr; ct += curr->payload.i, curr = curr->next){
            if (curr->type!=INT)
                return 0;
            if (curr->payload.i==i+1)
                numoccurances++;
            if (numoccurances>1)
                return 0;
        };
        numoccurances=0;
        for (st = 0, curr = diagnal.my_sector.head; curr; st += curr->payload.i, curr = curr->next){
            if (curr->type!=INT)
                return 0;
            if (curr->payload.i==i+1)
                numoccurances++;
            if (numoccurances>1)
                return 0;
        };
    }
    return 1;
}

int revise(Box grid[9][9], char *board[9][9]){
    int i, j, weight=0, largestweight=0, sw,rw, cw;
    Box* boa = NULL;
    for (i=0;i<9;i++){
        for (j=0;j<9;j++){
            if (board[i][j]!="0") continue; //reject any box that's already filled in

            //NOTE: weight refers to the number of filled boxes in a row, col, or sector
            sw = getWeight(grid[i][j].my_sector);
            rw = getWeight(grid[i][j].my_row);
            cw = getWeight(grid[i][j].my_column);

            weight = sw+rw+cw;
            if (weight > largestweight) {
                largestweight=weight;
                boa = &grid[i][j]; //save the address of the box with the least number of blanks in its row, column, and sector
            }
        }
    }

    if (largestweight==0) {
        int itaddsup = checkBoard(grid);
        if (itaddsup) {
            fprintf(stderr, "***Puzzle Solved!.***\n");
            return 1; //puzzlesolved
        }
        else
            return -1; //failure occured
    }

    int size=0;
    char * possible[9];

    for (i = 1; i <= 9; i++){
        if (findIntInList(i,boa->my_column) || findIntInList(i,boa->my_row) || findIntInList(i,boa->my_sector))
            continue;

        possible[size]=malloc(2*sizeof(char));
        snprintf(possible[size++], 2, "%d", i);
    }

    int guess=0,row,col;
    for (i=0;i<size;i++){
        row=boa->rownum; col=boa->columnnum;
        guess=atoi(board[row][col]=possible[i]);
        if (!updatePointers(&boa, guess))
            return -1;//failure

        //print(board);

        int exitcondition = revise(grid,board); //recursive call to continue guess tree

        if (exitcondition) //will be 1 for board solved, -1 if error occured
            return exitcondition;
        else{
            fprintf(stderr, "Mistakes were Made... Undoing...\n\t>>%i in row %i col %i is blanked out<<\n\n", guess, row, col);
            board[row][col] = "0";
            updatePointers(&boa, 0);
        }
    }
    return 0;
}

int main(int argc, char ** argv) {
    FILE * fl;
    int download_successful = 0;
    char * filename = "";

    if (argc!=2){
        fprintf(stderr,"Usage: %s [filename | url]\n", argv[0]);
        return 1;
    }
    else if (!(fl=fopen(argv[1] ,"r"))){
        filename = "puzzlefile.tmp";
        download_successful = downloadFromURL(argv[1], filename);
        struct stat sb;
        if (download_successful)
            if (!stat(filename, &sb))
                fl = fopen(filename, "r");
            else{
                fprintf(stderr, "Could not open file: %s... does it exist?\n", filename);
                return 1;
            }
        else {
            fprintf(stderr, "Could not open file at url: %s... is it spelled right?\n", argv[1]);
            return 1;
        }
    }

    char * board[9][9], *carray[9][9], *boxes[9][9];
    int  i,j;
    make9x9(fl, board);
    fclose(fl);

    if (download_successful){
        char * cmd = malloc(sizeof("rm ")+sizeof(filename));
        cmd[0]='\0';
        strcat(cmd, "rm ");
        strcat(cmd, filename);
        system(cmd);
        fprintf(stderr, "\t> Removed temporary file %s. \n\n", filename);
    }
    fprintf(stdout, "problem:\n");
    pprintf(stdout, board);

    LinkedList row[9];
    LinkedList column[9];
    LinkedList sector[9];

    subdivide(board, boxes);
    transform(board, carray);

    for (i=0;i<9;i++){
        row[i]=createListFromStringArray(9,board[i]);
        column[i]=createListFromStringArray(9, carray[i]);
        sector[i]=createListFromStringArray(9, boxes[i]);
    }

    Box grid[9][9];
    Box* newBox;
    for (i=0;i<9;i++)
        for (j=0; j<9; j++){
            newBox=malloc(sizeof(Box));
            newBox->rownum=i; newBox->columnnum=j, newBox->sectornum=findSector(i,j);
            newBox->my_row=row[i]; newBox->my_column=column[j];
            newBox->my_sector=sector[newBox->sectornum];
            grid[i][j]=*newBox;
        }

    int success=1;
    if (checkBoard(grid)==0 || revise(grid, board)!=success) {
        fprintf(stderr,"is not solvable.\n");
        return 1;
    }

    fprintf(stdout, "solution:\n");
    pprintf(stdout,board);

    return 0;
}