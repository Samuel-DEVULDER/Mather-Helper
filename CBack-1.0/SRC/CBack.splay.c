#include "CBack.h"
char *StackBottom = (char*) 0xeffff347;
int Merit;
void (*Fiasco)(void);
#define StackSize labs(StackBottom - StackTop)
#define Syncronize /* {jmp_buf E; if (!setjmp(E)) longjmp(E,1);} */ 

typedef struct State {  
    struct State *Previous, *Next, *Son;	 	
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

#define Root TopState
#define Left Previous
#define Right Next
#define Parent Son
#define Node State

static void RotateLeft(Node *y)
{
    Node *x, *z;	
    if (!y) 
        return;
    x = y->Right; z = y->Parent;
    if (z) {
        if (z->Left == y)   
            z->Left = x; 
        else
            z->Right = x;
    }
    y->Right = x->Left; 
    x->Left = y;
    x->Parent = z; 
    y->Parent = x;
    if (y->Right) 
        y->Right->Parent = y;
}

static void RotateRight(Node *y)
{
    Node *x, *z;
    if (!y) 
        return;
    x = y->Left;
    z = y->Parent;
    if (z) {
        if (z->Right == y) 
            z->Right = x; 
        else z->Left = x;
    }
    y->Left = x->Right; 
    x->Right = y;
    x->Parent = z; 
    y->Parent = x;
    if (y->Left) y->Left->Parent = y;
}

static void Splay(Node *x)
{
    Node *p, *g;
    while ((p = x->Parent)) {
        g = p->Parent;
        if (x == p->Left) {
            if (g && p == g->Left) 
                RotateRight(g);
            RotateRight(p);
        }
        else {
            if (g && p == g->Right) 
                RotateLeft(g);
            RotateLeft(p);
        }
    }
    Root = x;
}

static Node 
 *FindMax(Node *t) 
{
    Node *x;
    if (!t) 
        return 0;
     while ((x = t->Left))
        t = x;
    Splay(t);
    return t;
}

static Node *Merge(Node *x, Node *y)
{
    if (!x) 
        return y;
    if (!y) 
        return x;
    y = FindMax(y);
    y->Left = x; 
    x->Parent = y;
    return y;
}

static void Delete(Node *x)
{
    Node *y, *p;
    if (!x)
        return;
    if ((y = x->Left))
        y->Parent = 0;
    if ((y = x->Right))
        y->Parent = 0;
    y = Merge(x->Left,x->Right);
    if ((p = x->Parent)) {
        if (p->Left == x) 
            p->Left = y; 
        else 
            p->Right = y;
        if (y) 
            y->Parent = p;
        Splay(p);
    }
    else 
        Root = y;
    x->Left = x->Right = x->Parent = 0;
}

static Node *DeleteMax()
{
    Node *x = FindMax(Root);
    Delete(x);
    return x;
}

static void Insert(Node *x)
{
    Node *p, *y;
    x->Left = x->Right = x->Parent = 0;
    if (!x)
        return;
    if (!Root) 
        Root = x;
    else {
        p = Root;
        do {
            if (x->Merit >= p->Merit) {
                y = p->Left;
                if (!y) 
                    p->Left = x; 
                else 
                    p = y;
            }
            else {
               y = p->Right;
                if (!y)  
                    p->Right = x; 
                else 
                    p = y;
            }
        } while (y);
        x->Parent = p;
        Splay(x);
    }
}

static void SiftUp(Node *x)
{
    Delete(x);
    Insert(x);
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
        if (!NewState) Error("No more space available for Choice");
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

static void PopState(void) 
{
    State *OldTopState = TopState;
    Delete(TopState);
    TopState =  FindMax(TopState);
    OldTopState->Next = FirstFree;
    FirstFree = OldTopState;
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
    if (!setjmp(NewState->Environment)) {
        Insert(NewState);
        TopState = FindMax(TopState);
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
        memcpy(N->Base, B, N->Size);  
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
