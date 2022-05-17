#include "CBack.h"
#define P CurrentPosition.Piece
#define G GoalPosition.Piece

typedef struct Position {                     
  	int Piece[5][5];                /* Piece[r][c]: number of piece on (r,c) */
  	struct Position *Dad;           /* points to previous position           */
} Position;

static Position GoalPosition;

void PrintSolution(Position *p) 
{
    int i, j;
   
    if (p->Dad) {
    	PrintSolution(p->Dad);
    	printf("\n");
    }
    for (i = 1; i <= 4; i++) {
        for (j = 1; j <= 4; j++)
            printf("%d ",p->Piece[i][j]);
        printf("\n");
    }
}
                               
void Puzzle15()
{
    Position CurrentPosition = {0}, *s;
    int r, c, X, Y, d, Moves = 0;                 /* (X,Y): the empty square  */

    for (r = 1; r <= 4; r++) {
        for (c = 1; c <= 4; c++) {
            scanf("%d",&P[r][c]);
            if (!P[r][c]) {X = r; Y = c;} 
                G[r][c] = (r != 4 || c != 4 ? (r-1)*4 + c : 0);             
        }
    }      
        
    while (memcmp(P, G, sizeof(P))) {             /* while solution not found */
        Merit = -Moves;                           /* compute Merit            */
        for (r = 1; r <= 4; r++) {
            for (c = 1; c <= 4; c++) {
                 if (P[r][c] && P[r][c] != G[r][c]) { /* if P[r][c] misplaced */
                     d = abs(1+(P[r][c]-1)/4 - r) +   /* d = Manhattan        */
                  	     abs(1+(P[r][c]-1)%4 - c);    /*     distance to goal */
                     Merit -= d*d;
                     if (G[r][c]) {
                         if (r < 4 && P[r][c] == G[r+1][c]  
                             && P[r+1][c] == G[r][c])
                             Merit -= 100;                  /* exchanged pair */
                         if (c < 4 && P[r][c] == G[r][c+1] 
                             && P[r][c+1] == G[r][c])
                             Merit -= 100;                  /* exchanged pair */
                     }
                 }
             }
         }
         switch(Choice(4)) {                      /* choose a move            */
         case 1: r = X;     c = Y + 1; break;
         case 2: r = X + 1; c = Y;     break;
         case 3: r = X;     c = Y - 1; break;
         case 4: r = X - 1; c = Y;     break;
         }
         if (r < 1 || r > 4 || c < 1 || c > 4)    /* outside board ?          */
             Backtrack();                      
         P[X][Y] = P[r][c];                       /* make move                */
         P[X = r][Y = c] = 0;
         for (s = CurrentPosition.Dad; s; s = s->Dad) 
             if (!memcmp(P, s->Piece, sizeof(P))) /* duplicate?               */
                 Backtrack();
         s = (Position*) malloc(sizeof(Position));
         *s = CurrentPosition;                    /* copy CurrentPosition     */
         CurrentPosition.Dad = s;
         Moves++;
    }
    PrintSolution(&CurrentPosition); 
    printf("Number og moves = %d\n",Moves);      
}

int main() Backtracking(Puzzle15())
