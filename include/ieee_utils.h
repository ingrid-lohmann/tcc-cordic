#ifndef IEEE_UTILS_H
#define IEEE_UTILS_H

#include <math.h>

#define PI 3.14159265358979323846264   
#define E1 2.718281828459045235360287  

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

/* * JUSTIFICATIVA DE ARQUITETURA (TCC):
 * As macros MULT_NICE_NUMBER e MULT_NICE_NUMBER_NEG foram usadas em uma
 * versao anterior do projeto para multiplicacoes por constantes fixas. Com a
 * implementacao atual, elas deixaram de ser adequadas ao calculo do CORDIC.
 *
 * Em cada iteracao, o CORDIC usa um valor diferente de i. O deslocamento
 * corresponde a dividir o numero por 2^i, portanto nao faz sentido manter uma
 * macro baseada em valores fixos. A macro CORDIC_SHIFT_DOUBLE atua diretamente
 * sobre o expoente e acompanha a operacao feita em cada iteracao.
 *
 * O nucleo tambem passou a usar double para reduzir a perda de precisao nas
 * operacoes e permitir a convergencia em ate 24 iteracoes. Essa mudanca foi o
 * motivo para deixar o codigo anterior de lado: ele estava ligado a uma forma
 * de calculo que ja nao correspondia ao algoritmo e usava uma precisao menor.
 *
 * Em deslocamentos maiores, o expoente pode ser menor que o valor de i. Como
 * o campo 'E' e sem sinal, uma subtracao direta faria o valor dar a volta e
 * poderia gerar um resultado interpretado pela FPU como infinito. A macro
 * verifica essa condicao e zera o valor quando o deslocamento nao pode ser
 * representado. Assim, os calculos circular e hiperbolico mantem um resultado
 * definido durante as iteracoes.
 */

#define CORDIC_SHIFT_DOUBLE(val, i) do { \
    if ((val).bits.E > (unsigned int)(i)) { \
        (val).bits.E -= (i);             \
    } else {                             \
        (val).bits.E = 0;                \
        (val).bits.f = 0;                \
    }                                    \
} while(0)

/* * CODIGO ANTIGO DESCONTINUADO:
 * * #define MULT_NICE_NUMBER(y, w, k) do { \
 * (w).x = (y).x;                     \
 * (w).bits.E += (k);                 \
 * (w).x += (y).x;                    \
 * } while(0)
 * * #define MULT_NICE_NUMBER_NEG(y, w, k) do { \
 * (w).x = -(y).x;                        \
 * (w).bits.E += (k);                     \
 * (w).x += (y).x;                        \
 * } while(0)
 */

void print_float_components(FloatIEEE val);

#endif

