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

void Check(int P[], int n, int i)
{
   int j, k, x, y, *Q;

   for (j = 1; j < i; j++)             /* check diagonals          */
       if (i + P[i] == j + P[j] || i - P[i] == j - P[j])
           Backtrack();
   Q = (int*) malloc((i+1)*sizeof(int));
   n++;
   for (k = 1; k <= 7; k++) {          /* for each transformation  */
       for (j = 1; j <= i; j++) {         
           switch(k) {                 /*    (j,P[j]) --> (x,y)    */
           case 1: x = j;        y = n - P[j]; break;
           case 2: x = n - j;    y = P[j];     break;
           case 3: x = n - j;    y = n - P[j]; break;
           case 4: x = P[j];     y = j;        break;
           case 5: x = P[j];     y = n - j;    break;
           case 6: x = n - P[j]; y = j;        break;
           case 7: x = n - P[j]; y = n - j;    break;
           }
           if (x > i) 
               break;
           Q[x] = y;
       }
       if (j > i) {                   /* if Q[1:i] found          */
           for (j = 1; j < i && Q[j] == P[j]; j++) 
               ;
           if (Q[j] < P[j]) {         /* if Q[1:i] < P[1:i]       */
               free(Q);
               Backtrack();
           }
       } 
   }
   free(Q);
}        

void NQ3()
{
    int P[N+1], i;

    for (i = 1; i <= N; i++) /* initialize p                         */
        P[i] = i;
    if (Choice(2) == 1) {    /* Choice == 2: all solutions are found */
        Permute(P, N, Check);
        Count++;
        Backtrack();
    } 
    printf("The %d-queen problem has %d non-symmetric solutions.\n", N, Count);
}

int main() Backtracking(NQ3())
