#include "CBack.h"
#include <time.h>
#define P CurrentPosition.Piece
#define G GoalPosition.Piece

typedef struct Position {                     
    char Piece[4][4];         			
    struct Position *Dad;  				
} Position;

Position GoalPosition;

static FILE *input;
static int LevelSum, TimeSum, StartTime;
                               
void Puzzle15()
{
    Position CurrentPosition = {0}; 
    int X, Y, Moves = 0;
  	
    static int r, c, p, d, g, u, w, Count;
    static Position *s;
    static int StartTime;
  	
    StartTime = clock(); 
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            fscanf(input,"%d",&u);
            P[r][c] = u;
            if (!P[r][c]) {X = r; Y = c;} 
                G[r][c] = r*4 + c;            
        }
    }        
    for (Count = 0; memcmp(P, G, sizeof(P)); Count++) {
        Merit = -Moves/2;         				    
        for (r = 0; r < 4; r++) {
            for (c = 0; c < 4; c++) {
                if ((p = P[r][c]) && p != (g = G[r][c])) {
                    u = p/4 - r; w = p%4 - c; 
                    Merit -= u*u + w*w;
                    if (g) {
                        if (r < 3 && p == G[r+1][c] &&
                            P[r+1][c] == g) 
                        Merit -= 100;
                        if (c < 3 && p == G[r][c+1] &&
                            P[r][c+1] == g) 
                            Merit -= 100;
                    } 
              }
         }
    }
    switch(Choice(4)) {           	 			
    case 1: r = X;     c = Y + 1; break;
    case 2: r = X + 1; c = Y;     break;
    case 3: r = X;     c = Y - 1; break;
    case 4: r = X - 1; c = Y;     break;
    }
    if (r < 0 || r > 3 || c < 0 || c > 3)    	
        Backtrack();                      
    P[X][Y] = P[r][c];                			
    P[X = r][Y = c] = 0;
    for (s = CurrentPosition.Dad; s; s = s->Dad) 
        if (!memcmp(P, s->Piece, sizeof(P)))   	
            Backtrack();
        s = (Position*) malloc(sizeof(Position));
        *s = CurrentPosition;           			
        CurrentPosition.Dad = s;
        Moves++;
    }
    LevelSum += Moves;
    TimeSum += clock() - StartTime;
    ClearChoices();
    printf("Time: %7.2f sec., Level = %3d, Count = %d\n",
            1.0*(clock() - StartTime)/CLOCKS_PER_SEC, 
            Moves, Count);      
}

int main() 
{
    int i;
	 
    if (!(input  = fopen("KorfProblems", "r"))) {
        printf("File KorfProblems not found\n");
        exit(0);
    }
    for (i = 1; i <= 100; i++) {
        printf("%3d: ",i); fflush(stdout); 
        Backtracking(Puzzle15()); 
     }
     printf("Avg. Level = %5.2f, Avg. Time = %7.2f sec.\n",
            0.01*LevelSum,0.01*TimeSum/CLOCKS_PER_SEC);
     return 0; 			
}
