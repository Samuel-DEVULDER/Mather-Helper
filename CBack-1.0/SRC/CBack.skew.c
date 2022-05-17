#include "CBack.h"
char *StackBottom = (char*) 0xeffff347;
int Merit;
void (*Fiasco)(void);
#define StackSize labs(StackBottom - StackTop)
#define Syncronize /* {jmp_buf E; if (!setjmp(E)) longjmp(E,1);} */

typedef struct State {  
    struct State *Left, *Right, *Next;	 	
    unsigned int LastChoice, Alternatives;
    int Merit;
    char *StackBottom, *StackTop;
    size_t Size;
    jmp_buf Environment;   
} State;

typedef struct Notification {
    void *Base;
    size_t Size;
    struct Notification *Next;
} Notification;

static State *TopState = 0, *FirstFree = 0, *NewState, *S;
static unsigned int LastChoice = 0, Alternatives = 0;
static char *StackTop;
static Notification *FirstNotification = 0;
static size_t NotifiedSpace = 0;

static State *Merge(State *H1, State *H2) {
    State *LeftChild;
    if (!H1) 
        return H2;
    if (!H2) 
        return H1;
    if (H1->Merit < H2->Merit)
        return Merge(H2, H1);
    LeftChild = H1->Left;
    if (!LeftChild)
        H1->Left = H2;
    else {
        H1->Left = Merge(H1->Right, H2);
        H1->Right = LeftChild;
    }
    return H1;
}

static void Insert(State *A)
{
     A->Left = A->Right = A->Next = 0;
     TopState = Merge(TopState, A);
}

static void DeleteMax()
{
     State *OldTopState = TopState;
     if (!OldTopState) 
         return;
     TopState = Merge(OldTopState->Left, OldTopState->Right);
      OldTopState->Next = FirstFree;
      FirstFree = OldTopState;
}

static void Error(char *Msg)
{
    fprintf(stderr,"Error: %s\n",Msg); 
    exit(0);
}

static void PushState(void) 
{
    static char *B;
    static size_t Size;
    Notification *N;
    
    StackTop = (char*) &N;
    Size = sizeof(State) + NotifiedSpace + StackSize;
    NewState = 0;
    if (FirstFree) {
        for (S = 0, NewState = FirstFree; NewState; S = NewState, NewState = S->Next) 
            if (NewState->Size <= Size)
                break;
        if (NewState) {
            if (!S) 
                FirstFree = NewState->Next; 
            else 
                S->Next = NewState->Next;
        }
    }
    if (!NewState) {
        NewState = (State*) malloc(Size);
        NewState->Size = Size;
        if (!NewState) 
            Error("No more space available for Choice");
    }
    NewState->LastChoice = LastChoice;
    NewState->Alternatives = Alternatives;
    NewState->Merit = Merit;
    NewState->StackBottom = StackBottom;
    NewState->StackTop = StackTop;
    B = (char*) NewState + sizeof(State);
    for (N = FirstNotification; N; B += N->Size, N = N->Next) 
        memcpy(B, (char *) N->Base, N->Size);
    Syncronize;
    memcpy(B, StackBottom < StackTop ? StackBottom : StackTop, StackSize);
}


#define PopState() DeleteMax()

unsigned int Choice(const int N) 
{
    if (N <= 0) 
        Backtrack();
    LastChoice = 1;
    Alternatives = N;
    if (N == 1 && (!TopState || TopState->Merit <= Merit))  
        return 1;
    PushState(); 
    if (!setjmp(NewState->Environment)) {
        Insert(NewState);
        if (TopState != NewState) {
        	NewState->LastChoice = 0;
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
        memcpy((char *) N->Base, B, N->Size);  
    Syncronize;
    memcpy(StackBottom < StackTop ? StackBottom : StackTop, B, StackSize);
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
        Error("Notification (unfinished Choice calls)");
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
        Error("RemoveNotification (unfinished Choice calls)");
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
