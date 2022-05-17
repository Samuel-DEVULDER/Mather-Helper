#include "CBack.h"
 
void main_8Q()
{
    int r, c;
    int R[9]={0}, S[15]={0}, D[15]={0};   /* clear board             */

    for (r = 1; r <= 8; r++) {            /* for each row, r         */
        c = Choice(8);                    /*   choose a column, c    */
        if (R[c] || S[r+c-2] || D[r-c+7]) /*   if (r,c) under attack */
            Backtrack();                  /*   then backtrack        */
        R[c] = S[r+c-2] = D[r-c+7] = r;   /*   else place queen      */
    }
    for (c = 1; c <= 8; c++)              /* print solution          */
        printf("(%d,%d)  ",R[c],c);
    printf("\n");    
    Backtrack();
}

int main() Backtracking(main_8Q())
