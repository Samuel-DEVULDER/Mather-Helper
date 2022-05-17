#include "CBack.h"
char *StackBottom = (char*) 0xeffff347;		
int Merit;
void (*Fiasco)(void);

#define StackSize labs(StackBottom - StackTop)
#define Synchronize /* {jmp_buf E; if (!setjmp(E)) longjmp(E,1);} */

typedef struct State {   
    struct State *Previous;	 	
    unsigned int LastChoice, Alternatives;
    int Merit;
    char *StackBottom, *StackTop;
    jmp_buf Environment;   
} State;

typedef struct Notification {
    void *Base;
    size_t Size;
    struct Notification *Next;
} Notification;

static State *TopState = 0, *Previous, *S;
static unsigned int LastChoice = 0, Alternatives = 0;
static char *StackTop;
static Notification *FirstNotification = 0;
static size_t NotifiedSpace = 0;

static void Error(char *Msg)
{
    fprintf(stderr,"Error: %s\n",Msg); 
    exit(0);
}

static void PopState(void) 
{
    Previous = TopState->Previous;
    free(TopState); 
    TopState = Previous;
}

static void PushState(void) 
{
    static char *B;
    Notification *N;

    StackTop = (char*) &N;
    Previous = TopState;
    TopState = (State*) malloc(sizeof(State) + NotifiedSpace + StackSize);
    if (!TopState) 
        Error("No more space available for Choice");
    TopState->Previous = Previous;
    TopState->LastChoice = LastChoice;
    TopState->Alternatives = Alternatives;
    TopState->Merit = Merit;
    TopState->StackBottom = StackBottom;
    TopState->StackTop = StackTop;
    B = (char*) TopState + sizeof(State);
    for (N = FirstNotification; N; B += N->Size, N = N->Next) 
        memcpy(B, N->Base, N->Size);
    Synchronize;
    memcpy(B,StackBottom < StackTop ? StackBottom : StackTop, StackSize);
}

unsigned int Choice(const int N) 
{
    if (N <= 0) 
        Backtrack();
    LastChoice = 1;
    Alternatives = N;
    if (N == 1 && (!TopState || TopState->Merit <= Merit))  
        return 1;
    PushState(); 
    if (!setjmp(TopState->Environment)) {
        if (Previous && Previous->Merit > Merit) {
           for (S = Previous; S->Previous; S = S->Previous)
                if (S->Previous->Merit <= Merit) 
                    break;
            TopState->Previous = S->Previous;
            S->Previous = TopState;
            TopState->LastChoice = 0;
            TopState = Previous;
            Backtrack();
        }
    } 
    if (LastChoice == Alternatives) 
        PopState();
    return LastChoice;
}

void Backtrack(void)
{
    char *B;
    Notification *N;
   
    if (!TopState) { 
        if (Fiasco) 
            Fiasco(); 
        exit(0);
    }   
    StackTop = (char*) &N;   
    if ((StackBottom < StackTop) == (StackTop < TopState->StackTop)) 
        Backtrack();
    LastChoice = ++TopState->LastChoice;
    Alternatives = TopState->Alternatives;
    Merit = TopState->Merit;
    StackBottom = TopState->StackBottom;
    StackTop = TopState->StackTop; 
    B = (char*) TopState + sizeof(State);
    for (N = FirstNotification; N; B += N->Size, N = N->Next)   
        memcpy(N->Base, B, N->Size);  
    Synchronize;
    memcpy(StackBottom < StackTop ? StackBottom : StackTop,B, StackSize);
    longjmp(TopState->Environment, 1);
} 

unsigned int NextChoice(void) 
{
    if (++LastChoice > Alternatives) 
        Backtrack();
    if (LastChoice == Alternatives) 
        PopState();
    else 
        TopState->LastChoice = LastChoice;
    return LastChoice;
}

void Cut(void)
{
    if (LastChoice < Alternatives) 
        PopState(); 
    Backtrack();
}

void *NotifyStorage(void *Base, size_t Size) 
{
    Notification *N;

    if (TopState) 
        Error("Notification (unfinished Choice-calls)");
    for (N = FirstNotification; N; N = N->Next)
    	    if (N->Base == Base)
	        return 0;
    N = (Notification*) malloc(sizeof(Notification));
    if (!N) 
        Error("No more space for notification");
    N->Base = Base; 
    N->Size = Size; 
    NotifiedSpace += Size; 
    N->Next = FirstNotification; 
    FirstNotification = N;
    return Base;
}

void RemoveNotification(void *Base)
{
    Notification *N, *Prev = 0;

    if (TopState) 
        Error("RemoveNotification (unfinished Choice-calls)");
    for (N = FirstNotification; N; Prev = N, N = N->Next) {
        if (N->Base == Base) {
            NotifiedSpace -= N->Size;
            if (!Prev) 
                FirstNotification = N->Next; 
            else 
                Prev->Next = N->Next;
            free(N);
            return;
        }
    }
}

void ClearChoices(void) 
{
    while (TopState) 
        PopState();
    LastChoice = Alternatives = 0;
}

void ClearNotifications(void) 
{
   while (FirstNotification)
       RemoveNotification(FirstNotification->Base);            
}
