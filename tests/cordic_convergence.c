#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <math.h>

#include "ieee_utils.h"
#include "cordic_circular.h"
#include "cordic_hyperbolic.h"
#include "cordic_transcendental.h"

int main(void) {
    char filename[64];
    snprintf(filename, sizeof(filename), "tests/cordic_convergence_data_%d.csv", CORDIC_ITERATIONS);
    FILE *fp = fopen(filename, "w");

    if (fp == NULL) {
        fprintf(stderr, "Erro ao criar o arquivo %s\n", filename);
        return 1;
    }

    fprintf(fp, "N,Theoretical_Bound,Err_Sin_PI_4,Err_Cos_PI_4,Err_Tan_PI_4,Err_Atan_0_5,"
                "Err_Circular_Rotate_X,Err_Circular_Rotate_Y,Err_Circular_Rotate_Z,"
                "Err_Circular_Vector_X,Err_Circular_Vector_Y,Err_Circular_Vector_Z,"
                "Err_Sinh_0_5,Err_Cosh_0_5,Err_Tanh_0_5,Err_Atanh_1_3,"
                "Err_Hyperbolic_Rotate_X,Err_Hyperbolic_Rotate_Y,Err_Hyperbolic_Rotate_Z,"
                "Err_Hyperbolic_Vector_X,Err_Hyperbolic_Vector_Y,Err_Hyperbolic_Vector_Z,"
                "Err_Exp_1_0,Err_Ln_2_0,Err_Sqrt_2_0,Err_Pow_1_5_1_25\n");

    /* Pontos de teste fixos para analise de convergencia */
    double theta = PI_4;
    double circular_vector_y = 0.5;
    double u = 0.5;
    double hyperbolic_vector_y = 1.0 / 3.0;
    double x_exp = 1.0;
    double x_ln = 2.0;
    double x_sqrt = 2.0;
    double pow_base = 1.5;
    double pow_exponent = 1.25;

    /* Valores de referencia (math.h) */
    double ref_sin = sin(theta);
    double ref_cos = cos(theta);
    double ref_tan = tan(theta);
    double ref_atan = atan(circular_vector_y);
    double ref_circular_vector_x = sqrt(1.0 + circular_vector_y * circular_vector_y) / CORDIC_K_CIRCULAR;
    double ref_sinh = sinh(u);
    double ref_cosh = cosh(u);
    double ref_tanh = tanh(u);
    double ref_atanh = atanh(hyperbolic_vector_y);
    double ref_hyperbolic_vector_x = sqrt(1.0 - hyperbolic_vector_y * hyperbolic_vector_y) / CORDIC_K_HYPERBOLIC;
    double ref_exp = exp(x_exp);
    double ref_ln = log(x_ln);
    double ref_sqrt = sqrt(x_sqrt);
    double ref_pow = pow(pow_base, pow_exponent);

    /* Varre de 1 ate o numero configurado de iteracoes */
    for (int n = 1; n <= CORDIC_ITERATIONS; n++) {
        double theoretical_bound = pow(2.0, -n);

        /* 1. Funcoes Circulares */
        double s_val, c_val;
        cordic_circular_sin_cos(theta, &s_val, &c_val, n);
        double err_sin = fabs(s_val - ref_sin);
        double err_cos = fabs(c_val - ref_cos);

        double tan_val = cordic_circular_tan(theta, n);
        double err_tan = fabs(tan_val - ref_tan);

        double atan_val = cordic_circular_atan(circular_vector_y, n);
        double err_atan = fabs(atan_val - ref_atan);

        CordicVector circular_rotate_input = { .x.x = CORDIC_K_CIRCULAR, .y.x = 0.0, .z.x = theta };
        CordicVector circular_rotate_result = cordic_circular_rotate(circular_rotate_input, n);
        double err_circular_rotate_x = fabs(circular_rotate_result.x.x - ref_cos);
        double err_circular_rotate_y = fabs(circular_rotate_result.y.x - ref_sin);
        double err_circular_rotate_z = fabs(circular_rotate_result.z.x);

        CordicVector circular_vector_input = { .x.x = 1.0, .y.x = circular_vector_y, .z.x = 0.0 };
        CordicVector circular_vector_result = cordic_circular_vector(circular_vector_input, n);
        double err_circular_vector_x = fabs(circular_vector_result.x.x - ref_circular_vector_x);
        double err_circular_vector_y = fabs(circular_vector_result.y.x);
        double err_circular_vector_z = fabs(circular_vector_result.z.x - ref_atan);

        /* 2. Funcoes Hiperbolicas */
        double sh_val, ch_val;
        cordic_hyperbolic_sinh_cosh(u, &sh_val, &ch_val, n);
        double err_sinh = fabs(sh_val - ref_sinh);
        double err_cosh = fabs(ch_val - ref_cosh);

        double tanh_val = cordic_hyperbolic_tanh(u, n);
        double err_tanh = fabs(tanh_val - ref_tanh);

        double atanh_val = cordic_hyperbolic_atanh(hyperbolic_vector_y, n);
        double err_atanh = fabs(atanh_val - ref_atanh);

        CordicVector hyperbolic_rotate_input = { .x.x = CORDIC_K_HYPERBOLIC, .y.x = 0.0, .z.x = u };
        CordicVector hyperbolic_rotate_result = cordic_hyperbolic_rotate(hyperbolic_rotate_input, n);
        double err_hyperbolic_rotate_x = fabs(hyperbolic_rotate_result.x.x - ref_cosh);
        double err_hyperbolic_rotate_y = fabs(hyperbolic_rotate_result.y.x - ref_sinh);
        double err_hyperbolic_rotate_z = fabs(hyperbolic_rotate_result.z.x);

        CordicVector hyperbolic_vector_input = { .x.x = 1.0, .y.x = hyperbolic_vector_y, .z.x = 0.0 };
        CordicVector hyperbolic_vector_result = cordic_hyperbolic_vector(hyperbolic_vector_input, n);
        double err_hyperbolic_vector_x = fabs(hyperbolic_vector_result.x.x - ref_hyperbolic_vector_x);
        double err_hyperbolic_vector_y = fabs(hyperbolic_vector_result.y.x);
        double err_hyperbolic_vector_z = fabs(hyperbolic_vector_result.z.x - ref_atanh);

        /* 3. Funcoes Transcendentais Compostas */
        double exp_val = cordic_exp(x_exp, n);
        double err_exp = fabs(exp_val - ref_exp);

        double ln_val = cordic_ln(x_ln, n);
        double err_ln = fabs(ln_val - ref_ln);

        double sqrt_val = cordic_sqrt(x_sqrt, n);
        double err_sqrt = fabs(sqrt_val - ref_sqrt);

        double pow_val = cordic_pow(pow_base, pow_exponent, n);
        double err_pow = fabs(pow_val - ref_pow);

        fprintf(fp, "%d,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,"
                "%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,"
                "%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e,%.10e\n",
                n,
                theoretical_bound,
                err_sin,
                err_cos,
            err_tan,
            err_atan,
            err_circular_rotate_x,
            err_circular_rotate_y,
            err_circular_rotate_z,
            err_circular_vector_x,
            err_circular_vector_y,
            err_circular_vector_z,
                err_sinh,
                err_cosh,
            err_tanh,
            err_atanh,
            err_hyperbolic_rotate_x,
            err_hyperbolic_rotate_y,
            err_hyperbolic_rotate_z,
            err_hyperbolic_vector_x,
            err_hyperbolic_vector_y,
            err_hyperbolic_vector_z,
                err_exp,
                err_ln,
            err_sqrt,
            err_pow);
    }

    fclose(fp);
    printf("Arquivo '%s' gerado com sucesso!\n", filename);
    return 0;
}