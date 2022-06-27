#if 0
gcc -DxSLOW_RAT -DxDEBUG -DxNUMBLE -DxEASY -DxNORMAL -DHARD -DxTHENUMBLE -Wall $0 -o a.out CBack-1.0/SRC/CBack.c -fshort-enums -O3 -fopenmp && time ./a.out
exit $?
#endif

/*
 * mathler.c - solves mathler-like games (https://www.mathler.com/,
 *             https://www.thenumble.app/) using the same technique as
 *             for the Mastermind(tm) (https://youtu.be/FR_71HyBytE).
 *
 * (c) 2022 by Samuel Devulder
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <locale.h>
#include <stdlib.h>

// #undef _OPENMP

#ifdef _OPENMP
#include <omp.h>
#endif

#include "CBack-1.0/SRC/CBack.h"

#ifdef EASY
#define SIZE                5
#define MAX_OP              1
#define URL                 "https://easy.mathler.com/"

#elif defined(NORMAL)
#define SIZE                6
#define MAX_OP              2
#define URL                 "https://mathler.com/"

#elif defined(HARD)
#define SIZE                8
#define MAX_OP              3
#define URL                 "https://hard.mathler.com/"

#elif defined(THENUMBLE)
#define SIZE                7
#define URL                 "https://www.thenumble.app/"

#elif defined(NUMBLE)
#define SIZE                8
#define URL                 "https://www.mathix.org/numble/"

#else
#error Please define one of EASY, NORMAL, HARD, NUMBLE, THENUMBLE.
#define SIZE                1
#define URL                 ""
#endif

#ifndef MAX_OP
#define MAX_OP              SIZE
#endif

#ifndef CONFIG
#define CONFIG              2
#endif

#define DO_SHUFFLE          0 //defined(_OPENMP)
#define DO_SORT             1
#define FASTER_RAND         1

#define MAX_CANDIDATES      (5000)
#define MAX_SAMPLES         (15000*4)
#define MIN_SAMPLE_RATIO    (0.05)

/*****************************************************************************/

#if (((SIZE)>=8) && !defined(NUMBLE))
#define ALLOW_PARENTHESIS   1
#else
#define ALLOW_PARENTHESIS   0
#endif
#define PRIVATE             static

typedef int integer;

PRIVATE const char *A_BOLD="", *A_NORM=""; /* ansi escape sequence */

#ifndef timersub
#define timersub(a, b, result)                            \
  do {                                                    \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;         \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;      \
    if ((result)->tv_usec < 0) {                          \
      --(result)->tv_sec;                                 \
      (result)->tv_usec += 1000000;                       \
    }                                                     \
  } while (0)
#endif

#ifdef WIN32
#define aligned_alloc(x,y) _aligned_malloc((y),(x))
#endif

/*****************************************************************************/

PRIVATE int popcount(uint32_t _x) {
#ifdef __GNUC__
    return __builtin_popcount(_x);
#else
    uint32_t x = _x;
    x -= (x >> 1) & 0x55555555;
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0f0f0f0f;
    x = (x * 0x01010101) >> 24;
    return x;
#endif
}

/*****************************************************************************/

PRIVATE void gettime(struct timeval *tv) {
    if(gettimeofday(tv, NULL)<0) {
        perror("gettimeofday");
        exit(EXIT_FAILURE);
    }
}

/*****************************************************************************/

#ifdef FASTER_RAND

#define XORSHIFT
// #define XOR128
// #define PARK_MILLER

#ifdef XORSHIFT
PRIVATE uint32_t _rnd_seed = 0xDEADBEEF;
PRIVATE void _srnd(int seed) {
    _rnd_seed = seed == 0 ? 0xABADCAFE : seed;
}
PRIVATE int _rnd(void) {
    uint32_t x = _rnd_seed;
    x ^= x<<13;
    x ^= x>>17;
    x ^= x<<5;
    _rnd_seed = x;
#if RAND_MAX==INT_MAX
    return x>>1;
#else
    return x&RAND_MAX;
#endif
}

#elif defined(PARK_MILLER)
PRIVATE uint32_t _rnd_seed = 0xDEADBEEF;
PRIVATE void _srnd(int seed) {
    _rnd_seed = seed == 0 || seed == 0x7fffffff ? 0xABADCAFE : seed;
}
PRIVATE int _rnd(void) {
    return _rnd_seed = (_rnd_seed * (uint64_t)48271) % 0x7fffffff;
}

#elif defined(XOR128)
PRIVATE void _srnd(int seed) {
    (void)seed;
}
PRIVATE int _rnd(void) {
  static uint32_t x = 123456789;
  static uint32_t y = 362436069;
  static uint32_t z = 521288629;
  static uint32_t w = 88675123;
  uint32_t t;
  t = x ^ (x << 11);
  x = y; y = z; z = w;
  return (w ^= (w >> 19) ^ (t ^ (t >> 8))) >> 1;
}
#endif

#define rand    _rnd
#define srand   _srnd
#endif

/*****************************************************************************/

PRIVATE int progress(long long count) {
    static int cpt, cpt_sec, last;
    static struct timeval start;
    static long long total;

    if(count==0) { // done
        struct timeval curr, temp;
        int i;
        for(i=0; i<last; ++i) putchar(' ');
        for(i=0; i<last; ++i) putchar('\b');
        fflush(stdout);
        gettime(&curr);
        timersub(&curr, &start, &temp);
        return temp.tv_sec>INT_MAX ? INT_MAX : (int)temp.tv_sec;
    } else if(count<0) { // set
        total   = -count;
        cpt_sec = -30;
        last    = 0;
        cpt     = 0;
        gettime(&start);
    } else if(count == LLONG_MAX) { // undef
        static char *mill = "-\\|/";
        if(((++cpt)&63)==0) {
            int i;
            last = printf(" (%c)", mill[(cpt/64)&3]);
            for(i=0; i<last; ++i) putchar('\b');
            fflush(stdout);
        }
    } else { // step
        struct timeval curr, temp; ++cpt;
        if(cpt_sec<0 && 0<=cpt_sec+cpt) {
            cpt_sec -= 30;
            gettime(&curr);
            timersub(&curr, &start, &temp);
            if(temp.tv_sec>=15) {
                cpt_sec = cpt/temp.tv_sec;
            }
        }
        if(cpt_sec>=0 && cpt>=cpt_sec) {
			double d = ((double)count)/total;
            int i, t; cpt = 0;
            gettime(&curr);
            timersub(&curr, &start, &temp);
            t = printf(" %.1f%% (%ds, rem %ds)", d*100,
                (int)temp.tv_sec,
                (int)(temp.tv_sec*(1/d-1)));
            for(i = t; i<last; ++i) putchar(' ');
            while(i--) putchar('\b');
            last = t;
            fflush(stdout);
        }
    }
    return -1;
}
// #define progress(X) (void)(X);

/*****************************************************************************/

#define ARRAY(TYPE)                                         \
struct {                                                    \
    size_t         capa;                                    \
    const size_t   cell;                                    \
    size_t         len;                                     \
    TYPE          *tab;                                     \
    volatile TYPE *ptr;                                     \
}

PRIVATE void _ARRAY_DISPOSE(void *_array) {
    ARRAY(void) *array = _array;
    if(array!=NULL) {
        if(array->tab!=NULL) free(array->tab);
        array->tab  = NULL;
        array->capa = 0;
        array->len  = 0;
    }
}

PRIVATE void *_ARRAY_PTR(void *_array, size_t n) {
    ARRAY(void) *array = _array;
    if(array!=NULL && array->capa<=n) {
        array->capa = n + 128;
        array->tab = realloc(array->tab, array->capa * array->cell);
        assert(array->tab != NULL);
    }
    return array->tab + n*array->cell;
}

#define ARRAY_DECL(TYPE, NAME) ARRAY(TYPE) NAME = {         \
    .capa = 0,                                              \
    .cell = sizeof(TYPE),                                   \
    .len  = 0,                                              \
    .tab  = NULL}

#define ARRAY_DONE(ARRAY)    _ARRAY_DISPOSE(&(ARRAY))

#define ARRAY_AT(ARRAY,INDEX)                               \
    *((ARRAY).ptr=_ARRAY_PTR(&(ARRAY),(INDEX)))

#if 0
#define ARRAY_REM(ARRAY, INDEX)                             \
    memmove(&(ARRAY).tab[(INDEX)], &(ARRAY).tab[(INDEX)+1], \
           (--(ARRAY).len - (INDEX))*(ARRAY).cell)
#else          
#define ARRAY_REM(ARRAY, INDEX)                             \
    (ARRAY).tab[(INDEX)] = (ARRAY).tab[--(ARRAY).len]
#endif

#if 1    
#define ARRAY_ADD(ARRAY,VAL)                                \
    ARRAY_AT((ARRAY),(ARRAY).len++)=(VAL)
#else
#define ARRAY_ADD(ARRAY, VAL) do {                          \
    _ARRAY_ENSURE_CAPA(&(ARRAY), ++(ARRAY).len);            \
    (ARRAY).tab[(ARRAY).len-1] = (VAL);                     \
} while(0)
#endif

#define ARRAY_CPY(DST, SRC) do {                            \
    _ARRAY_PTR(&(DST), (DST).len = (SRC).len);      \
    /* if only whe could do typeof(x)==typeof(y)... */      \
    if((DST).cell == (SRC).cell) {                          \
        memcpy((DST).tab, (SRC).tab, (SRC).len*(SRC).cell); \
    } else {                                                \
        int i, len = (SRC).len;                             \
        for(i = 0; i<len; ++i) (DST).tab[i] = (SRC).tab[i]; \
    }                                                       \
} while(0)

/*****************************************************************************/

typedef struct {
    integer p;
    integer q;
} rat;

#ifndef NUMBLE
/* converts a double to a rationnal */
PRIVATE void rat_double(rat *r, double f) {
    // https://rosettacode.org/wiki/Convert_decimal_number_to_rational#C

    /*  a: continued fraction coefficients. */
    int64_t a, h[3] = { 0, 1, 0 }, k[3] = { 1, 0, 0 };
    int64_t x, d, n = 1, md = 32767; // md = max denominator
    bool neg = f<0;
    int i;

    if(neg) f = -f;
    while (f != floor(f)) { n <<= 1; f *= 2; }
    d = f;

    /* continued fraction and check denominator each step */
    for (i = 0; i < 64; i++) {
        a = n ? d / n : 0;
        if (i && !a) break;

        x = d; d = n; n = x % n;

        x = a;
        if (k[1] * a + k[0] >= md) {
            x = (md - k[0]) / k[1];
            if (x * 2 >= a || k[1] >= md)
                i = 65;
            else
                break;
        }

        h[2] = x * h[1] + h[0]; h[0] = h[1]; h[1] = h[2];
        k[2] = x * k[1] + k[0]; k[0] = k[1]; k[1] = k[2];
    }
    r->q = k[1];
    r->p = neg ? -h[1] : h[1];
}

PRIVATE bool rat_whole(rat *p) {
    return p->q == 1;
}
#endif

PRIVATE bool rat_integer(rat *r, integer n) {
    r->p = n;
    r->q = 1;
    return true;
}

//* https://rosettacode.org/wiki/Greatest_common_divisor#C */
PRIVATE integer gcd(integer a, integer b) {
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    if(b) while ((a %= b) && (b %= a));
    return a+b;
}

PRIVATE void rat_norm(rat *r, integer p, integer q) {
    integer t = gcd(p,q);
    r->p = p/t;
    r->q = q/t;
}

PRIVATE void rat_add(rat *r, rat *u, rat *v) {
/* define SLOW_RAT if normalisation of rationnal is slow
  (not the case on modern cpus). */
#ifdef SLOW_RAT
    if(rat_whole(u) && rat_whole(v)) rat_integer(r, u->p + v->p); else
#endif
    rat_norm(r, u->p*v->q + v->p*u->q, u->q*v->q);
}

PRIVATE void rat_sub(rat *r, rat *u, rat *v) {
#ifdef SLOW_RAT
    if(rat_whole(u) && rat_whole(v)) rat_integer(r, u->p - v->p); else
#endif
    rat_norm(r, u->p*v->q - v->p*u->q, u->q*v->q);
}

PRIVATE void rat_mul(rat *r, rat *u, rat *v) {
#ifdef SLOW_RAT
    if(rat_whole(u) && rat_whole(v)) rat_integer(r, u->p * v->p); else
#endif
    rat_norm(r, u->p * v->p, u->q * v->q);
}

PRIVATE void rat_div(rat *r, rat *u, rat *v) {
#ifdef SLOW_RAT
    if(rat_whole(u) && rat_whole(v)) rat_norm(r, u->p, v->p); else
#endif
    rat_norm(r, u->p * v->q, u->q * v->p);
}

/*****************************************************************************/

typedef enum {
    MSK5=1,
    MSK4=2,
    MSK6=4,
    MSK3=8,
    MSK7=16,
    MSK2=32,
    MSK8=64,
    MSK1=128,
    MSK9=256,
    MSK0=512,
    MSKsub=1024,
    MSKadd=2048,
    MSKdiv=4096,
    MSKmul=8192,
#if ALLOW_PARENTHESIS
    MSKbra=16384,
    MSKket=32768,
#elif defined(NUMBLE)
    MSKequ=16384,
#endif
    MSKnone=0
} mask;

PRIVATE const mask MSKall = MSKnone
    | MSK0
    | MSK1
    | MSK2
    | MSK3
    | MSK4
    | MSK5
    | MSK6
    | MSK7
    | MSK8
    | MSK9
    | MSKadd
    | MSKsub
    | MSKmul
    | MSKdiv
#if ALLOW_PARENTHESIS
    | MSKbra
    | MSKket
#elif defined(NUMBLE)
    | MSKequ
#endif
    | MSKnone;

PRIVATE mask char_to_mask(char symbol) {
    switch(symbol) {
        case '0': return MSK0;
        case '1': return MSK1;
        case '2': return MSK2;
        case '3': return MSK3;
        case '4': return MSK4;
        case '5': return MSK5;
        case '6': return MSK6;
        case '7': return MSK7;
        case '8': return MSK8;
        case '9': return MSK9;
        case '+': return MSKadd;
        case '-': return MSKsub;
        case '*': return MSKmul;
        case '/': return MSKdiv;
#if ALLOW_PARENTHESIS
        case '(': return MSKbra;
        case ')': return MSKket;
#elif defined(NUMBLE)
        case '=': return MSKequ;
#endif
        default:  return MSKnone;
    }
}

PRIVATE char mask_to_char(mask mask) {
    switch(mask) {
        case MSK0:   return '0';
        case MSK1:   return '1';
        case MSK2:   return '2';
        case MSK3:   return '3';
        case MSK4:   return '4';
        case MSK5:   return '5';
        case MSK6:   return '6';
        case MSK7:   return '7';
        case MSK8:   return '8';
        case MSK9:   return '9';
        case MSKadd: return '+';
        case MSKsub: return '-';
        case MSKmul: return '*';
        case MSKdiv: return '/';
#if ALLOW_PARENTHESIS
        case MSKbra: return '(';
        case MSKket: return ')';
#elif defined(NUMBLE)
        case MSKequ: return '=';
#endif
        default:     return '\0';
    }
}

#ifdef DEBUG
PRIVATE void mask_print(mask msk) {
    while(msk) {
        mask m = msk & -msk;
        putchar(mask_to_char(m));
        msk -= m;
    }
}
#endif

/*****************************************************************************/

// converted from prolog (https://pastebin.com/YV7xRsdg) to C

typedef struct {
    bool set;
    rat val;
} opt_rat;

PRIVATE opt_rat *expression(opt_rat *T, int from, int to);
PRIVATE opt_rat *term(opt_rat *T, int from, int to);
PRIVATE opt_rat *factor(opt_rat *T, int from, int to);
PRIVATE opt_rat *number(opt_rat *T, int from, int to);

PRIVATE char buffer[SIZE];

typedef opt_rat *(*goal)(opt_rat *T, int from, int to);

/*
 * solves T = U op V
 *
 * if T is set, compute U to satisy equation;
 * else (U is set), compute W.
 * Note: V is always set.
 */
PRIVATE opt_rat *solve(opt_rat *T, goal goal, opt_rat *U, int from, int to, char op, opt_rat *V) {
    assert(V->set);

    buffer[to] = op;
    switch(op) {
        case '+':
        if(T->set) {
            rat_sub(&U->val, &T->val, &V->val);
            U->set = true;  goal(U, from, to);
        } else {
            U->set = false; goal(U, from, to);
            rat_add(&T->val, &U->val, &V->val);
            T->set = true;
        }
        break;

        case '=':
        case '-':
        if(T->set) {
            rat_add(&U->val, &T->val, &V->val);
            U->set = true;  goal(U, from, to);
        } else {
            U->set = false; goal(U, from, to);
            rat_sub(&T->val, &U->val, &V->val);
            T->set = true;
        }
        break;

        case '/':
        if(V->val.p == 0) Backtrack();
        if(T->set) {
            rat_mul(&U->val, &T->val, &V->val);
            U->set = true;  goal(U, from, to);
        } else {
            U->set = false; goal(U, from, to);
            rat_div(&T->val, &U->val, &V->val);
            T->set = true;
        }
        break;

        case '*':
        if(!T->set || 0 == (T->val.p | V->val.p)) {
            U->set = false; goal(U, from, to);
            rat_mul(&T->val, &U->val, &V->val);
            T->set = true;
        } else {
            if(V->val.p == 0) Backtrack();
            rat_div(&U->val, &T->val, &V->val);
            U->set = true;  goal(U, from, to);
        }
        break;

        default:
        assert(false);
    }
    return T;
}

PRIVATE opt_rat *expression(opt_rat *T, int from, int to) {
    char op;

    switch(Choice(3)) {
        case 1: // expression ::= term
        return term(T, from, to);

        case 2: // expression ::= expression + term
        op = '+'; break;

        case 3: // expression ::= expression - term
        op = '-'; break;

        default: assert(false); return T;
    }
    {
        int split = from + Choice(to - from - 2);
        opt_rat U, V;  V.set = false;
        return solve(T, expression, &U, from, split, op, term(&V, split+1, to));
    }
}

PRIVATE opt_rat *term(opt_rat *T, int from, int to) {
    char op;
    switch(Choice(3)) {
        case 1: // term ::= factor
        return factor(T, from, to);

        case 2: // term ::= term * factor
        op = '*'; break;

        case 3: // term ::= term / factor
        op = '/' ; break;

        default: assert(false); return T;
    }
    {
        int split = from + Choice(to - from - 2);
        opt_rat U, V;  V.set = false;
        return solve(T, term, &U, from, split, op, factor(&V, split+1, to));
    }
}

PRIVATE opt_rat *factor(opt_rat *T, int from, int to) {
    switch(Choice(1
#if ALLOW_PARENTHESIS
                    +1
#endif
    )) {
        case 1: // factor ::= number
        return number(T, from, to);

        case 2: // factor ::= ( expression )
        if(to-from<3) Backtrack();
        buffer[from] = '(';
        buffer[to-1] = ')';
        return expression(T, from+1, to-1);
    }
    assert(false);
    return T;
}

PRIVATE bool num(int n, int from, int to) {
    int j = to;
    do {
        buffer[--j] = '0' + (n%10);
        n /= 10;
    } while(n && from<j);
    return n==0 && from==j;
}

PRIVATE integer ipow(integer a, int b) {
    integer x = 1, y = a, z = b;
    while(z>0) {
        if(z & 1) x *= y;
        y *= y;
        z >>= 1;
    }
    return x;
}

PRIVATE opt_rat *number(opt_rat *T, int from, int to) {
    if(T->set) {
        if(T->val.q!=1
        || T->val.p<0
        || !num(T->val.p, from, to)) Backtrack();
    } else {
        integer i = ipow(10, to - from - 1);
        rat_integer(&T->val, Choice(i==1 ? 10 : (9*i)) + i - (i==1 ? 2 : 1));
        T->set = true;
        (void)num(T->val.p, from, to);
    }
    return T;
}

/*****************************************************************************/

#ifdef SIMD
#ifdef __SSE4_1__
#include <immintrin.h>
#define SIMD_TYPE __m128i
#else
/* poor man SIMD: use 32/64 bits to handle 2 or for 4 masks in one go */
#define SIMD_TYPE uint_fast32_t 
#endif
#endif

typedef union {
#ifdef  SIMD_TYPE
    SIMD_TYPE   vect[(SIZE*sizeof(mask)+sizeof(SIMD_TYPE)-1)/sizeof(SIMD_TYPE)];
#endif
    mask        masks[SIZE];
} masks;


typedef struct formula {
    masks           _symbols;
#define symbols _symbols.masks
    mask            unused;
    unsigned char   used_count;
} formula;

PRIVATE ARRAY_DECL(formula *, formulae);
PRIVATE ARRAY_DECL(formula *, found);


#ifdef _OPENMP
PRIVATE int nthreads = 1;
#endif

PRIVATE void findall(rat *num) {
    opt_rat T;  T.set = true;
    T.val = *num;
#ifdef NUMBLE
    {
        int split = Choice(SIZE - 2);
        opt_rat U, V;  V.set = false;
        solve(&T, expression, &U, 0, split, '=', term(&V, split+1, SIZE));
    }
#else
    expression(&T, 0, SIZE);
#endif

    {
        int i, op = 0;
        for(i=0; i<SIZE; ++i) {
            switch(buffer[i]) {
                case '+': case '-': case '*': case '/': ++op; break;
                default: break;
            }
        }
        if(op > MAX_OP) Backtrack();
    }

    {
        formula *f = 
#if defined(__SSE4_1__)
#define ALIGN (sizeof(SIMD_TYPE))
            aligned_alloc(ALIGN, ((sizeof(formula)+ALIGN-1)/ALIGN)*ALIGN);
#else
            malloc(sizeof(formula));
#endif
        int i;

        assert(f!=NULL);
        memset(f, 0, sizeof(formula));

        f->unused = MSKall;
        for(i=0; i<SIZE; ++i) {
            mask m = char_to_mask(buffer[i]);
            f->symbols[i] =  m;
            f->unused    &= ~m;
        }
        f->used_count  = popcount(MSKall ^ f->unused);
        ARRAY_ADD(formulae, f);
    }

#ifdef DEBUGxx
{
    static int num = 0; int i;
    for(i=0; i<SIZE; ++i) putchar(buffer[i]);
    printf("\t#%d\n", ++num);
}
#else
    progress(LLONG_MAX);
#endif

    Backtrack();
}

/*****************************************************************************/

typedef struct state {
    masks   _impossible;
#define impossible _impossible.masks
    mask    mandatory;
} state;

#ifdef DEBUG
PRIVATE void state_print(state *state) {
    int i;
    printf("mandatory: "); mask_print(state->mandatory);
    printf("\npossible:");
    for(i=0; i<SIZE; ++i) {
        printf(" ");
        mask_print(MSKall ^ state->impossible[i]);
    }
    printf("\n");
}
#endif

PRIVATE void state_relax(state *s) {
    mask imp = MSKall;
    int i;

    for(i=0; i<SIZE; ++i) imp &= s->impossible[i];
    for(i=0; i<SIZE; ++i) {
        mask m = MSKall ^ s->impossible[i];
        if((m & -m)==m) s->impossible[i] = imp;
    }
	// s->mandatory = 0;
#ifdef DEBUG
    printf("relaxed state:\n");
    state_print(s);
#endif
}

#define GREEN   0   /* must be 0 */
#define YELLOW  1
#define BLACK   2

PRIVATE void state_init(state *s) {
    int i;

    for(i=0; i<SIZE; ++i) s->impossible[i] = MSKnone;
    s->mandatory  = MSKnone;
}

// returns false if update set is empty
PRIVATE bool state_update(state *st, mask *formula, int colors) {
    mask yellow_ones = MSKnone;
    mask forbidden   = MSKnone;
    int i; div_t r;

    // update yellow
    for(r.quot=colors, i=0; i<SIZE; ++i) {
        r = div(r.quot, 3);
        switch(r.rem) {
            case YELLOW: {
                mask m = formula[i];
                st->impossible[i] |= m;
                st->mandatory     |= m;
                yellow_ones       |= m;
            }
        }
    }

    // update green & find impossible ones
    for(r.quot=colors, i=0; i<SIZE; ++i) {
        mask m = formula[i];
        r = div(r.quot, 3);
        switch(r.rem) {
            case GREEN:
                // should not happen
                //if(st->impossible[i] & m) return false;
                st->impossible[i] = MSKall ^ m;
                st->mandatory    |=  m;
#ifdef NUMBLE
                if(m==MSKequ) forbidden |= m;
#endif
            break;
            case BLACK:
                if(MSKnone == (yellow_ones & m)) {
                    forbidden |= m;
                }
            break;
        }
    }

    // remove impossible ones
    for(i=0; i<SIZE; ++i) {
        mask m = MSKall ^ st->impossible[i];
        if((m & -m) != m) {
            st->impossible[i] |= forbidden;
            m &= ~forbidden;
        }
        if(MSKnone == m) return false;
    }

    return true;
}

PRIVATE bool state_compatible(state *state, formula *formula) {
    if((state->mandatory & formula->unused)) { // bitwise and
        return false; // some mandatory are not present
    } else {
#ifdef SIMD_TYPE
        int i = sizeof(formula->_symbols.vect)/sizeof(formula->_symbols.vect[0]);
        SIMD_TYPE *imp = state->_impossible.vect;
        SIMD_TYPE *sym = formula->_symbols.vect;
        do {
#if (CONFIG&16)
#ifdef __SSE4_1__
            if(!(_mm_test_all_zeros(*sym++, *imp++))) return false;
#else
            if((*imp++ & *sym++)) return false;
#endif
#else
#ifdef __SSE4_1__
            if(!(_mm_test_all_zeros(sym[i-1], imp[i-1]))) return false;
#else
            if((imp[i-1] & sym[i-1])) return false;
#endif
#endif
        } while(--i);
        return true;
#else       
        mask *sym = formula->symbols, *imp = state->impossible, acc = MSKnone;
        int i = SIZE;
        // do acc |= (*sym++ & *imp++); while(--i);
        do acc = (*sym++ & *imp++); while(!acc && --i);
        // do acc |= (*sym++ & *imp++); while(!acc && --i);
        // for(i=0;i<SIZE;++i) acc |= sym[i] & imp[i];
        return acc==MSKnone;
#endif
    }
}

PRIVATE int state_compatible_count(state *state, int threshold, formula **sample) {
    int n = 0;
    while(*sample) {
        if(state_compatible(state, *sample++)) {
            if(++n>threshold) {
                break;
            }
        }
    }
    return n;
}

/* find the worst number of incompatible states for the
   current candidate */
typedef struct {
    formula **samples;
    formula **candidates;
    int worst; 
	int worst_c;
	int least_c;
    int all_colors;
} least_worst_data;

PRIVATE void find_worst2(least_worst_data *data, state *state, int color, formula *candidate) {
    struct state state2 = *state;
    if(state_update(&state2, candidate->symbols, color)) {
		int count = state_compatible_count(&state2, data->least_c, data->samples);
		if(count > data->worst) {
#ifdef _OPENMP
			if(nthreads>1) 
			#pragma omp critical
			{
				if(count > data->worst) {
					data->worst   = count;
					data->worst_c = color;
				}
			}
			else
#endif
			{
				data->worst   = count;
				data->worst_c = color;
			}
		}
	}
}

PRIVATE void find_worst(least_worst_data *data, state *state, formula *candidate) {
	data->worst = 0;
#ifdef _OPENMP
    if(nthreads>1)
    #pragma omp parallel 
    {
        int least_c = data->least_c;
        int color = data->all_colors + omp_get_thread_num();
        while((color-=nthreads)>=0 && data->worst<least_c) {
			find_worst2(data, state, color, candidate);
        }
    } else
#endif
    {
		int least_c = data->least_c;
        int color = data->all_colors;  
        while(--color>=0 && data->worst<least_c) {
			find_worst2(data, state, color, candidate);
		}
    }
}

#define ARRAY_NULL(A) ARRAY_AT((A),(A).len) = NULL

PRIVATE bool least_worst(state *state, int round) {
    const double max_ops =((double)MAX_CANDIDATES)*MAX_SAMPLES*nthreads;
    const int all_colors = ipow(3,SIZE);

    int rnd_thr = -1;
    
	int least1, least2;	
    double num_ops;
    int i;

    ARRAY_DECL(formula *, candidates);
    ARRAY_DECL(formula *, samples);
    formula *least_f = formulae.tab[0];
	
	formula **candidate2_tab;
	int       candidate2_len;
	
	long long p = 0;
    
    least_worst_data data;

    if(formulae.len == 0) return false;

    if(formulae.len == 1) {
        printf("Only one possible equation.\n");
        for(i=0; i<SIZE; ++i) {
            buffer[i] = mask_to_char(formulae.tab[0]->symbols[i]);
        }
        return true;
    }

    data.all_colors = all_colors;
	least2 = least1 = formulae.len;

    printf("Finding least worst equation..."); fflush(stdout);
	// keep valid candidates
    ARRAY_CPY(candidates, found);
	struct state st = *state;
	if(round<=1) state_relax(&st);
	for(i=0; i<candidates.len;) {
		if(state_compatible(&st, candidates.tab[i])) 
			++i;
		else ARRAY_REM(candidates, i);
	}

	// reduce if too big
    if(candidates.len >= MAX_CANDIDATES) {
        for(i=0; i<candidates.len;) {
            if(candidates.tab[i]->used_count==SIZE)
                ++i;
            else ARRAY_REM(candidates, i);
        }
        #if ALLOW_PARENTHESIS
        if(candidates.len >= MAX_CANDIDATES) {
            for(i=0; i<candidates.len;) {
                if((candidates.tab[i]->unused & MSKbra))
                    ++i;
                else ARRAY_REM(candidates, i);
            }
        }
        #endif
        if(candidates.len >= MAX_CANDIDATES) {
            for(i=0; i<candidates.len;) {
                if((candidates.tab[i]->unused & MSK0))
                    ++i;
                else ARRAY_REM(candidates, i);
            }
        }
        if(candidates.len >= MAX_CANDIDATES) {
            for(i=0; i<candidates.len;) {
                if((candidates.tab[i]->unused & MSKdiv))
                    ++i;
                else ARRAY_REM(candidates, i);
            }
        }
        printf("sel(%u)...", (unsigned)candidates.len); fflush(stdout);
    }
    
	// get samples
    // printf("%d %d\n", candidates.len, all_colors);
    ARRAY_CPY(samples, formulae);
    num_ops= all_colors*candidates.len*samples.len;
    if(num_ops >= max_ops) {
        double f = max_ops / num_ops;
        if(f<MIN_SAMPLE_RATIO) f = MIN_SAMPLE_RATIO;
        printf("sample(%.01f%%)...", 100*f); fflush(stdout);
        rnd_thr = RAND_MAX * f;
    }
    
    ARRAY_NULL(candidates);
    ARRAY_NULL(samples);
    data.candidates   = candidates.tab;
    data.samples      = samples.tab;

	candidate2_tab = found.tab;
	candidate2_len = found.len;
	
    progress(-(long long)candidates.len*(long long)candidate2_len);
    for(i=0; i<candidates.len; ++i, p += candidate2_len) {
        
        /* refesh our sample list from time to time */
        if(rnd_thr>=0 && 0==(i & 7)) {
            int j;
            while(samples.len>0) ARRAY_REM(samples, 0);
            for(j=0; j<formulae.len; ++j) if(j==i || rand()<=rnd_thr) {
                ARRAY_ADD(samples, formulae.tab[j]);
            }
            ARRAY_NULL(samples);
        }

        data.least_c = least1;
		data.samples = samples.tab;
        find_worst(&data, state, candidates.tab[i]);
		// for(int k=0; k<SIZE; ++k) putchar(mask_to_char(candidates.tab[i]->symbols[k]));
		// printf(" = %d\n", data.worst);
        
        /* keep the least-worse candidate */
        if(data.worst <= least1) {
			bool lt = data.worst < least1;
			// printf("eq=%d %d %d\n", eq, data.worst, least1);

			struct state state2 = *state;
			state_update(&state2, candidates.tab[i]->symbols, data.worst_c);
			
			struct state st	= state2; if(round+1<=1) state_relax(&st);
			
			int j; 

			// TODO early exit when least1=1 or 0 ?
			least1 = data.worst;
			for(j=0; j<candidate2_len; progress(p + j++)) if(state_compatible(&st, candidate2_tab[j])) {
				data.least_c = least2;
				// data.samples = formulae.tab;
				find_worst(&data, &state2, candidate2_tab[j]);
				// if((data.worst < least2) || (!eq && data.worst <= least2)) {
					// printf("%d %d %d\n", eq, data.worst, least2);
				if(data.worst < least2 || (lt && data.worst <= least2)) { 
#if 1 //def DEBUG
					int  k;
					printf("\n%5d %5d [", least1, data.worst);
					for(k=0; k<SIZE; ++k) putchar(mask_to_char(candidates.tab[i]->symbols[k]));
					putchar(' ');
					for(k=0; k<SIZE; ++k) putchar(mask_to_char(candidate2_tab[j]->symbols[k]));
					putchar(']');
					fflush(stdout);
#endif
					least_f = candidates.tab[i];
					least2  = data.worst;
					lt = false;
				}
			}
		}
    }
    ARRAY_DONE(samples);
    ARRAY_DONE(candidates);
    printf("done");
    if((i=progress(0))>1) printf(" (%s%d%s secs)", A_BOLD, i, A_NORM);
    printf("\n");
    for(i=0; i<SIZE; ++i) {
        buffer[i] = mask_to_char(least_f->symbols[i]);
    }
#ifdef DEBUG
    printf("least=");
    for(i=0; i<SIZE; ++i) putchar(buffer[i]);
    printf(" (%d / %d)\n", data.least_c, formulae.len);
#endif
    return data.least_c>0;
}

/*****************************************************************************/

PRIVATE jmp_buf _env;
PRIVATE void _return(void) {
    longjmp(_env, 1);
}
#define _Backtracking(S) do {                       \
    void (*_Fiasco)(void) = Fiasco;                 \
    Fiasco = _return;                               \
    if(!setjmp(_env)) do Backtracking(S) while(0);  \
    else Fiasco = _Fiasco;                          \
} while(0)

/*****************************************************************************/

PRIVATE void remove_impossible(state *s) {
#ifdef DEBUG
    size_t before = formulae.len;
#endif
    int i;
    for(i = 0; i<formulae.len;) {
        if(state_compatible(s, formulae.tab[i]))
            ++i;
        else ARRAY_REM(formulae, i);
    }
#ifdef DEBUG
    printf("Removed: %d\n", before - formulae.len);
#endif
}

PRIVATE void remove_played(mask symbs[SIZE]) {
	int i;
	for(i=0; i<formulae.len; ++i)
		if(!memcmp(formulae.tab[i]->symbols, symbs, SIZE*sizeof(mask))) {
			ARRAY_REM(formulae, i);
			break;
		}
	// for(i=0; i<found.len; ++i)
		// if(!memcmp(found.tab[i]->symbols, symbs, SIZE*sizeof(mask))) {
			// ARRAY_REM(found, i);
			// break;
		// }
}

PRIVATE bool play_round(state *state, int round) {
    int colors;
    while(true) {
        int i, index;
        mask symbs[SIZE];
        struct state back = *state;
        int c;

        printf(formulae.len>1 ? "Try: %s" : "Sol: %s", A_BOLD);
        for(i=0; i<SIZE; ++i) {
            symbs[i] = char_to_mask(buffer[i]);
            putchar(buffer[i]);
        }
        printf("%s\n", A_NORM);

#if !defined(NUMBLE) || !defined(DEBUG)
        if(formulae.len<=1) {
            --formulae.len;
            fflush(stdout);
            return false;
        }
#endif
        printf("Ans: ");
        fflush(stdout);

        for(colors = i = 0, index = 1; i<SIZE; ) {
            int code = -1;
            switch((c = getchar())) {
                case EOF: exit(0); break;

                case ' ': case '\r': case '\n': case '\t': break;

                case '!': code = GREEN;  break;
                case '+': code = YELLOW; break;
                case '-': code = BLACK;  break;

                default:
                printf("ERROR, invalid char: %c\nAns: ", (char)c);
                fflush(stdout); colors = i = 0; index = 1; 
                while(c!='\n') c = getchar();
                break;
            }
            if(code>=0) {
                colors += code*index;
                ++i; index *= 3;
            }
        }
        while(c!='\n') c = getchar();

        if(0 == colors) {
            --formulae.len;
            fflush(stdout);
            return false;
        }

        state_update(state, symbs, colors);
#ifdef DEBUG
        state_print(state);
#endif
        if(0 == state_compatible_count(state, INT_MAX, formulae.tab)) {
            printf("ERROR, invalid colors: ");
            for(i=0; i<SIZE; ++i, colors /= 3) {
                switch(colors % 3) {
                    case GREEN:  putchar('!'); break;
                    case YELLOW: putchar('+'); break;
                    case BLACK:  putchar('-'); break;
                }
                buffer[i] = mask_to_char(symbs[i]);
            }
            printf("\n");
            *state = back;
        } else {
            bool ok;
            remove_impossible(state);
			remove_played(symbs);
            ok = least_worst(state, round);
            if(ok) break;
        }
    }

    return colors!=0;
}

/*****************************************************************************/

#if DO_SORT
PRIVATE int cmp_formula(const void *_a, const void *_b) {
    formula * const *x = _a, * const *y = _b;
    const formula *a = *x, *b = *y;
    int d=0;
#if ALLOW_PARENTHESIS
#if (CONFIG&8)
    d = (b->unused & MSKbra) - (a->unused & MSKbra);
#endif
#endif
#if (CONFIG&1)
    if(d==0) d = b->used_count - a->used_count;
#else
    if(d==0) d = a->used_count - b->used_count;
#endif
#if (CONFIG&2)
    int i = SIZE; while(d==0 && --i>=0) d =
#else
    int i; for(i=0;d==0 && i<SIZE;++i) d =
#endif
#if (CONFIG&4)
        b->symbols[i] - a->symbols[i];
#else
        a->symbols[i] - b->symbols[i];
#endif
    // if(d==0) d = a->used - b->used;
    // int i; for(i=0; d==0 && i<SIZE; ++i) d = (*b)->symbols[i] - (*a)->symbols[i];
    return -d;
}

PRIVATE void sort_formulae(void) {
    // int i;

    qsort(formulae.tab, formulae.len, sizeof(*formulae.tab), cmp_formula);

    // for(i=0; i<formlae.len; ++i) {
        // int j;
        // for(j=0; j<SIZE; ++j) putchar(mask_to_char(formulaetab[i]->symbols[j]));
        // putchar('\n');
    // }
}
#endif

#if DO_SHUFFLE
PRIVATE void shuffle_formulae(void) {
    int i = formulae.len;

    while(i>1) {
        int j = rand() % i--;
        formula *t = formulae.tab[i];
        formulae.tab[i] = formulae.tab[j];
        formulae.tab[j] = t;
    }

    // for(i = formulae.len; i--;) {
        // int j;
        // for(j=0; j<SIZE; ++j) putchar(mask_to_char(formulae.tab[i]->symbols[j]));
        // putchar('\n');
    // }
}
#endif

/*****************************************************************************/

PRIVATE void title(void) {
    char *TITLE1 = "Helper for ";
    char *TITLE2 = " by Samuel Devulder";
    const int len = strlen(TITLE1) + strlen(URL) + strlen(TITLE2);
    int i;
    putchar('\r');
    for(i=len; --i>=0; putchar('~')){} putchar('\n');
    printf("%s%s%s%s%s\n", TITLE1, A_BOLD, URL, A_NORM, TITLE2);
    for(i=len; --i>=0; putchar('~')){} putchar('\n');
}

/*****************************************************************************/

int main(int argc, char **argv) {
    state state;
    rat target;
    int i = 0;

    srand(time(0));
    setlocale(LC_ALL, "");

    if(isatty(fileno(stdout))) {
        A_BOLD = "\033[1m";
        A_NORM = "\033[0m";
    }

    title();

    if(argc>1) {
#ifdef NUMBLE
        rat_integer(&target, 0);
#else
        rat_double(&target, atof(argv[1]));
#endif
    } else {
        int ignored;
#if defined(_WIN32) || defined(__CYGWIN__)
        ignored = system("cmd /c start " URL);
#elif defined(__linux__) || defined(__unix__)
        ignored = system("xdg-open " URL);
#elif defined(__APPLE__)
        ignored = system("open " URL);
#endif
        (void)ignored;
#ifdef NUMBLE
        rat_integer(&target, 0);
#else
        double x = 0;
        printf("Num? ");
        fflush(stdout);
        while(scanf("%lf", &x)!=1);
        rat_double(&target, x);
#endif
    }

#ifdef _OPENMP
#pragma omp parallel
    {
        #pragma omp single
        nthreads = omp_get_num_threads();
    }
    if(nthreads>1) printf("Using %s%d%s threads.\n", A_BOLD, nthreads, A_NORM);
#endif

    // be nice with the other processes
    {int ignored=nice(20);(void)ignored;}

#ifdef NUMBLE
    do {
        if(found.len==0) {
            printf("Finding equations..."); fflush(stdout);
        }
#else
        if(rat_whole(&target))
                printf("Finding equations for %s%d%s...",
                    A_BOLD, target.p, A_NORM);
        else    printf("Finding equations for %s%d/%d%s...",
                    A_BOLD, target.p, target.q, A_NORM);
        fflush(stdout);
#endif

        if(found.len==0) {
            formulae.len = 0;
            progress(-1); _Backtracking(findall(&target)); i = progress(0);
            printf("done ("); if(i>1) printf("%s%d%s secs, ", A_BOLD, i, A_NORM);
            printf("%s%'u%s found)\n", A_BOLD, (unsigned)formulae.len, A_NORM);
#if DO_SHUFFLE
	        shuffle_formulae();
#endif
#if DO_SORT
			sort_formulae();
#endif
            ARRAY_CPY(found, formulae);
        } else {
            ARRAY_CPY(formulae, found);
        }

        ARRAY_NULL(formulae);
		ARRAY_NULL(found);
        state_init(&state);
#if NUMBLE
        memcpy(buffer, "9*42=378", SIZE);
#else
        least_worst(&state, 0);
#endif
        i=0; do ++i; while(play_round(&state, i));
        printf("Solved in %s%d%s round%s.\n", A_BOLD, i, A_NORM, i>1?"s":"");
        if(formulae.len>0)
            printf("You were lucky. There existed %s%u%s other possibilit%s.\n", 
                A_BOLD, (unsigned)formulae.len, A_NORM, formulae.len>1?"ies":"y");
#ifdef NUMBLE
        if(formulae.len==0) {
            printf("Press enter..."); fflush(stdout); 
            while('\n'!=getchar());
        }
        title();
    } while(true);
#endif
    while(found.len) {free(found.tab[0]); ARRAY_REM(found, 0);}
    ARRAY_DONE(found);
    ARRAY_DONE(formulae);
    return 0;
}
