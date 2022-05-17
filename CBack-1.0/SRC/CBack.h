#ifndef Backtracking
#define Backtracking(S) {char Dummy; StackBottom = &Dummy; S; } 
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#define Notify(V) NotifyStorage(&V, sizeof(V))
#define Nmalloc(Size) NotifyStorage(malloc(Size), Size)
#define Ncalloc(N, Size) NotifyStorage(calloc(N,Size), (N)*Size)
#define Nrealloc(P, Size)\
        (RemoveNotification(P),\
         NotifyStorage(realloc(P, Size), Size))
#define Nfree(P) (RemoveNotification(P), free(P))
#define ClearAll(void) (ClearChoices(), ClearNotifications())

unsigned int Choice(const int N);
void Backtrack(void);       
unsigned int NextChoice(void);
void Cut(void);
void ClearChoices(void);

void *NotifyStorage(void *Base, size_t Size);
void RemoveNotification(void *Base);
void ClearNotifications(void);

extern char *StackBottom;
extern void (*Fiasco)(void);
extern int Merit;  
#endif
