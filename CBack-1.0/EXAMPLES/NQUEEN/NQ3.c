#include "CBack.h"
#define N 8
int Count;

int P[N+1];      /* P[0] is not used */

void Permute(int P[], int n, void (*Check) (int*, int, int))
{
    int i, j, k;

    for (k = P[i = 1]; i <= n; k = P[++i]) {
        P[i] = P[j = i-1 + Choice(n-i+1)];
        P[j] = k;
        if (Check)
            Check(P, n, i);
    }
}

void CheckDiagonals(int P[], int n, int i)
{
      int j;

      for (j = 1; j < i; j++)
          if (i + P[i] == j + P[j] || i - P[i] == j - P[j])
              Backtrack();
}

void NQ3()
{
    int P[N+1], i;

    for (i = 1; i <= N; i++) /* initialize                           */
        P[i] = i;
    if (Choice(2) == 1) {    /* Choice == 2: all solutions are found */
        Permute(P, N, CheckDiagonals);
        Count++;
        Backtrack();
    } 
    printf("The %d-queen problem has %d solutions.\n", N, Count);
}

int main() Backtracking(NQ3())
