#include "CBack.h"
#include <time.h>
#include <string.h>

/*
#define N 3
char *Number[] = {"SEND","MORE","MONEY"};
*/
/*
#define N 3;
char *Number[] = {"DONALD","GERALD","ROBERT"};
*/
#define N 4
char *Number[] = {"SIX","SEVEN","SEVEN","TWENTY"};

static int StartTime;

void Finish()
{ printf("Time: %5.2f sec\n",1.0*(clock() - StartTime)/CLOCKS_PER_SEC); }

Cryptarithm()
{
    static char Digits[] = "0123456789";
    char D[N], *c, Carry = 0, V, FreeDigits = strlen(Digits), i, j;
    int Sum, k, MaxLength = strlen(Number[N-1]);

    StartTime = clock(); 
    Fiasco = Finish;
    for (i = 0; i < N; i++)
        NotifyStorage(Number[i],strlen(Number[i]));
    NotifyStorage(Digits,FreeDigits = strlen(Digits));
    for (i = MaxLength - 1; i >= 0; i--) {
        for (j = 0; j < N; j++) {
            k = strlen(Number[j]) - MaxLength + i;
            if (k < 0) 
                D[j] = 0;
            else {
                if (isalpha(D[j] = Number[j][k])) {
                    V = Digits[k = Choice(FreeDigits--) - 1];
                    Digits[k] = Digits[FreeDigits];
                    for (k = 0; k < N; k++) 
                        while (c = strchr(Number[k],D[j]))
                            *c = V;
                    D[j] = V;
                }
                D[j] -= '0';
            }
        }  
        Sum = 0;
        for (k = 0; k < N-1; k++)
            Sum += D[k];
        if ((Sum + Carry)%10 != D[N-1]) 
            Backtrack();
        Carry = (Sum + Carry)/10;
    }
    if (Carry || !D[N-1])
        Backtrack();
    for (j = 0; j < N; j++) {
        for (k = MaxLength - strlen(Number[j]); k > 0; k--)
            printf(" "); 
        printf("%s\n",Number[j]);
    }
    printf("\n");
    Finish();
}

int main() Backtracking(Cryptarithm())
