#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "ieee_utils.h"
#include "cordic_circular.h"
#include "cordic_hyperbolic.h"
#include "cordic_transcendental.h"

#define TEST_ITERATIONS CORDIC_ITERATIONS
#define EPSILON 1e-7

static int total_tests = 0;
static int passed_tests = 0;
static FILE *f_md = NULL;
static FILE *f_csv = NULL;

void log_result(const char *category, const char *test_name, const char *status, 
                const char *actual_str, const char *expected_str, double diff) {
    total_tests++;
    bool is_pass = (status[0] == 'P');
    if (is_pass) passed_tests++;

    printf("  [%s] %-32s = %s (esperado: %s)\n", status, test_name, actual_str, expected_str);

    if (f_md) {
        fprintf(f_md, "| %s | `%s` | %s | %s | %.2e | **%s** |\n",
                category, test_name, actual_str, expected_str, diff, status);
    }

    if (f_csv) {
        fprintf(f_csv, "%s,%s,%s,%s,%.2e,%s\n",
                category, test_name, actual_str, expected_str, diff, status);
    }
}

void assert_close(const char *category, const char *name, double actual, double expected, double tol) {
    char act_buf[64], exp_buf[64];
    snprintf(act_buf, sizeof(act_buf), "%.9f", actual);
    snprintf(exp_buf, sizeof(exp_buf), "%.9f", expected);
    double diff = fabs(actual - expected);
    const char *status = (diff <= tol) ? "PASS" : "FAIL";
    log_result(category, name, status, act_buf, exp_buf, diff);
}

void assert_isnan(const char *category, const char *name, double actual) {
    char act_buf[64];
    snprintf(act_buf, sizeof(act_buf), "%s", isnan(actual) ? "NaN" : "Numérico");
    const char *status = isnan(actual) ? "PASS" : "FAIL";
    log_result(category, name, status, act_buf, "NaN", isnan(actual) ? 0.0 : 1.0);
}

void assert_isneginf(const char *category, const char *name, double actual) {
    char act_buf[64];
    snprintf(act_buf, sizeof(act_buf), "%s", (isinf(actual) && actual < 0) ? "-Inf" : "Outro");
    const char *status = (isinf(actual) && actual < 0) ? "PASS" : "FAIL";
    log_result(category, name, status, act_buf, "-Infinity", (isinf(actual) && actual < 0) ? 0.0 : 1.0);
}

int main(void) {
    char md_filename[64];
    char csv_filename[64];
    snprintf(md_filename, sizeof(md_filename), "tests/edge_cases_report_%d.md", TEST_ITERATIONS);
    snprintf(csv_filename, sizeof(csv_filename), "tests/edge_cases_results_%d.csv", TEST_ITERATIONS);
    f_md = fopen(md_filename, "w");
    f_csv = fopen(csv_filename, "w");

    if (f_md) {
        fprintf(f_md, "# Relatório de Casos de Borda e Conformidade IEEE 754\n\n");
        fprintf(f_md, "| Categoria | Teste / Entrada | Obtido | Esperado | Diferença Abs. | Status |\n");
        fprintf(f_md, "| :--- | :--- | :--- | :--- | :---: | :---: |\n");
    }
    if (f_csv) {
        fprintf(f_csv, "Category,Test_Name,Actual,Expected,Diff,Status\n");
    }

    printf("=================================================================\n");
    printf("BATERIA DE TESTES DE CASOS DE BORDA E CONFORMIDADE IEEE 754\n");
    printf("=================================================================\n\n");

    printf("1. PONTO ZERO E IDENTIDADES FUNDAMENTAIS:\n");
    double s, c;
    cordic_circular_sin_cos(0.0, &s, &c, TEST_ITERATIONS);
    assert_close("Zero/Identidade", "cordic_circular_sin(0.0)", s, 0.0, EPSILON);
    assert_close("Zero/Identidade", "cordic_circular_cos(0.0)", c, 1.0, EPSILON);
    assert_close("Zero/Identidade", "cordic_circular_tan(0.0)", cordic_circular_tan(0.0, TEST_ITERATIONS), 0.0, EPSILON);

    double sh, ch;
    cordic_hyperbolic_sinh_cosh(0.0, &sh, &ch, TEST_ITERATIONS);
    assert_close("Zero/Identidade", "cordic_hyperbolic_sinh(0.0)", sh, 0.0, EPSILON);
    assert_close("Zero/Identidade", "cordic_hyperbolic_cosh(0.0)", ch, 1.0, EPSILON);

    assert_close("Zero/Identidade", "cordic_exp(0.0)", cordic_exp(0.0, TEST_ITERATIONS), 1.0, EPSILON);
    assert_close("Zero/Identidade", "cordic_ln(1.0)", cordic_ln(1.0, TEST_ITERATIONS), 0.0, EPSILON);
    assert_close("Zero/Identidade", "cordic_sqrt(0.0)", cordic_sqrt(0.0, TEST_ITERATIONS), 0.0, EPSILON);

    printf("\n2. DOMÍNIOS INVÁLIDOS (IEEE 754 / NaN / INF):\n");
    assert_isnan("Dominio Invalido", "cordic_sqrt(-1.0)", cordic_sqrt(-1.0, TEST_ITERATIONS));
    assert_isnan("Dominio Invalido", "cordic_sqrt(-100.0)", cordic_sqrt(-100.0, TEST_ITERATIONS));
    assert_isnan("Dominio Invalido", "cordic_ln(-2.0)", cordic_ln(-2.0, TEST_ITERATIONS));
    assert_isneginf("Dominio Invalido", "cordic_ln(0.0)", cordic_ln(0.0, TEST_ITERATIONS));

    printf("\n3. ENTRADAS ESPECIAIS (PROPAGAÇÃO IEEE 754):\n");
    cordic_circular_sin_cos(NAN, &s, &c, TEST_ITERATIONS);
    assert_isnan("Entrada Especial", "cordic_circular_sin(NAN)", s);
    assert_isnan("Entrada Especial", "cordic_circular_cos(NAN)", c);
    assert_isnan("Entrada Especial", "cordic_sqrt(NAN)", cordic_sqrt(NAN, TEST_ITERATIONS));
    assert_isnan("Entrada Especial", "cordic_ln(NAN)", cordic_ln(NAN, TEST_ITERATIONS));

    printf("\n4. SIMETRIAS E ÂNGULOS FORA DO 1o QUADRANTE:\n");
    cordic_circular_sin_cos(-PI / 4.0, &s, &c, TEST_ITERATIONS);
    assert_close("Simetria/Quadrante", "cordic_circular_sin(-pi/4)", s, -sin(PI / 4.0), EPSILON);
    assert_close("Simetria/Quadrante", "cordic_circular_cos(-pi/4)", c, cos(PI / 4.0), EPSILON);

    cordic_circular_sin_cos(3.0 * PI, &s, &c, TEST_ITERATIONS);
    assert_close("Simetria/Quadrante", "cordic_circular_sin(3*pi)", s, sin(3.0 * PI), EPSILON);
    assert_close("Simetria/Quadrante", "cordic_circular_cos(3*pi)", c, cos(3.0 * PI), EPSILON);

    printf("\n5. FUNCOES RESTANTES E MOTORES PRIMITIVOS:\n");
    double circular_value = 0.5;
    assert_close("Circular", "cordic_circular_atan(0.5)",
                 cordic_circular_atan(circular_value, TEST_ITERATIONS), atan(circular_value), EPSILON);

    CordicVector circular_rotate_input = { .x.x = CORDIC_K_CIRCULAR, .y.x = 0.0, .z.x = PI_4 };
    CordicVector circular_rotate_result = cordic_circular_rotate(circular_rotate_input, TEST_ITERATIONS);
    assert_close("Motor Circular", "rotate.x", circular_rotate_result.x.x, cos(PI_4), EPSILON);
    assert_close("Motor Circular", "rotate.y", circular_rotate_result.y.x, sin(PI_4), EPSILON);
    assert_close("Motor Circular", "rotate.z", circular_rotate_result.z.x, 0.0, EPSILON);

    CordicVector circular_vector_input = { .x.x = 1.0, .y.x = circular_value, .z.x = 0.0 };
    CordicVector circular_vector_result = cordic_circular_vector(circular_vector_input, TEST_ITERATIONS);
    assert_close("Motor Circular", "vector.x", circular_vector_result.x.x,
                 sqrt(1.0 + circular_value * circular_value) / CORDIC_K_CIRCULAR, EPSILON);
    assert_close("Motor Circular", "vector.y", circular_vector_result.y.x, 0.0, 2.0 * EPSILON);
    assert_close("Motor Circular", "vector.z", circular_vector_result.z.x, atan(circular_value), EPSILON);

    double hyperbolic_value = 0.5;
    assert_close("Hiperbolica", "cordic_hyperbolic_tanh(0.5)",
                 cordic_hyperbolic_tanh(hyperbolic_value, TEST_ITERATIONS), tanh(hyperbolic_value), EPSILON);
    double atanh_value = 1.0 / 3.0;
    assert_close("Hiperbolica", "cordic_hyperbolic_atanh(1/3)",
                 cordic_hyperbolic_atanh(atanh_value, TEST_ITERATIONS), atanh(atanh_value), EPSILON);
    assert_isnan("Dominio Invalido", "cordic_hyperbolic_atanh(1.0)",
                 cordic_hyperbolic_atanh(1.0, TEST_ITERATIONS));

    double sinh_value, cosh_value;
    cordic_hyperbolic_sinh_cosh(hyperbolic_value, &sinh_value, &cosh_value, TEST_ITERATIONS);
    CordicVector hyperbolic_rotate_input = {
        .x.x = CORDIC_K_HYPERBOLIC, .y.x = 0.0, .z.x = hyperbolic_value
    };
    CordicVector hyperbolic_rotate_result = cordic_hyperbolic_rotate(hyperbolic_rotate_input, TEST_ITERATIONS);
    assert_close("Motor Hiperbolico", "rotate.x", hyperbolic_rotate_result.x.x, cosh(hyperbolic_value), EPSILON);
    assert_close("Motor Hiperbolico", "rotate.y", hyperbolic_rotate_result.y.x, sinh(hyperbolic_value), EPSILON);
    assert_close("Motor Hiperbolico", "rotate.z", hyperbolic_rotate_result.z.x, 0.0, EPSILON);

    CordicVector hyperbolic_vector_input = { .x.x = 1.0, .y.x = atanh_value, .z.x = 0.0 };
    CordicVector hyperbolic_vector_result = cordic_hyperbolic_vector(hyperbolic_vector_input, TEST_ITERATIONS);
    assert_close("Motor Hiperbolico", "vector.x", hyperbolic_vector_result.x.x,
                 sqrt(1.0 - atanh_value * atanh_value) / CORDIC_K_HYPERBOLIC, EPSILON);
    assert_close("Motor Hiperbolico", "vector.y", hyperbolic_vector_result.y.x, 0.0, EPSILON);
    assert_close("Motor Hiperbolico", "vector.z", hyperbolic_vector_result.z.x, atanh(atanh_value), EPSILON);

    double pow_base = 1.5;
    double pow_exponent = 1.25;
    assert_close("Transcendental", "cordic_pow(1.5, 1.25)",
                 cordic_pow(pow_base, pow_exponent, TEST_ITERATIONS), pow(pow_base, pow_exponent), EPSILON);
    assert_isnan("Dominio Invalido", "cordic_pow(-1.0, 2.0)",
                 cordic_pow(-1.0, 2.0, TEST_ITERATIONS));

    printf("\n=================================================================\n");
    printf("RESUMO: %d/%d testes passaram (%.1f%%)\n", 
           passed_tests, total_tests, (float)passed_tests / total_tests * 100.0f);
    printf("Arquivos gerados:\n  -> %s\n  -> %s\n", md_filename, csv_filename);
    printf("=================================================================\n");

    if (f_md) {
        fprintf(f_md, "\n**Resumo**: %d/%d testes passaram (taxa de sucesso: %.1f%%).\n",
                passed_tests, total_tests, (float)passed_tests / total_tests * 100.0f);
        fclose(f_md);
    }
    if (f_csv) fclose(f_csv);

    return (passed_tests == total_tests) ? 0 : 1;
}