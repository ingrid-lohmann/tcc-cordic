#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "cordic_circular.h"
#include "cordic_hyperbolic.h"
#include "cordic_transcendental.h"

int main(void) {
    const char *filename = "cordic_convergence_data.csv";
    FILE *fp = fopen(filename, "w");

    if (fp == NULL) {
        fprintf(stderr, "Erro ao criar o arquivo %s\n", filename);
        return 1;
    }

    /* Cabecalho do CSV */
    fprintf(fp, "N,Theoretical_Bound,Err_Sin_PI_4,Err_Cos_PI_4,Err_Sinh_0_5,Err_Cosh_0_5,Err_Exp_1_0,Err_Ln_2_0,Err_Sqrt_2_0\n");

    /* Pontos de teste fixos para analise de convergencia */
    double theta = M_PI / 4.0; /* 45 graus para seno e cosseno */
    double u = 0.5;            /* argumento para sinh e cosh */
    double x_exp = 1.0;        /* argumento para exp */
    double x_ln = 2.0;         /* argumento para ln */
    double x_sqrt = 2.0;       /* argumento para raiz quadrada */

    /* Valores de referencia (math.h) */
    double ref_sin = sin(theta);
    double ref_cos = cos(theta);
    double ref_sinh = sinh(u);
    double ref_cosh = cosh(u);
    double ref_exp = exp(x_exp);
    double ref_ln = log(x_ln);
    double ref_sqrt = sqrt(x_sqrt);

    /* Varre de 1 a 24 iteracoes */
    for (int n = 1; n <= 24; n++) {
        double theoretical_bound = pow(2.0, -n);

        /* 1. Funcoes Circulares */
        double s_val, c_val;
        cordic_circular_sin_cos(theta, &s_val, &c_val, n);
        double err_sin = fabs(s_val - ref_sin);
        double err_cos = fabs(c_val - ref_cos);

        /* 2. Funcoes Hiperbolicas */
        double sh_val, ch_val;
        cordic_hyperbolic_sinh_cosh(u, &sh_val, &ch_val, n);
        double err_sinh = fabs(sh_val - ref_sinh);
        double err_cosh = fabs(ch_val - ref_cosh);

        /* 3. Funcoes Transcendentais Compostas */
        double exp_val = cordic_exp(x_exp, n);
        double err_exp = fabs(exp_val - ref_exp);

        double ln_val = cordic_ln(x_ln, n);
        double err_ln = fabs(ln_val - ref_ln);

        double sqrt_val = cordic_sqrt(x_sqrt, n);
        double err_sqrt = fabs(sqrt_val - ref_sqrt);

        /* Escreve a linha no CSV em notacao cientifica */
        fprintf(fp, "%d,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e\n",
                n,
                theoretical_bound,
                err_sin,
                err_cos,
                err_sinh,
                err_cosh,
                err_exp,
                err_ln,
                err_sqrt);
    }

    fclose(fp);
    printf("Arquivo '%s' gerado com sucesso!\n", filename);
    return 0;
}