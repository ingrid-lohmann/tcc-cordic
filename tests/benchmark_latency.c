#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if defined(__x86_64__) || defined(_M_X64)
#include <x86intrin.h>
static inline uint64_t read_cycles(void) {
    return __rdtsc();
}
#else
static inline uint64_t read_cycles(void) {
    return 0;
}
#endif

#include "cordic_circular.h"
#include "cordic_hyperbolic.h"
#include "cordic_transcendental.h"

#define NUM_RUNS 1000000
#define CORDIC_ITERS 24

static double get_elapsed_seconds(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) + 
           (double)(end.tv_nsec - start.tv_nsec) * 1e-9;
}

int main(void) {
    struct timespec t_start, t_end;
    uint64_t c_start, c_end;
    volatile double sink = 0.0;

    printf("Iniciando bateria de testes com %d repeticoes...\n", NUM_RUNS);

    /* -------------------------------------------------------------------------
     * 1. Funcoes Circulares (Seno e Cosseno)
     * ------------------------------------------------------------------------- */
    double theta = M_PI / 4.0;

    c_start = read_cycles();
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    for (int i = 0; i < NUM_RUNS; i++) {
        double s, c;
        cordic_circular_sin_cos(theta, &s, &c, CORDIC_ITERS);
        sink += s + c;
    }
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    c_end = read_cycles();
    double time_cordic_sincos = get_elapsed_seconds(t_start, t_end);
    uint64_t cycles_cordic_sincos = c_end - c_start;

    c_start = read_cycles();
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    for (int i = 0; i < NUM_RUNS; i++) {
        double s = sin(theta);
        double c = cos(theta);
        sink += s + c;
    }
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    c_end = read_cycles();
    double time_math_sincos = get_elapsed_seconds(t_start, t_end);
    uint64_t cycles_math_sincos = c_end - c_start;

    /* -------------------------------------------------------------------------
     * 2. Funcoes Hiperbolicas (Sinh e Cosh)
     * ------------------------------------------------------------------------- */
    double u = 0.5;

    c_start = read_cycles();
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    for (int i = 0; i < NUM_RUNS; i++) {
        double sh, ch;
        cordic_hyperbolic_sinh_cosh(u, &sh, &ch, CORDIC_ITERS);
        sink += sh + ch;
    }
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    c_end = read_cycles();
    double time_cordic_hyp = get_elapsed_seconds(t_start, t_end);
    uint64_t cycles_cordic_hyp = c_end - c_start;

    c_start = read_cycles();
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    for (int i = 0; i < NUM_RUNS; i++) {
        double sh = sinh(u);
        double ch = cosh(u);
        sink += sh + ch;
    }
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    c_end = read_cycles();
    double time_math_hyp = get_elapsed_seconds(t_start, t_end);
    uint64_t cycles_math_hyp = c_end - c_start;

    /* -------------------------------------------------------------------------
     * 3. Raiz Quadrada (sqrt)
     * ------------------------------------------------------------------------- */
    double x_sqrt = 2.0;

    c_start = read_cycles();
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    for (int i = 0; i < NUM_RUNS; i++) {
        sink += cordic_sqrt(x_sqrt, CORDIC_ITERS);
    }
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    c_end = read_cycles();
    double time_cordic_sqrt = get_elapsed_seconds(t_start, t_end);
    uint64_t cycles_cordic_sqrt = c_end - c_start;

    c_start = read_cycles();
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    for (int i = 0; i < NUM_RUNS; i++) {
        sink += sqrt(x_sqrt);
    }
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    c_end = read_cycles();
    double time_math_sqrt = get_elapsed_seconds(t_start, t_end);
    uint64_t cycles_math_sqrt = c_end - c_start;

    /* -------------------------------------------------------------------------
     * Saída 1: Terminal (stdout)
     * ------------------------------------------------------------------------- */
    printf("\n========================================================================================\n");
    printf("                  BENCHMARK DE LATENCIA: CORDIC vs math.h (FPU/SIMD)                    \n");
    printf("                  Amostras: %d | Iteracoes CORDIC: %d | Formato: DoubleIEEE           \n", NUM_RUNS, CORDIC_ITERS);
    printf("========================================================================================\n\n");

    printf("%-20s | %-12s | %-12s | %-10s | %-12s\n", 
           "Operacao", "Tempo Total", "Tempo/Op", "Ciclos/Op", "Slowdown vs math.h");
    printf("----------------------------------------------------------------------------------------\n");

    printf("%-20s | %10.4f s | %10.2f ns | %10.1f | %10.2fx\n",
           "CORDIC sin+cos", time_cordic_sincos, 
           (time_cordic_sincos / NUM_RUNS) * 1e9, 
           (double)cycles_cordic_sincos / NUM_RUNS,
           time_cordic_sincos / time_math_sincos);

    printf("%-20s | %10.4f s | %10.2f ns | %10.1f | %10.2fx (ref)\n",
           "math.h sin+cos", time_math_sincos, 
           (time_math_sincos / NUM_RUNS) * 1e9, 
           (double)cycles_math_sincos / NUM_RUNS, 1.0);

    printf("----------------------------------------------------------------------------------------\n");

    printf("%-20s | %10.4f s | %10.2f ns | %10.1f | %10.2fx\n",
           "CORDIC sinh+cosh", time_cordic_hyp, 
           (time_cordic_hyp / NUM_RUNS) * 1e9, 
           (double)cycles_cordic_hyp / NUM_RUNS,
           time_cordic_hyp / time_math_hyp);

    printf("%-20s | %10.4f s | %10.2f ns | %10.1f | %10.2fx (ref)\n",
           "math.h sinh+cosh", time_math_hyp, 
           (time_math_hyp / NUM_RUNS) * 1e9, 
           (double)cycles_math_hyp / NUM_RUNS, 1.0);

    printf("----------------------------------------------------------------------------------------\n");

    printf("%-20s | %10.4f s | %10.2f ns | %10.1f | %10.2fx\n",
           "CORDIC sqrt", time_cordic_sqrt, 
           (time_cordic_sqrt / NUM_RUNS) * 1e9, 
           (double)cycles_cordic_sqrt / NUM_RUNS,
           time_cordic_sqrt / time_math_sqrt);

    printf("%-20s | %10.4f s | %10.2f ns | %10.1f | %10.2fx (ref)\n",
           "math.h sqrt", time_math_sqrt, 
           (time_math_sqrt / NUM_RUNS) * 1e9, 
           (double)cycles_math_sqrt / NUM_RUNS, 1.0);

    printf("========================================================================================\n\n");

    /* -------------------------------------------------------------------------
     * Saída 2: Arquivo CSV (.csv)
     * ------------------------------------------------------------------------- */
    const char *csv_filename = "benchmark_results.csv";
    FILE *fp = fopen(csv_filename, "w");
    if (fp == NULL) {
        perror("Erro ao abrir benchmark_results.csv para gravacao");
        return 1;
    }

    /* Cabeçalho do CSV */
    fprintf(fp, "Operation,Total_Time_s,Latency_ns_op,Cycles_op,Slowdown_vs_math\n");

    /* Linhas de dados */
    fprintf(fp, "CORDIC_sin_cos,%.6f,%.2f,%.2f,%.2f\n",
            time_cordic_sincos,
            (time_cordic_sincos / NUM_RUNS) * 1e9,
            (double)cycles_cordic_sincos / NUM_RUNS,
            time_cordic_sincos / time_math_sincos);

    fprintf(fp, "math_sin_cos,%.6f,%.2f,%.2f,1.00\n",
            time_math_sincos,
            (time_math_sincos / NUM_RUNS) * 1e9,
            (double)cycles_math_sincos / NUM_RUNS);

    fprintf(fp, "CORDIC_sinh_cosh,%.6f,%.2f,%.2f,%.2f\n",
            time_cordic_hyp,
            (time_cordic_hyp / NUM_RUNS) * 1e9,
            (double)cycles_cordic_hyp / NUM_RUNS,
            time_cordic_hyp / time_math_hyp);

    fprintf(fp, "math_sinh_cosh,%.6f,%.2f,%.2f,1.00\n",
            time_math_hyp,
            (time_math_hyp / NUM_RUNS) * 1e9,
            (double)cycles_math_hyp / NUM_RUNS);

    fprintf(fp, "CORDIC_sqrt,%.6f,%.2f,%.2f,%.2f\n",
            time_cordic_sqrt,
            (time_cordic_sqrt / NUM_RUNS) * 1e9,
            (double)cycles_cordic_sqrt / NUM_RUNS,
            time_cordic_sqrt / time_math_sqrt);

    fprintf(fp, "math_sqrt,%.6f,%.2f,%.2f,1.00\n",
            time_math_sqrt,
            (time_math_sqrt / NUM_RUNS) * 1e9,
            (double)cycles_math_sqrt / NUM_RUNS);

    fflush(fp);
    fclose(fp);

    printf("Sucesso: Resultados salvos no arquivo '%s' no diretorio atual.\n", csv_filename);

    if (sink == 42.0) printf("Ignore: %f\n", sink);
    return 0;
}