#ifndef IEEE_UTILS_H
#define IEEE_UTILS_H

#define PI       3.141592653589793238462643383279502884
#define TWO_PI   6.283185307179586476925286766559005768
#define PI_2     1.570796326794896619231321691639751442
#define PI_4     0.785398163397448309615660845819875721
#define E1       2.718281828459045235360287471352662498

typedef union {
    float x; 
    struct {
        unsigned int f : 23; 
        unsigned int E : 8;  
        unsigned int s : 1;  
    } bits;
} FloatIEEE;

typedef union {
    double x; 
    struct {
        unsigned long long f : 52; 
        unsigned long long E : 11; 
        unsigned long long s : 1;  
    } bits;
} DoubleIEEE;

#define InvertSign(y) ((y).bits.s ^= 1)

#define CORDIC_SHIFT_DOUBLE(val, i) do { \
    if ((val).bits.E > (unsigned int)(i)) { \
        (val).bits.E -= (i);             \
    } else {                             \
        (val).bits.E = 0;                \
        (val).bits.f = 0;                \
    }                                    \
} while(0)


/* ====================================================================
 * PRIMITIVAS MATEMÁTICAS BARE-METAL (Substitutas da math.h)
 * ==================================================================== */

/* 1. Geração de Not-a-Number (NaN) e Infinito via máscara de bits */
static inline double cordic_nan(void) {
    DoubleIEEE v;
    v.bits.s = 0; v.bits.E = 2047; v.bits.f = 1; /* Fração não nula */
    return v.x;
}

static inline double cordic_inf(int sign) {
    DoubleIEEE v;
    v.bits.s = (sign < 0) ? 1 : 0; v.bits.E = 2047; v.bits.f = 0; /* Fração nula */
    return v.x;
}

/* 2. Validações IEEE 754 (Substitui isnan e isinf) */
static inline int cordic_isnan(double x) {
    DoubleIEEE v; v.x = x;
    return (v.bits.E == 2047 && v.bits.f != 0);
}

static inline int cordic_isinf(double x) {
    DoubleIEEE v; v.x = x;
    return (v.bits.E == 2047 && v.bits.f == 0);
}

/* 3. Valor Absoluto (Substitui fabs): Basta zerar o bit de sinal */
static inline double cordic_fabs(double x) {
    DoubleIEEE v; v.x = x;
    v.bits.s = 0; 
    return v.x;
}

/* 4. Arredondamento (Substitui round usado na exponencial) */
static inline double cordic_round(double x) {
    return (x >= 0.0) ? (double)(long long)(x + 0.5) : (double)(long long)(x - 0.5);
}

/* 5. Redução de Domínio (Substitui fmod(x, TWO_PI) no cordic_circular.c) */
static inline double cordic_fmod_twopi(double x) {
    long long quotient = (long long)(x / TWO_PI);
    return x - (quotient * TWO_PI);
}


void print_float_components(FloatIEEE val);
void print_double_components(DoubleIEEE val);

#endif