#include "CBack.h"
#define N 8
int Count;

void PrintCount()
{ printf("The %d-queens problem has %d solutions.\n", N, Count); }

void NQ2()
{
  int r, c, rf, cf, i, j, A[N+1], Q[N+1][N+1];

  Fiasco = PrintCount;
  for (r = 1; r <= N; r++) {
      A[r] = N;
      for (c = 1; c <= N; c++)
          Q[r][c] = c;
  }
  A[0] = N+1;     
  for (i = 1; i <= N; i++) {
      for (r = 0, rf = 1; rf <= N; rf++)  /* find best row, r     */
          if (A[rf] && A[rf] < A[r]) 
              r = rf;               
      c = Q[r][Choice(A[r])];             /* choose c in r        */
      A[r] = 0;
      for (rf = 1; rf <= N; rf++) {                  
          for (j = 1; j <= A[rf]; ) {     /* check (r,c) against  */
              cf = Q[rf][j];              /* future (rf,cf)       */   
              if (cf == c || r + c == rf + cf || r - c == rf - cf) {
                  if (A[rf] == 1)              
                      Backtrack(); 
                  Q[rf][j] = Q[rf][A[rf]--]; /* exclude (rf,cf)   */   
              }
              else
                  j++;
          }
      }                                                              
  }
  Count++;
  Backtrack();                            /* find next solution   */
}

int main() Backtracking(NQ2())
