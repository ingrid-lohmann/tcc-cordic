#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "cordic_circular.h"
#include "cordic_hyperbolic.h"
#include "cordic_transcendental.h"

#define TEST_ITERATIONS 24
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
    f_md = fopen("tests/edge_cases_report.md", "w");
    f_csv = fopen("tests/edge_cases_results.csv", "w");

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
    cordic_circular_sin_cos(-M_PI / 4.0, &s, &c, TEST_ITERATIONS);
    assert_close("Simetria/Quadrante", "cordic_circular_sin(-pi/4)", s, -sin(M_PI / 4.0), EPSILON);
    assert_close("Simetria/Quadrante", "cordic_circular_cos(-pi/4)", c, cos(M_PI / 4.0), EPSILON);

    cordic_circular_sin_cos(3.0 * M_PI, &s, &c, TEST_ITERATIONS);
    assert_close("Simetria/Quadrante", "cordic_circular_sin(3*pi)", s, sin(3.0 * M_PI), EPSILON);
    assert_close("Simetria/Quadrante", "cordic_circular_cos(3*pi)", c, cos(3.0 * M_PI), EPSILON);

    printf("\n=================================================================\n");
    printf("RESUMO: %d/%d testes passaram (%.1f%%)\n", 
           passed_tests, total_tests, (float)passed_tests / total_tests * 100.0f);
    printf("Arquivos gerados:\n  -> tests/edge_cases_report.md\n  -> tests/edge_cases_results.csv\n");
    printf("=================================================================\n");

    if (f_md) {
        fprintf(f_md, "\n**Resumo**: %d/%d testes passaram (taxa de sucesso: %.1f%%).\n",
                passed_tests, total_tests, (float)passed_tests / total_tests * 100.0f);
        fclose(f_md);
    }
    if (f_csv) fclose(f_csv);

    return (passed_tests == total_tests) ? 0 : 1;
}