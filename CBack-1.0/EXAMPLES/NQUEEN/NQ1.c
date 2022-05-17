#include "CBack.h"
int N, Count, r, c, *R, *S, *D;

void PrintCount()
{ printf("The %d-queens problem has %d solutions.\n",N,Count); }
  
void NQ1() {
    printf("Number of queens: "); scanf("%d",&N);
    Fiasco = PrintCount;
    Notify(r);
    R = (int*) Ncalloc(N+1,   sizeof(int));
    S = (int*) Ncalloc(2*N-1, sizeof(int));
    D = (int*) Ncalloc(2*N-1, sizeof(int));
    for (r = 1; r <= N; r++) {
        c = Choice(N);
        if (R[c] || S[r+c-2] || D[r-c+N-1])  
            Backtrack();
        R[c] = S[r+c-2] = D[r-c+N-1] = r;
    }
	Count++;
    Backtrack();
}

int main() Backtracking(NQ1())
