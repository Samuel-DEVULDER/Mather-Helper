#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 4

int Piece[N*N], Solved;
int Count, Moves, h;

int DepthFirstSearch(int Bound, int Blank, int LastMove);

int DepthFirstSearch(int Bound, int Blank, int LastMove) {
    int NewBound, delta_h, B;
    int P, NextBlank, Move;
	
    Count++; Moves++;
    NewBound = 100000;
    for (Move = 1; Move <= 4; Move++) {
        switch(Move) {
        case 1: if (LastMove == 2 || Blank%N == 0) continue;
                    P = Piece[NextBlank = Blank - 1];
                    delta_h = P%N >= Blank%N ? -1 : 1;  
                    break;
        case 2: if (LastMove == 1 || Blank%N == N-1) continue;
                    P = Piece[NextBlank = Blank + 1];
                    delta_h = P%N <= Blank%N ? -1 : 1;  
                    break;
        case 3: if (LastMove == 4 || Blank < N) continue;
                    P = Piece[NextBlank = Blank - N];
                    delta_h = P/N >= Blank/N ? -1 : 1;
                    break;
        case 4: if (LastMove == 3 || Blank + N >= N*N) continue;
                    P = Piece[NextBlank = Blank + N];
                    delta_h = P/N <= Blank/N ? -1 : 1; 
                    break;
        }
        h += delta_h;
        if (h == 0) {
            Piece[Blank] = P;
            Piece[NextBlank] = 0;
            Solved = 1;
        	 return 1;
        }
        B = 1 + h;
        if (B <= Bound) {
            Piece[Blank] = P;
            Piece[NextBlank] = 0;
            B = 1 + DepthFirstSearch(Bound - 1, NextBlank, Move);
        	  if (Solved) return B;
        	  Piece[NextBlank] = P;
        	  Piece[Blank] = 0;
        }
        if (B < NewBound) NewBound = B;
        h -= delta_h;
    }
    Moves--;
    return NewBound;
}

#define Problems 100

int main() {
    int i, j, Blank, P;
    int Bound, Start, TimeSum = 0, LevelSum = 0; 
    FILE *input;
 	
    if (!(input  = fopen("KorfProblems", "r"))) {
        printf("File KorfProblems not found\n");
        exit(0);
    }
    for (i = 1; i <= Problems; i++) {	
        h = 0; 
        for (j = 0; j < N*N; j++) {
            fscanf(input,"%d",&P);
            Piece[j] = P; 
            if (!P) { Blank = j; }
            else h += abs(P/N - j/N) + abs(P%N - j%N);
        }
        printf("%3d: ",i); fflush(stdout);
        Start = clock();
        Count = Moves = 0;
        Bound = h;
        Solved = 0;
        while (!Solved) 
            Bound = DepthFirstSearch(Bound, Blank, 0);
        printf("Time: %7.2f sec., Level = %2d, Count = %d\n",
               1.0*(clock() - Start)/CLOCKS_PER_SEC, Moves, Count);
        LevelSum += Moves;
        TimeSum += clock() - Start;
    }
    printf("\nAvg. Level = %5.2f, Avg. Time = %5.2f sec.\n",
        1.0*LevelSum/Problems,1.0*TimeSum/CLOCKS_PER_SEC/Problems);
    return 1; 			
}
