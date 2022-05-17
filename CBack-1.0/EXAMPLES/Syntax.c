#include "CBack.h"

char Input[81], *Sym;

#define test(C) if (*Sym++ != C) Backtrack()

void T();

void S()
{
    switch (Choice(2)) {
    case 1: test('b'); S(); break;
    case 2: T(); test('c'); break;
    }
}

void T()
{
    switch (Choice(3)) {
    case 1: test('a'); T(); test('a'); break;
    case 2: test('a'); T(); test('b'); break;
    case 3: test('a'); break;
    }
}

void SyntaxError() { printf("Syntax error\n"); }    

int main()
{
    Fiasco = SyntaxError;
    Notify(Sym);              /* Sym must be backtracked      */
    fgets(Input,81,stdin);    /* read input                   */
    Sym = Input;                                       
    Backtracking(S());        /* analyse input                */
    test('.');                /* a period must end the string */
    printf("No errors\n");
}
