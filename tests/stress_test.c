#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <math.h>
#include <float.h>

#include "cordic_circular.h"
#include "cordic_hyperbolic.h"
#include "cordic_transcendental.h"
#include "ieee_utils.h"

#define CORDIC_ITERS CORDIC_ITERATIONS

static void test_edge_case(const char *category, const char *expr_name, double input, double cordic_res, double math_res) {
    (void)category;

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
    double near_pi2 = (PI / 2.0) - 1e-6;
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

    printf("--- [2] Cobertura das Demais Funcoes CORDIC ---\n");

    double circular_value = 0.5;
    test_edge_case("Circular", "atan(x)", circular_value,
                   cordic_circular_atan(circular_value, CORDIC_ITERS), atan(circular_value));

    CordicVector circular_rotate_input = { .x.x = CORDIC_K_CIRCULAR, .y.x = 0.0, .z.x = PI_4 };
    CordicVector circular_rotate_result = cordic_circular_rotate(circular_rotate_input, CORDIC_ITERS);
    test_edge_case("Circular", "rotate.x", PI_4, circular_rotate_result.x.x, cos(PI_4));
    test_edge_case("Circular", "rotate.y", PI_4, circular_rotate_result.y.x, sin(PI_4));
    test_edge_case("Circular", "rotate.z", PI_4, circular_rotate_result.z.x, 0.0);

    CordicVector circular_vector_input = { .x.x = 1.0, .y.x = circular_value, .z.x = 0.0 };
    CordicVector circular_vector_result = cordic_circular_vector(circular_vector_input, CORDIC_ITERS);
    test_edge_case("Circular", "vector.x", circular_value, circular_vector_result.x.x,
                   sqrt(1.0 + circular_value * circular_value) / CORDIC_K_CIRCULAR);
    test_edge_case("Circular", "vector.y", circular_value, circular_vector_result.y.x, 0.0);
    test_edge_case("Circular", "vector.z", circular_value, circular_vector_result.z.x, atan(circular_value));

    double hyperbolic_angle = 0.5;
    double sinh_value, cosh_value;
    cordic_hyperbolic_sinh_cosh(hyperbolic_angle, &sinh_value, &cosh_value, CORDIC_ITERS);
    test_edge_case("Hiperbolica", "sinh(x)", hyperbolic_angle, sinh_value, sinh(hyperbolic_angle));
    test_edge_case("Hiperbolica", "cosh(x)", hyperbolic_angle, cosh_value, cosh(hyperbolic_angle));
    test_edge_case("Hiperbolica", "tanh(x)", hyperbolic_angle,
                   cordic_hyperbolic_tanh(hyperbolic_angle, CORDIC_ITERS), tanh(hyperbolic_angle));

    double hyperbolic_value = 1.0 / 3.0;
    test_edge_case("Hiperbolica", "atanh(x)", hyperbolic_value,
                   cordic_hyperbolic_atanh(hyperbolic_value, CORDIC_ITERS), atanh(hyperbolic_value));

    CordicVector hyperbolic_rotate_input = {
        .x.x = CORDIC_K_HYPERBOLIC, .y.x = 0.0, .z.x = hyperbolic_angle
    };
    CordicVector hyperbolic_rotate_result = cordic_hyperbolic_rotate(hyperbolic_rotate_input, CORDIC_ITERS);
    test_edge_case("Hiperbolica", "rotate.x", hyperbolic_angle,
                   hyperbolic_rotate_result.x.x, cosh(hyperbolic_angle));
    test_edge_case("Hiperbolica", "rotate.y", hyperbolic_angle,
                   hyperbolic_rotate_result.y.x, sinh(hyperbolic_angle));
    test_edge_case("Hiperbolica", "rotate.z", hyperbolic_angle,
                   hyperbolic_rotate_result.z.x, 0.0);

    CordicVector hyperbolic_vector_input = { .x.x = 1.0, .y.x = hyperbolic_value, .z.x = 0.0 };
    CordicVector hyperbolic_vector_result = cordic_hyperbolic_vector(hyperbolic_vector_input, CORDIC_ITERS);
    test_edge_case("Hiperbolica", "vector.x", hyperbolic_value, hyperbolic_vector_result.x.x,
                   sqrt(1.0 - hyperbolic_value * hyperbolic_value) / CORDIC_K_HYPERBOLIC);
    test_edge_case("Hiperbolica", "vector.y", hyperbolic_value, hyperbolic_vector_result.y.x, 0.0);
    test_edge_case("Hiperbolica", "vector.z", hyperbolic_value,
                   hyperbolic_vector_result.z.x, atanh(hyperbolic_value));

    double exp_input = 1.0;
    test_edge_case("Transcendental", "exp(x)", exp_input,
                   cordic_exp(exp_input, CORDIC_ITERS), exp(exp_input));
    double pow_base = 1.5;
    double pow_exponent = 1.25;
    test_edge_case("Transcendental", "pow(x,y)", pow_base,
                   cordic_pow(pow_base, pow_exponent, CORDIC_ITERS), pow(pow_base, pow_exponent));
    printf("=========================================================================================================\n\n");

    /* -------------------------------------------------------------------------
     * 3. Varredura Fina Contínua do 1º Quadrante (Exportação para CSV)
     * ------------------------------------------------------------------------- */
    char csv_file[64];
    snprintf(csv_file, sizeof(csv_file), "tests/stress_test_domain_%d.csv", CORDIC_ITERS);
    FILE *fp = fopen(csv_file, "w");
    if (fp == NULL) {
        perror("Erro ao criar stress_test_domain.csv");
        return 1;
    }

    fprintf(fp, "Angle_Rad,Sin_AbsErr,Sin_RelErr,Cos_AbsErr,Cos_RelErr,Tan_AbsErr,Tan_RelErr\n");

    double step = 0.005; /* ~315 pontos entre 0 e pi/2 */
    double max_err_sin = 0.0, max_err_cos = 0.0, max_err_tan = 0.0;
    double worst_angle_sin = 0.0, worst_angle_cos = 0.0;

    for (double theta = 0.0; theta <= (PI / 2.0); theta += step) {
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
        if (err_t > max_err_tan && theta < (PI / 2.0 - 0.05)) { /* ignora singularidade exata de pi/2 */
            max_err_tan = err_t;
        }

        fprintf(fp, "%.6f,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e\n",
                theta, err_s, rel_s, err_c, rel_c, err_t, rel_t);
    }

    fflush(fp);
    fclose(fp);

    printf("--- [3] Resumo da Varredura Continua [0, pi/2] ---\n");
    printf("Total de pontos analisados: %d\n", (int)((PI / 2.0) / step) + 1);
    printf("Pior Erro Absoluto Seno:    %1.2e (em theta = %.4f rad)\n", max_err_sin, worst_angle_sin);
    printf("Pior Erro Absoluto Cosseno: %1.2e (em theta = %.4f rad)\n", max_err_cos, worst_angle_cos);
    printf("Pior Erro Absoluto Tangente (fora do polo): %1.2e\n", max_err_tan);
    printf("\nArquivo com a curva completa salvo em: '%s'\n", csv_file);
    printf("=========================================================================================================\n");

    return 0;
}