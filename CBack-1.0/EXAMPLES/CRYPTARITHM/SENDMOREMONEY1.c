#include "CBack.h"
#include <stdio.h>
#define Set(V) { V = Digits[i = Choice(k)-1]; Digits[i] = Digits[--k]; }    

void SEND_MORE_MONEY() {
    int Digits[10] = {0,1,2,3,4,5,6,7,8,9};
    int S, E, N, D, M, O, R, Y;
    int i, k = 10;

    Set(S); if (S == 0) Backtrack(); Set(E); Set(N); Set(D); 
    Set(M); if (M == 0) Backtrack(); Set(O); Set(R); Set(Y);

    if (            S*1000L + E*100 + N*10 + D 
                  + M*1000L + O*100 + R*10 + E !=
         M*10000L + O*1000 + N*100 + E*10 + Y)
        Backtrack();
	
    printf("%d%d%d%d + %d%d%d%d = %d%d%d%d%d\n",
            S,E,N,D,M,O,R,E,M,O,N,E,Y);
    printf("S = %d, E = %d, N = %d, D = %d\n",S,E,N,D);
    printf("M = %d, O = %d, R = %d, Y = %d\n",M,O,R,Y);
}

int main() Backtracking(SEND_MORE_MONEY())
