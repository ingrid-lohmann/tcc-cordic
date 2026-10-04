#define _POSIX_C_SOURCE 199309L

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "ieee_utils.h"

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
#define CORDIC_ITERS CORDIC_ITERATIONS
#define POW_EXPONENT 1.25

typedef double (*BenchmarkFunction)(double);

typedef struct {
    const char *name;
    double input;
    BenchmarkFunction cordic;
    BenchmarkFunction reference;
} BenchmarkCase;

typedef struct {
    double elapsed_seconds;
    uint64_t cycles;
} BenchmarkResult;

static volatile double sink;

static double vector_sum(CordicVector vector) {
    return vector.x.x + vector.y.x + vector.z.x;
}

static double bench_circular_rotate(double angle) {
    CordicVector vector = { .x.x = CORDIC_K_CIRCULAR, .y.x = 0.0, .z.x = angle };
    return vector_sum(cordic_circular_rotate(vector, CORDIC_ITERS));
}

static double bench_circular_vector(double value) {
    CordicVector vector = { .x.x = 1.0, .y.x = value, .z.x = 0.0 };
    return vector_sum(cordic_circular_vector(vector, CORDIC_ITERS));
}

static double bench_sin_cos(double angle) {
    double sine, cosine;
    cordic_circular_sin_cos(angle, &sine, &cosine, CORDIC_ITERS);
    return sine + cosine;
}

static double bench_tan(double angle) {
    return cordic_circular_tan(angle, CORDIC_ITERS);
}

static double bench_atan(double value) {
    return cordic_circular_atan(value, CORDIC_ITERS);
}

static double bench_hyperbolic_rotate(double angle) {
    CordicVector vector = { .x.x = CORDIC_K_HYPERBOLIC, .y.x = 0.0, .z.x = angle };
    return vector_sum(cordic_hyperbolic_rotate(vector, CORDIC_ITERS));
}

static double bench_hyperbolic_vector(double value) {
    CordicVector vector = { .x.x = 1.0, .y.x = value, .z.x = 0.0 };
    return vector_sum(cordic_hyperbolic_vector(vector, CORDIC_ITERS));
}

static double bench_sinh_cosh(double angle) {
    double sinh_value, cosh_value;
    cordic_hyperbolic_sinh_cosh(angle, &sinh_value, &cosh_value, CORDIC_ITERS);
    return sinh_value + cosh_value;
}

static double bench_tanh(double angle) {
    return cordic_hyperbolic_tanh(angle, CORDIC_ITERS);
}

static double bench_atanh(double value) {
    return cordic_hyperbolic_atanh(value, CORDIC_ITERS);
}

static double bench_exp(double value) {
    return cordic_exp(value, CORDIC_ITERS);
}

static double bench_ln(double value) {
    return cordic_ln(value, CORDIC_ITERS);
}

static double bench_sqrt(double value) {
    return cordic_sqrt(value, CORDIC_ITERS);
}

static double bench_pow(double base) {
    return cordic_pow(base, POW_EXPONENT, CORDIC_ITERS);
}

static double reference_sin_cos(double angle) {
    return sin(angle) + cos(angle);
}

static double reference_sinh_cosh(double angle) {
    return sinh(angle) + cosh(angle);
}

static double reference_pow(double base) {
    return pow(base, POW_EXPONENT);
}

static double get_elapsed_seconds(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) +
           (double)(end.tv_nsec - start.tv_nsec) * 1e-9;
}

static BenchmarkResult measure(BenchmarkFunction function, double input) {
    struct timespec start, end;
    uint64_t cycles_start = read_cycles();
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_RUNS; i++) {
        sink += function(input);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    uint64_t cycles_end = read_cycles();

    BenchmarkResult result = {
        .elapsed_seconds = get_elapsed_seconds(start, end),
        .cycles = cycles_end - cycles_start
    };
    return result;
}

static void print_result(FILE *csv, const char *operation, const char *implementation,
                         BenchmarkResult result, double slowdown) {
    printf("%-22s | %-7s | %10.4f s | %10.2f ns | %10.1f | ",
           operation, implementation, result.elapsed_seconds,
           (result.elapsed_seconds / NUM_RUNS) * 1e9,
           (double)result.cycles / NUM_RUNS);
    if (isnan(slowdown)) {
        printf("N/A\n");
    } else {
        printf("%10.2fx\n", slowdown);
    }

    if (csv != NULL) {
        if (isnan(slowdown)) {
            fprintf(csv, "%s,%s,%.6f,%.2f,%.2f,\n", operation, implementation,
                    result.elapsed_seconds, (result.elapsed_seconds / NUM_RUNS) * 1e9,
                    (double)result.cycles / NUM_RUNS);
        } else {
            fprintf(csv, "%s,%s,%.6f,%.2f,%.2f,%.2f\n", operation, implementation,
                    result.elapsed_seconds, (result.elapsed_seconds / NUM_RUNS) * 1e9,
                    (double)result.cycles / NUM_RUNS, slowdown);
        }
    }
}

static void run_case(FILE *csv, const BenchmarkCase *benchmark) {
    BenchmarkResult cordic_result = measure(benchmark->cordic, benchmark->input);
    BenchmarkResult reference_result = { 0.0, 0 };
    double slowdown = NAN;

    if (benchmark->reference != NULL) {
        reference_result = measure(benchmark->reference, benchmark->input);
        slowdown = cordic_result.elapsed_seconds / reference_result.elapsed_seconds;
    }

    print_result(csv, benchmark->name, "CORDIC", cordic_result, slowdown);
    if (benchmark->reference != NULL) {
        print_result(csv, benchmark->name, "math.h", reference_result, 1.0);
    }
}

int main(void) {
    static const BenchmarkCase benchmarks[] = {
        { "circular_rotate", 0.5, bench_circular_rotate, NULL },
        { "circular_vector", 0.5, bench_circular_vector, NULL },
        { "sin_cos", PI_4, bench_sin_cos, reference_sin_cos },
        { "tan", 0.5, bench_tan, tan },
        { "atan", 0.5, bench_atan, atan },
        { "hyperbolic_rotate", 0.5, bench_hyperbolic_rotate, NULL },
        { "hyperbolic_vector", 1.0 / 3.0, bench_hyperbolic_vector, NULL },
        { "sinh_cosh", 0.5, bench_sinh_cosh, reference_sinh_cosh },
        { "tanh", 0.5, bench_tanh, tanh },
        { "atanh", 1.0 / 3.0, bench_atanh, atanh },
        { "exp", 0.5, bench_exp, exp },
        { "ln", 2.0, bench_ln, log },
        { "sqrt", 2.0, bench_sqrt, sqrt },
        { "pow", 1.5, bench_pow, reference_pow }
    };
    char csv_filename[64];
    snprintf(csv_filename, sizeof(csv_filename), "tests/benchmark_results_%d.csv", CORDIC_ITERS);
    FILE *csv = fopen(csv_filename, "w");
    if (csv == NULL) {
        perror("Erro ao abrir arquivo de benchmark para gravacao");
        return 1;
    }

    fprintf(csv, "Operation,Implementation,Total_Time_s,Latency_ns_op,Cycles_op,Slowdown_vs_math\n");
    printf("Benchmark CORDIC: %d repeticoes | %d iteracoes\n\n", NUM_RUNS, CORDIC_ITERS);
    printf("%-22s | %-7s | %-12s | %-10s | %-10s | %s\n",
           "Operacao", "Implementacao", "Tempo Total", "Tempo/Op", "Ciclos/Op", "Slowdown");
    printf("-----------------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < sizeof(benchmarks) / sizeof(benchmarks[0]); i++) {
        run_case(csv, &benchmarks[i]);
    }

    if (fclose(csv) != 0) {
        perror("Erro ao fechar arquivo de benchmark");
        return 1;
    }

    printf("\nResultados salvos em '%s'.\n", csv_filename);
    if (sink == 42.0) {
        printf("Ignore: %f\n", sink);
    }
    return 0;
}