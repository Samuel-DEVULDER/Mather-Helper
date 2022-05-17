#include "CBack.h"
#include <time.h>
static int StartTime;
         
void SendMoreMoney() {
    int S, E, N, D, M, O, R, Y;

    S = Choice(9);
    E = Choice(10)-1; 
    while (E == S) 
        E = NextChoice() - 1;
    N = Choice(10)-1; 
    while (N == S || N == E) 
        N = NextChoice() - 1;	
    D = Choice(10)-1; 
    while (D == S || D == E || D == N) 
        D = NextChoice() - 1;	
    M = Choice(9); 
    while (M == S || M == E || M == N || M == D) 
        M = NextChoice() - 1;		
    O = Choice(10)-1; 
    while (O == S || O == E || O == N || O == D || O == M) 
       O = NextChoice() - 1;		
    R = Choice(10)-1; 
    while (R == S || R == E || R == N || R == D || R == M || R == O) 
        R = NextChoice() -1;		
    Y = Choice(10)-1; 
    while (Y == S || Y == E || Y == N || Y == D || Y == M || Y == O || Y == R) 
        Y = NextChoice() - 1;

    if (          1000*S + 100*E + 10*N + D +
                  1000*M + 100*O + 10*R + E !=
        10000*M + 1000*O + 100*N + 10*E + Y)
        Backtrack();
	
    printf("%d%d%d%d + %d%d%d%d = %d%d%d%d%d\n",
           S,E,N,D,M,O,R,E,M,O,N,E,Y);
    printf("S = %d, E = %d, N = %d, D = %d\n",S,E,N,D);
     printf("M = %d, O = %d, R = %d, Y = %d\n",M,O,R,Y);
}

int main() Backtracking(SendMoreMoney())
