#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <math.h>
#include <float.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "cordic_circular.h"
#include "cordic_hyperbolic.h"
#include "cordic_transcendental.h"

#define CORDIC_ITERS 24

static void test_edge_case(const char *category, const char *expr_name, double input, double cordic_res, double math_res) {
    double abs_err = fabs(cordic_res - math_res);
    double rel_err = (fabs(math_res) > 1e-15) ? (abs_err / fabs(math_res)) : abs_err;
    
    printf("%-20s | in: %12.4e | CORDIC: %16.10e | math.h: %16.10e | AbsErr: %1.2e | RelErr: %1.2e\n",
           expr_name, input, cordic_res, math_res, abs_err, rel_err);
}

int main(void) {
    printf("=========================================================================================================\n");
    printf("                               BANCADA DE TESTE NUMERICO DE ESTRESSE                                     \n");
    printf("                  Iteracoes: %d | Formato: IEEE 754 64-bit | Alvo: Casos Limite e Singularidades         \n", CORDIC_ITERS);
    printf("=========================================================================================================\n\n");

    /* -------------------------------------------------------------------------
     * 1. Testes em Casos Limite e Pontos Críticos
     * ------------------------------------------------------------------------- */
    printf("--- [1] Casos Limite e Entradas Proximas a Zero / Fronteiras ---\n");
    
    /* 1.1 Entradas extremamente pequenas (Verifica underflow / CORDIC_SHIFT_DOUBLE) */
    double tiny_inputs[] = {1e-3, 1e-6, 1e-9, 1e-12};
    for (int i = 0; i < 4; i++) {
        double x = tiny_inputs[i];
        double s, c;
        cordic_circular_sin_cos(x, &s, &c, CORDIC_ITERS);
        test_edge_case("Pequeno", "sin(x)", x, s, sin(x));
        test_edge_case("Pequeno", "cos(x)", x, c, cos(x));
    }
    printf("---------------------------------------------------------------------------------------------------------\n");

    /* 1.2 Proximidade de pi/2 para Tangente (Quase assíntota vertical) */
    double near_pi2 = (M_PI / 2.0) - 1e-6;
    test_edge_case("Assintota", "tan(pi/2 - 1e-6)", near_pi2, 
                   cordic_circular_tan(near_pi2, CORDIC_ITERS), tan(near_pi2));

    /* 1.3 Fronteira superior de convergencia de atanh(x) (dominio aberto (-1, 1)) */
    double near_one_atanh = 0.9990;
    test_edge_case("Borda Dominio", "atanh(0.999)", near_one_atanh, 
                   cordic_hyperbolic_atanh(near_one_atanh, CORDIC_ITERS), atanh(near_one_atanh));

    /* 1.4 Cruzamento de chaveamento de expoente (potencia de 2) para ln(x) e sqrt(x) */
    double trans_inputs[] = {1.999999, 2.000001, 3.999999, 4.000001};
    for (int i = 0; i < 4; i++) {
        double x = trans_inputs[i];
        test_edge_case("Transicao Exp", "ln(x)", x, cordic_ln(x, CORDIC_ITERS), log(x));
        test_edge_case("Transicao Exp", "sqrt(x)", x, cordic_sqrt(x, CORDIC_ITERS), sqrt(x));
    }
    printf("=========================================================================================================\n\n");

    /* -------------------------------------------------------------------------
     * 2. Varredura Fina Contínua do 1º Quadrante (Exportação para CSV)
     * ------------------------------------------------------------------------- */
    const char *csv_file = "stress_test_domain.csv";
    FILE *fp = fopen(csv_file, "w");
    if (fp == NULL) {
        perror("Erro ao criar stress_test_domain.csv");
        return 1;
    }

    fprintf(fp, "Angle_Rad,Sin_AbsErr,Sin_RelErr,Cos_AbsErr,Cos_RelErr,Tan_AbsErr,Tan_RelErr\n");

    double step = 0.005; /* ~315 pontos entre 0 e pi/2 */
    double max_err_sin = 0.0, max_err_cos = 0.0, max_err_tan = 0.0;
    double worst_angle_sin = 0.0, worst_angle_cos = 0.0;

    for (double theta = 0.0; theta <= (M_PI / 2.0); theta += step) {
        double s, c;
        cordic_circular_sin_cos(theta, &s, &c, CORDIC_ITERS);
        double ref_s = sin(theta);
        double ref_c = cos(theta);

        double err_s = fabs(s - ref_s);
        double err_c = fabs(c - ref_c);

        double rel_s = (fabs(ref_s) > 1e-12) ? (err_s / fabs(ref_s)) : err_s;
        double rel_c = (fabs(ref_c) > 1e-12) ? (err_c / fabs(ref_c)) : err_c;

        double t = cordic_circular_tan(theta, CORDIC_ITERS);
        double ref_t = tan(theta);
        double err_t = fabs(t - ref_t);
        double rel_t = (fabs(ref_t) > 1e-12) ? (err_t / fabs(ref_t)) : err_t;

        if (err_s > max_err_sin) {
            max_err_sin = err_s;
            worst_angle_sin = theta;
        }
        if (err_c > max_err_cos) {
            max_err_cos = err_c;
            worst_angle_cos = theta;
        }
        if (err_t > max_err_tan && theta < (M_PI / 2.0 - 0.05)) { /* ignora singularidade exata de pi/2 */
            max_err_tan = err_t;
        }

        fprintf(fp, "%.6f,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e\n",
                theta, err_s, rel_s, err_c, rel_c, err_t, rel_t);
    }

    fflush(fp);
    fclose(fp);

    printf("--- [2] Resumo da Varredura Continua [0, pi/2] ---\n");
    printf("Total de pontos analisados: %d\n", (int)((M_PI / 2.0) / step) + 1);
    printf("Pior Erro Absoluto Seno:    %1.2e (em theta = %.4f rad)\n", max_err_sin, worst_angle_sin);
    printf("Pior Erro Absoluto Cosseno: %1.2e (em theta = %.4f rad)\n", max_err_cos, worst_angle_cos);
    printf("Pior Erro Absoluto Tangente (fora do polo): %1.2e\n", max_err_tan);
    printf("\nArquivo com a curva completa salvo em: '%s'\n", csv_file);
    printf("=========================================================================================================\n");

    return 0;
}