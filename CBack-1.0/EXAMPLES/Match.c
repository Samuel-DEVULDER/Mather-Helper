#include <string.h>
#include "CBack.h"

int Match(char* s, char *p)
{
     if (Choice(2) == 2)
         return 0;          /* no match, all alternatives exhausted */
     while (*p) {           /* while more characters in pattern p   */
         if (*p == '*') {   /* current character in pattern is * ?  */
             p++;           /* move to next character in pattern    */
             s += Choice(strlen(s)) - 1;     /* try to skip in s    */
         }
         else if (*p++ != *s++) /* if match, advance p and s        */
             Backtrack();       /* otherwise backtrack              */
    }
    if (*s)                    /* if no more characters in s        */
       Backtrack();            /* then backtrack                    */
    ClearChoices();            /* clear all pending Choice-calls    */
    return 1;                  /* and return 1 (a match was found)  */
}

void main_Match()
{
    char pattern[81], string[81];
    printf("Pattern = "); fflush(stdout);
    fgets(pattern, 80, stdin);
    printf("String = "); fflush(stdout);
    fgets(string, 80, stdin);
    if (Match(string,pattern)) printf("Succes\n"); 
         else printf("Failure\n");
} 

int main() Backtracking(main_Match())
